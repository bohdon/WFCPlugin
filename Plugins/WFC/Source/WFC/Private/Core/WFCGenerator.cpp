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


// UWFCGenerator
// -------------

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

void UWFCGenerator::Configure(FWFCGeneratorConfig InConfig)
{
	Config = InConfig;
}

void UWFCGenerator::Initialize(bool bFull)
{
	if (bIsInitialized)
	{
		return;
	}

	SCOPE_LOG_TIME_FUNC();

	// TODO: cache in WFCAsset snapshot, and then put this behind bFull
	Config.Model->GenerateTiles();
	NumTiles = Config.Model->GetNumTiles();
	SET_DWORD_STAT(STAT_WFCGeneratorNumTiles, NumTiles);

	InitializeGrid(Config.GridConfig.Get());
	InitializeCells();

	CreateConstraints();
	CreateCellSelectors();

	if (bFull)
	{
		InitializeConstraints();
	}
	InitializeCellSelectors();

	SetState(EWFCGeneratorState::InProgress);

	bIsInitialized = true;
}

void UWFCGenerator::InitializeGrid(const UWFCGridConfig* GridConfig)
{
	check(GridConfig != nullptr);
	Grid = UWFCGrid::NewGrid(this, GridConfig);
	check(Grid != nullptr);
}

void UWFCGenerator::CreateConstraints()
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
}

void UWFCGenerator::InitializeConstraints()
{
	for (UWFCConstraint* Constraint : Constraints)
	{
		Constraint->Initialize(this);
	}
}

void UWFCGenerator::CreateCellSelectors()
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
}

void UWFCGenerator::InitializeCellSelectors()
{
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

	bDidSelectCellThisStep = false;
	NumBansThisUpdate = 0;
	CurrentStepPhase = EWFCGeneratorStepPhase::None;
	CellsAffectedThisUpdate.Reset();
	SetState(EWFCGeneratorState::None);
}

void UWFCGenerator::Run(int32 StepLimit)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogWFC, Error, TEXT("Initialize must be called before Run on a WFCGenerator"));
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

void UWFCGenerator::RunStartup(int32 StepLimit)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogWFC, Error, TEXT("Initialize must be called before RunStartup on a WFCGenerator"));
		return;
	}

	SCOPE_LOG_TIME(TEXT("UWFCGenerator::RunStartup"), nullptr);

	StepGranularity = EWFCGeneratorStepGranularity::None;

	for (int32 Step = 0; Step < StepLimit; ++Step)
	{
		Next(true);

		if (CurrentStepPhase == EWFCGeneratorStepPhase::Selection)
		{
			return;
		}

		if (State != EWFCGeneratorState::InProgress &&
			State != EWFCGeneratorState::None)
		{
			break;
		}
	}
}

void UWFCGenerator::Next(bool bNoSelection)
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
			UE_LOG(LogWFC, Verbose, TEXT("Applied constraint: %s, bans: %d"), *Constraint->GetName(), NumBansThisUpdate);

			bDidApplyConstraints = true;
			if (State == EWFCGeneratorState::Finished || State == EWFCGeneratorState::Error)
			{
				return;
			}

			SetState(EWFCGeneratorState::InProgress);

			if (bDidSelectCellThisStep && StepGranularity >= EWFCGeneratorStepGranularity::ConstraintCollapse)
			{
				// stop immediately if the constraint caused a cell selection
				return;
			}

			if (StepGranularity >= EWFCGeneratorStepGranularity::EachConstraint)
			{
				// break after every constraint that did work
				return;
			}
		}
	}

	if (StepGranularity >= EWFCGeneratorStepGranularity::Constraints && bDidApplyConstraints)
	{
		return;
	}

	CurrentStepPhase = EWFCGeneratorStepPhase::Selection;

	if (bNoSelection)
	{
		return;
	}

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

	if (State == EWFCGeneratorState::Finished || State == EWFCGeneratorState::Error)
	{
		return;
	}

	SetState(EWFCGeneratorState::InProgress);
}

bool UWFCGenerator::Ban(int32 CellIndex, int32 TileId)
{
	if (IsValidCellIndex(CellIndex))
	{
		++NumBansThisUpdate;
		FWFCCell& Cell = GetCell(CellIndex);
		if (Cell.RemoveCandidate(TileId))
		{
			OnCellCandidateBanned(CellIndex, TileId);
		}

		return Cell.HasNoCandidates();
	}
	return false;
}

