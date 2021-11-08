// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCGenerator.h"

#include "WFCGrid.h"
#include "WFCModel.h"
#include "WFCModule.h"


UWFCGenerator::UWFCGenerator()
	: State(EWFCGeneratorState::None)
{
}

void UWFCGenerator::Initialize(const UWFCGrid* InGrid, const UWFCModel* InModel)
{
	if (!bIsInitialized)
	{
		Grid = InGrid;
		Model = InModel;

		// initialize cells
		TArray<FWFCTileId> AllTileCandidates;
		AllTileCandidates.SetNum(Model->Tiles.Num());
		for (int32 Idx = 0; Idx < AllTileCandidates.Num(); ++Idx)
		{
			// tile id is the same as the tile index
			AllTileCandidates[Idx] = Idx;
		}

		NumCells = Grid->GetNumCells();
		Cells.SetNum(NumCells);
		for (FWFCCellIndex Idx = 0; Idx < NumCells; ++Idx)
		{
			Cells[Idx].TileCandidates = AllTileCandidates;
		}

		bIsInitialized = true;
	}
}

void UWFCGenerator::Run(int32 StepLimit)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogWFC, Warning, TEXT("Initialize must be called before Run on a WFCGenerator"));
		return;
	}

	for (int32 Step = 0; Step < StepLimit; ++Step)
	{
		Next();

		if (State != EWFCGeneratorState::InProgress &&
			State != EWFCGeneratorState::None)
		{
			break;
		}
	}
}

void UWFCGenerator::Next()
{
	if (PropagateNext())
	{
		// some changes were propagated, that's enough for this step
		return;
	}

	// select a cell to observe
	const FWFCCellIndex CellIndex = SelectNextCellIndex();

	if (CellIndex == INDEX_NONE)
	{
		State = EWFCGeneratorState::Error;
		return;
	}

	// select the tile for the cell
	const FWFCTileId TileId = SelectNextTileForCell(CellIndex);

	if (TileId == INDEX_NONE)
	{
		State = EWFCGeneratorState::Error;
		return;
	}

	Select(CellIndex, TileId);

	State = EWFCGeneratorState::InProgress;
}

void UWFCGenerator::Ban(int32 CellIndex, int32 TileId)
{
	if (IsValidCellIndex(CellIndex))
	{
		FWFCCell& Cell = GetCell(CellIndex);
		if (Cell.RemoveCandidate(TileId))
		{
			OnCellChanged(CellIndex);
		}
	}
}

void UWFCGenerator::BanMultiple(int32 CellIndex, TArray<int32> TileIds)
{
	if (IsValidCellIndex(CellIndex) && TileIds.Num() > 0)
	{
		bool bWasChanged = false;

		FWFCCell& Cell = GetCell(CellIndex);
		for (const int32& TileId : TileIds)
		{
			bWasChanged |= Cell.RemoveCandidate(TileId);
		}

		if (bWasChanged)
		{
			OnCellChanged(CellIndex);
		}
	}
}

void UWFCGenerator::Select(int32 CellIndex, int32 TileId)
{
	if (IsValidCellIndex(CellIndex))
	{
		FWFCCell& Cell = GetCell(CellIndex);
		TArray<FWFCTileId> IdsToBan;
		for (const FWFCTileId& Id : Cell.TileCandidates)
		{
			if (Id != TileId)
			{
				IdsToBan.Add(Id);
			}
		}
		if (IdsToBan.Num() > 0)
		{
			BanMultiple(CellIndex, IdsToBan);
		}
	}
}

void UWFCGenerator::GetSelectedTiles(TArray<FWFCTile>& OutTiles) const
{
	OutTiles.SetNum(NumCells);

	for (FWFCCellIndex Idx = 0; Idx < NumCells; ++Idx)
	{
		const FWFCCell& Cell = GetCell(Idx);
		const FWFCTileId TileId = Cell.GetSelectedTileId();
		OutTiles[Idx] = Model->GetTile(TileId);
	}
}

FWFCCell& UWFCGenerator::GetCell(FWFCCellIndex Index)
{
	return Cells[Index];
}

const FWFCCell& UWFCGenerator::GetCell(FWFCCellIndex Index) const
{
	return Cells[Index];
}

void UWFCGenerator::OnCellChanged(FWFCCellIndex Index)
{
	// TODO: move this to adjacent constraint
	MarkCellForAdjacencyCheck(Index);
}

bool UWFCGenerator::PropagateNext()
{
	// TODO: move this to adjacent constraint
	return PropagateNextAdjacencyConstraint();
}

FWFCCellIndex UWFCGenerator::SelectNextCellIndex()
{
	// implement in subclass, finding a value between [0..NumCells)
	return INDEX_NONE;
}

FWFCTileId UWFCGenerator::SelectNextTileForCell(FWFCCellIndex Index)
{
	// implement in subclass
	return INDEX_NONE;
}

void UWFCGenerator::AddAdjacentTileMapping(FWFCTileId TileId, FWFCGridDirection Direction, FWFCTileId AcceptedTileId)
{
	TileAdjacencyMap.FindOrAdd(TileId).FindOrAdd(Direction).AddUnique(AcceptedTileId);
}


// Adjacency Constraint
// --------------------

void UWFCGenerator::MarkCellForAdjacencyCheck(FWFCCellIndex Index)
{
	// apply adjacency constraints
	const int32 NumDirections = Grid->GetNumDirections();
	for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
	{
		AdjacentCellDirsToCheck.AddUnique(FWFCCellIndexAndDirection(Index, Direction));
	}
}

bool UWFCGenerator::PropagateNextAdjacencyConstraint()
{
	while (AdjacentCellDirsToCheck.Num() > 0)
	{
		const FWFCCellIndexAndDirection CellDir = AdjacentCellDirsToCheck[0];
		check(Grid->IsValidCellIndex(CellDir.CellIndex));
		AdjacentCellDirsToCheck.RemoveAt(0);

		const FWFCCell& ChangedCell = GetCell(CellDir.CellIndex);

		// find the cell in the direction to check
		const FWFCCellIndex CellIndexToCheck = Grid->GetCellIndexInDirection(CellDir.CellIndex, CellDir.Direction);
		if (!Grid->IsValidCellIndex(CellIndexToCheck))
		{
			continue;
		}

		const FWFCCell& CellToCheck = GetCell(CellIndexToCheck);
		if (CellToCheck.HasSelection())
		{
			// don't change cells that are already selected
			continue;
		}

		// check all candidates and ban any that don't pass the constraint
		// (iterate in reverse to support banning)
		for (int32 Idx = CellToCheck.TileCandidates.Num() - 1; Idx >= 0; --Idx)
		{
			const FWFCTileId& TileId = CellToCheck.TileCandidates[Idx];
			TArray<FWFCTileId> AllowedIncomingTileIds = GetValidAdjacentTileIds(TileId, CellDir.Direction);
			if (!ChangedCell.HasAnyMatchingCandidate(AllowedIncomingTileIds))
			{
				Ban(CellIndexToCheck, TileId);
			}
		}

		// this next call is finished if any cell was selected
		if (CellToCheck.HasSelection())
		{
			return true;
		}
	}
	return false;
}

TArray<FWFCTileId> UWFCGenerator::GetValidAdjacentTileIds(FWFCTileId TileId, FWFCGridDirection Direction) const
{
	return TileAdjacencyMap.FindRef(TileId).FindRef(Direction);
}
