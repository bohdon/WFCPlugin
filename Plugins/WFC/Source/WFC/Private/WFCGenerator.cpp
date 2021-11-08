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
		Cell.RemoveCandidate(TileId);
	}
}

void UWFCGenerator::BanMultiple(int32 CellIndex, TArray<int32> TileIds)
{
	if (IsValidCellIndex(CellIndex))
	{
		FWFCCell& Cell = GetCell(CellIndex);
		for (const int32& TileId : TileIds)
		{
			Cell.RemoveCandidate(TileId);
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
