// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCGenerator.h"

#include "WFCModule.h"
#include "Core/WFCConstraint.h"
#include "Core/WFCGrid.h"


UWFCGenerator::UWFCGenerator()
	: State(EWFCGeneratorState::None)
{
}

void UWFCGenerator::Initialize(FWFCGeneratorConfig InConfig)
{
	if (!bIsInitialized)
	{
		Config = InConfig;

		InitializeGrid(Config.GridConfig.Get());
		InitializeCells();
		InitializeConstraints(Config.ConstraintClasses);

		State = EWFCGeneratorState::InProgress;

		bIsInitialized = true;
	}
}

void UWFCGenerator::InitializeGrid(const UWFCGridConfig* GridConfig)
{
	check(GridConfig != nullptr);
	Grid = UWFCGrid::NewGrid(this, GridConfig);
	check(Grid != nullptr);
}

void UWFCGenerator::InitializeConstraints(TArray<TSubclassOf<UWFCConstraint>> ConstraintClasses)
{
	Constraints.Reset();
	for (TSubclassOf<UWFCConstraint> ConstraintClass : ConstraintClasses)
	{
		UWFCConstraint* Constraint = NewObject<UWFCConstraint>(this, ConstraintClass);
		if (Constraint)
		{
			Constraints.Add(Constraint);
		}
	}

	// initialize once all constraints are constructed in case they need to reference each other
	for (UWFCConstraint* Constraint : Constraints)
	{
		Constraint->Initialize(this);
	}
}

void UWFCGenerator::InitializeCells()
{
	// generate array of all tile ids for default cell candidate list
	TArray<FWFCTileId> AllTileCandidates;
	AllTileCandidates.SetNum(GetNumTiles());
	for (int32 Idx = 0; Idx < AllTileCandidates.Num(); ++Idx)
	{
		// tile id is the same as the tile index
		AllTileCandidates[Idx] = Idx;
	}

	// fill the cells array
	NumCells = Grid->GetNumCells();
	Cells.SetNum(NumCells);
	for (FWFCCellIndex Idx = 0; Idx < NumCells; ++Idx)
	{
		Cells[Idx].TileCandidates = AllTileCandidates;
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
	// update all constraints, which may lead to cell selection 
	for (UWFCConstraint* Constraint : Constraints)
	{
		if (Constraint->Next())
		{
			State = EWFCGeneratorState::InProgress;
			return;
		}
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

void UWFCGenerator::GetSelectedTileIds(TArray<int32>& OutTileIds) const
{
	OutTileIds.SetNum(NumCells);
	for (FWFCCellIndex Idx = 0; Idx < NumCells; ++Idx)
	{
		const FWFCCell& Cell = GetCell(Idx);
		OutTileIds[Idx] = Cell.GetSelectedTileId();
	}
}

FWFCCell& UWFCGenerator::GetCell(FWFCCellIndex CellIndex)
{
	return Cells[CellIndex];
}

const FWFCCell& UWFCGenerator::GetCell(FWFCCellIndex CellIndex) const
{
	return Cells[CellIndex];
}

void UWFCGenerator::OnCellChanged(FWFCCellIndex CellIndex)
{
	if (GetCell(CellIndex).HasSelection())
	{
		OnCellSelected.Broadcast(CellIndex);
	}

	for (UWFCConstraint* Constraint : Constraints)
	{
		Constraint->NotifyCellChanged(CellIndex);
	}
}

FWFCCellIndex UWFCGenerator::SelectNextCellIndex()
{
	// implement in subclass, finding a value between [0..NumCells)
	return INDEX_NONE;
}

FWFCTileId UWFCGenerator::SelectNextTileForCell(FWFCCellIndex CellIndex)
{
	// implement in subclass
	return INDEX_NONE;
}
