// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCGenerator.h"

#include "WFCModule.h"
#include "Core/WFCCellSelector.h"
#include "Core/WFCConstraint.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"

DECLARE_CYCLE_STAT(TEXT("WFCGenerator Next"), STAT_WFCGeneratorNext, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Cells"), STAT_WFCGeneratorNumCells, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Tiles"), STAT_WFCGeneratorNumTiles, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Cells Selected"), STAT_WFCGeneratorNumCellsSelected, STATGROUP_WFC);


UWFCGenerator::UWFCGenerator()
	: State(EWFCGeneratorState::None)
{
}

void UWFCGenerator::SetState(EWFCGeneratorState NewState)
{
	if (State != NewState)
	{
		State = NewState;
		OnStateChanged.Broadcast(State);
	}
}

UWFCConstraint* UWFCGenerator::GetConstraint(TSubclassOf<UWFCConstraint> ConstraintClass) const
{
	for (UWFCConstraint* Constraint : Constraints)
	{
		if (Constraint->IsA(ConstraintClass))
		{
			return Constraint;
		}
	}
	return nullptr;
}

UWFCCellSelector* UWFCGenerator::GetCellSelector(TSubclassOf<UWFCCellSelector> SelectorClass) const
{
	for (UWFCCellSelector* CellSelector : CellSelectors)
	{
		if (CellSelector->IsA(SelectorClass))
		{
			return CellSelector;
		}
	}
	return nullptr;
}