bool UWFCGenerator::BanMultiple(int32 CellIndex, TArray<int32> TileIds)
{
	bool bIsContradiction = false;
	if (IsValidCellIndex(CellIndex) && TileIds.Num() > 0)
	{
		TArray<FWFCTileId> BannedTileIds;
		BannedTileIds.Reserve(TileIds.Num());

		FWFCCell& Cell = GetCell(CellIndex);
		for (const int32& TileId : TileIds)
		{
			if (Cell.RemoveCandidate(TileId))
			{
				BannedTileIds.Add(TileId);
				++NumBansThisUpdate;
			}

			bIsContradiction |= Cell.HasNoCandidates();
		}

		if (!BannedTileIds.IsEmpty())
		{
			OnCellCandidatesBanned(CellIndex, BannedTileIds);
		}
	}
	return bIsContradiction;
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

UWFCGeneratorSnapshot* UWFCGenerator::CreateSnapshot(UObject* Outer) const
{
	UWFCGeneratorSnapshot* Snapshot = NewObject<UWFCGeneratorSnapshot>(Outer);
	Snapshot->Cells = Cells;

	for (const UWFCConstraint* Constraint : Constraints)
	{
		UWFCConstraintSnapshot* ConstraintSnapshot = Constraint->CreateSnapshot(Snapshot);
		if (ConstraintSnapshot)
		{
			Snapshot->ConstraintSnapshots.Add(Constraint->GetClass(), ConstraintSnapshot);
		}
	}

	return Snapshot;
}

void UWFCGenerator::ApplySnapshot(const UWFCGeneratorSnapshot* Snapshot)
{
	if (!Snapshot)
	{
		return;
	}

	if (Snapshot->Cells.Num() == !Cells.Num())
	{
		UE_LOG(LogWFC, Error, TEXT("Snapshot does not match cell count: %s"), *Snapshot->GetFullName(Snapshot->GetOuter()));
		return;
	}

	Cells = Snapshot->Cells;

	for (UWFCConstraint* Constraint : Constraints)
	{
		const UWFCConstraintSnapshot* ConstraintSnapshot = Snapshot->ConstraintSnapshots.FindRef(Constraint->GetClass());
		if (ConstraintSnapshot)
		{
			Constraint->ApplySnapshot(ConstraintSnapshot);
			UE_LOG(LogWFC, Verbose, TEXT("Applied constraint snapshot: %s"), *Constraint->GetName());
		}
	}

	UE_LOG(LogWFC, Log, TEXT("Applied snapshot: %s"), *Snapshot->GetFullName(Snapshot->GetOuter()));
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

void UWFCGenerator::OnCellCandidateBanned(FWFCCellIndex CellIndex, FWFCTileId BannedTileId)
{
	for (UWFCConstraint* Constraint : Constraints)
	{
		Constraint->NotifyCellBan(CellIndex, BannedTileId);
	}

	OnCellChanged(CellIndex);
}

void UWFCGenerator::OnCellCandidatesBanned(FWFCCellIndex CellIndex, const TArray<FWFCTileId>& BannedTileIds)
{
	for (UWFCConstraint* Constraint : Constraints)
	{
		for (const FWFCTileId& BannedTileId : BannedTileIds)
		{
			Constraint->NotifyCellBan(CellIndex, BannedTileId);
		}
	}

	OnCellChanged(CellIndex);
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
	else if (Cell.HasNoCandidates())
	{
		// contradiction
		SetState(EWFCGeneratorState::Error);
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

	if (FMath::IsNearlyZero(TotalWeight))
	{
		// no weights, treat all equally
		const int32 Idx = FMath::RandHelper(Cell.TileCandidates.Num());

		UE_LOG(LogWFC, Verbose, TEXT("Selected tile %s out of %d candidates, with 0 total weight."),
		       *GetModel()->GetTileDebugString(Cell.TileCandidates[Idx]), Cell.TileCandidates.Num());

		return Cell.TileCandidates[Idx];
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
			UE_LOG(LogWFC, Verbose, TEXT("Selected tile %s out of %d candidates. (Weight: %f, Probability: %f%%)"),
			       *GetModel()->GetTileDebugString(Cell.TileCandidates[Idx]), Cell.TileCandidates.Num(),
			       TileWeights[Idx], (TileWeights[Idx] / TotalWeight) * 100.f);

			return Cell.TileCandidates[Idx];
		}
	}

	return Cell.TileCandidates[0];
}