void UWFCGenerator::Initialize(FWFCGeneratorConfig InConfig)
{
	if (!bIsInitialized)
	{
		Config = InConfig;

		NumTiles = Config.Model->GetNumTiles();
		SET_DWORD_STAT(STAT_WFCGeneratorNumTiles, NumTiles);

		InitializeGrid(Config.GridConfig.Get());
		InitializeCells();
		InitializeConstraints();
		InitializeCellSelectors();

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

void UWFCGenerator::InitializeConstraints()
{
	Constraints.Reset();
	for (const TSubclassOf<UWFCConstraint>& ConstraintClass : Config.ConstraintClasses)
	{
		if (!ConstraintClass)
		{
			continue;
		}

		// create the new constraint object
		UWFCConstraint* Constraint = NewObject<UWFCConstraint>(this, ConstraintClass);
		check(Constraint != nullptr);
		Constraints.Add(Constraint);
	}

	// initialize once all constraints are constructed in case they need to reference each other
	for (UWFCConstraint* Constraint : Constraints)
	{
		Constraint->Initialize(this);
	}
}

void UWFCGenerator::InitializeCellSelectors()
{
	CellSelectors.Reset();
	for (TSubclassOf<UWFCCellSelector> CellSelectorClass : Config.CellSelectorClasses)
	{
		if (!CellSelectorClass)
		{
			continue;
		}

		UWFCCellSelector* NewSelector = NewObject<UWFCCellSelector>(this, CellSelectorClass);
		check(NewSelector != nullptr);
		CellSelectors.Add(NewSelector);
	}

	// initialize once all selectors are constructed in case they need to reference each other
	for (UWFCCellSelector* CellSelector : CellSelectors)
	{
		CellSelector->Initialize(this);
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

	SET_DWORD_STAT(STAT_WFCGeneratorNumCells, NumCells);
	SET_DWORD_STAT(STAT_WFCGeneratorNumCellsSelected, 0);
}

bool UWFCGenerator::AreAllCellsSelected() const
{
	// TODO: cache
	for (const FWFCCell& Cell : Cells)
	{
		if (!Cell.HasSelection())
		{
			return false;
		}
	}
	return true;
}

void UWFCGenerator::Reset()
{
	InitializeCells();

	for (UWFCConstraint* Constraint : Constraints)
	{
		Constraint->Reset();
	}

	SetState(EWFCGeneratorState::None);
}

void UWFCGenerator::Run(int32 StepLimit)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogWFC, Warning, TEXT("Initialize must be called before Run on a WFCGenerator"));
		return;
	}

	SCOPE_LOG_TIME(TEXT("UWFCGenerator::Run"), nullptr);

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

void UWFCGenerator::Next(bool bBreakAfterConstraints)
{
	if (State == EWFCGeneratorState::Finished)
	{
		return;
	}

	SCOPE_CYCLE_COUNTER(STAT_WFCGeneratorNext);
	bDidSelectCellThisStep = false;
	CellsAffectedThisUpdate.Reset();

	CurrentStepPhase = EWFCGeneratorStepPhase::Constraints;

	// update all constraints, which may lead to cell selection
	bool bDidApplyConstraints = false;
	for (UWFCConstraint* Constraint : Constraints)
	{
		NumBansThisUpdate = 0;
		if (Constraint->Next())
		{
			UE_LOG(LogWFC, VeryVerbose, TEXT("Applied constraint: %s, bans: %d"), *Constraint->GetName(), NumBansThisUpdate);
			bDidApplyConstraints = true;
			if (State == EWFCGeneratorState::Finished)
			{
				return;
			}
			SetState(EWFCGeneratorState::InProgress);

			if (bDidSelectCellThisStep)
			{
				// stop immediately if the constraint caused a cell selection
				return;
			}
		}
	}

	if (bBreakAfterConstraints && bDidApplyConstraints)
	{
		return;
	}

	CurrentStepPhase = EWFCGeneratorStepPhase::Selection;

	// select a cell to observe
	const FWFCCellIndex CellIndex = SelectNextCellIndex();

	if (CellIndex == INDEX_NONE)
	{
		UE_LOG(LogWFC, Verbose, TEXT("Failed to select a cell"));
		SetState(EWFCGeneratorState::Error);
		return;
	}

	// select the tile for the cell
	const FWFCTileId TileId = SelectNextTileForCell(CellIndex);

	if (TileId == INDEX_NONE)
	{
		UE_LOG(LogWFC, Verbose, TEXT("Failed to select a tile"));
		SetState(EWFCGeneratorState::Error);
		return;
	}

	Select(CellIndex, TileId);
	if (State == EWFCGeneratorState::Finished)
	{
		return;
	}

	SetState(EWFCGeneratorState::InProgress);
}

void UWFCGenerator::Ban(int32 CellIndex, int32 TileId)
{
	if (IsValidCellIndex(CellIndex))
	{
		++NumBansThisUpdate;
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

			if (bWasChanged)
			{
				++NumBansThisUpdate;
			}
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

FString UWFCGenerator::GetTileDebugString(int32 TileId) const
{
	if (Config.Model.IsValid())
	{
		if (const FWFCModelTile* ModelTile = Config.Model->GetTile(TileId))
		{
			return ModelTile->ToString();
		}
	}
	return FString();
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

int32 UWFCGenerator::GetNumCellCandidates(int32 CellIndex) const
{
	if (IsValidCellIndex(CellIndex))
	{
		return GetCell(CellIndex).TileCandidates.Num();
	}
	return 0;
}

void UWFCGenerator::OnCellChanged(FWFCCellIndex CellIndex)
{
	CellsAffectedThisUpdate.AddUnique(CellIndex);

	FWFCCell& Cell = GetCell(CellIndex);
	const bool bHasSelection = Cell.HasSelection();
	if (bHasSelection)
	{
		Cell.CollapsePhase = CurrentStepPhase;

		UE_LOG(LogWFC, VeryVerbose, TEXT("Cell %s collapsed to %s during %s phase."),
		       *Grid->GetCellName(CellIndex),
		       *GetModel()->GetTileDebugString(GetCell(CellIndex).GetSelectedTileId()),
		       Cell.CollapsePhase == EWFCGeneratorStepPhase::Constraints ? TEXT("Constraints") : TEXT("Selection"));

		INC_DWORD_STAT(STAT_WFCGeneratorNumCellsSelected);
		bDidSelectCellThisStep = true;
		OnCellSelected.Broadcast(CellIndex);
	}

	for (UWFCConstraint* Constraint : Constraints)
	{
		Constraint->NotifyCellChanged(CellIndex, bHasSelection);
	}

	if (AreAllCellsSelected())
	{
		SetState(EWFCGeneratorState::Finished);
	}
}

FWFCCellIndex UWFCGenerator::SelectNextCellIndex()
{
	// TODO: why would one cell selector not be used? how do we define phases of selection?
	for (UWFCCellSelector* CellSelector : CellSelectors)
	{
		const FWFCCellIndex CellIndex = CellSelector->SelectNextCell();

		if (CellIndex != INDEX_NONE)
		{
			return CellIndex;
		}
	}
	return INDEX_NONE;
}

FWFCTileId UWFCGenerator::SelectNextTileForCell(FWFCCellIndex CellIndex)
{
	const FWFCCell& Cell = GetCell(CellIndex);

	if (Cell.HasNoCandidates())
	{
		return INDEX_NONE;
	}

	// select a candidate, applying weighted probabilities
	float TotalWeight = 0.f;
	TArray<float> TileWeights;
	for (const FWFCTileId& TileId : Cell.TileCandidates)
	{
		const float TileWeight = Config.Model->GetTileWeightUnchecked(TileId);
		TileWeights.Add(TileWeight);
		TotalWeight += TileWeight;
	}

	float Rand = FMath::FRand() * TotalWeight;
	for (int32 Idx = 0; Idx < Cell.TileCandidates.Num(); ++Idx)
	{
		if (Rand >= TileWeights[Idx])
		{
			Rand -= TileWeights[Idx];
		}
		else
		{
			return Cell.TileCandidates[Idx];
		}
	}

	return Cell.TileCandidates[0];
}
