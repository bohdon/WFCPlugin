// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCAdjacencyConstraint.h"

#include "DrawDebugHelpers.h"
#include "WFCModule.h"
#include "WFCStatics.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"


DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Adjacency Constraint - Mappings"), STAT_WFCAdjacencyConstraintMappings, STATGROUP_WFC);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Adjacency Constraint - Time (ms)"), STAT_WFCAdjacencyConstraintTime, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Adjacency Constraint - Checks"), STAT_WFCAdjacencyConstraintNumChecks, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Adjacency Constraint - Bans"), STAT_WFCAdjacencyConstraintNumBans, STATGROUP_WFC);

TAutoConsoleVariable<bool> CVarAdjacencyConstraintDrawDebug(
	TEXT("wfc.AdjacencyConstraint.DrawDebug"), false,
	TEXT("Draw debug info for adjacency constraints"));


UWFCAdjacencyConstraintConfig::UWFCAdjacencyConstraintConfig()
	: bIgnoreContradictionCells(false),
	  bDebugNext(false)
{
}

TSubclassOf<UWFCConstraint> UWFCAdjacencyConstraintConfig::GetConstraintClass() const
{
	return UWFCAdjacencyConstraint::StaticClass();
}

void UWFCAdjacencyConstraintConfig::Configure(UWFCConstraint* Constraint) const
{
	Super::Configure(Constraint);

	UWFCAdjacencyConstraint* AdjacencyConstraint = Cast<UWFCAdjacencyConstraint>(Constraint);
	check(AdjacencyConstraint != nullptr);

	AdjacencyConstraint->bIgnoreContradictionCells = bIgnoreContradictionCells;
	AdjacencyConstraint->bDebugNext = bDebugNext;

	// TODO: add adjacency mappings
}


UWFCAdjacencyConstraint::UWFCAdjacencyConstraint()
	: bIgnoreContradictionCells(false),
	  bDebugNext(false)
{
}

void UWFCAdjacencyConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	Grid = Generator->GetGrid();

	SET_DWORD_STAT(STAT_WFCAdjacencyConstraintMappings, 0);
	SET_FLOAT_STAT(STAT_WFCAdjacencyConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCAdjacencyConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCAdjacencyConstraintNumBans, 0);
}

void UWFCAdjacencyConstraint::Reset()
{
	Super::Reset();

	AdjacentCellDirsToCheck.Reset();

	SET_FLOAT_STAT(STAT_WFCAdjacencyConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCAdjacencyConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCAdjacencyConstraintNumBans, 0);
}

void UWFCAdjacencyConstraint::AddAdjacentTileMapping(FWFCTileId TileId, FWFCGridDirection Direction, FWFCTileId AllowedTileId)
{
	INC_DWORD_STAT(STAT_WFCAdjacencyConstraintMappings);
	TileAdjacencyMap.FindOrAdd(TileId).AllowedTiles.FindOrAdd(Direction).AddUnique(AllowedTileId);
}

TArray<FWFCTileId> UWFCAdjacencyConstraint::GetValidAdjacentTileIds(FWFCTileId TileId, FWFCGridDirection Direction) const
{
	return TileAdjacencyMap.FindRef(TileId).AllowedTiles.FindRef(Direction);
}

void UWFCAdjacencyConstraint::MarkCellForAdjacencyCheck(FWFCCellIndex Index)
{
	// apply adjacency constraints
	const int32 NumDirections = Grid->GetNumDirections();
	for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
	{
		AdjacentCellDirsToCheck.AddUnique(FWFCCellIndexAndDirection(Index, Direction));
	}
}

void UWFCAdjacencyConstraint::NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection)
{
	// track the changed cell for changes to propagate during the next update
	const int32 NumDirections = Grid->GetNumDirections();

	for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
	{
		AdjacentCellDirsToCheck.AddUnique(FWFCCellIndexAndDirection(CellIndex, Direction));
	}
}

bool UWFCAdjacencyConstraint::Next()
{
	STAT(const double StartTime = FPlatformTime::Seconds());
	SET_FLOAT_STAT(STAT_WFCAdjacencyConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCAdjacencyConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCAdjacencyConstraintNumBans, 0);

	bool bDidMakeChanges = false;

	// go through pending list of changes to check, banning any tile candidates
	// that no longer match adjacency rules, and return true if a cell gets selected
	while (AdjacentCellDirsToCheck.Num() > 0)
	{
		const FWFCCellIndexAndDirection CellDir = AdjacentCellDirsToCheck[0];
		check(Grid->IsValidCellIndex(CellDir.CellIndex));
		AdjacentCellDirsToCheck.RemoveAt(0);

		const FWFCCell& ChangedCell = Generator->GetCell(CellDir.CellIndex);

		if (bIgnoreContradictionCells && ChangedCell.HasNoCandidates())
		{
			// treat cells with no candidates as empty spaces
			continue;
		}

		// find the cell in the direction to check
		const FWFCCellIndex CellIndexToCheck = Grid->GetCellIndexInDirection(CellDir.CellIndex, CellDir.Direction);
		if (!Grid->IsValidCellIndex(CellIndexToCheck))
		{
			continue;
		}

		const FWFCCell& CellToCheck = Generator->GetCell(CellIndexToCheck);
		if (CellToCheck.HasSelection())
		{
			// don't change cells that are already selected
			continue;
		}

		// check all candidates and ban any that don't pass the constraint
		TArray<FWFCTileId> TileIdsToBan;
		for (int32 Idx = 0; Idx < CellToCheck.TileCandidates.Num(); ++Idx)
		{
			INC_DWORD_STAT(STAT_WFCAdjacencyConstraintNumChecks);

			const FWFCTileId& TileId = CellToCheck.TileCandidates[Idx];
			TArray<FWFCTileId> AllowedIncomingTileIds = GetValidAdjacentTileIds(TileId, CellDir.Direction);
			if (!ChangedCell.HasAnyMatchingCandidate(AllowedIncomingTileIds))
			{
				TileIdsToBan.Add(TileId);
			}
		}
		if (TileIdsToBan.Num() > 0)
		{
#if ENABLE_DRAW_DEBUG
			if (CVarAdjacencyConstraintDrawDebug.GetValueOnAnyThread())
			{
				const float DebugInterval = CVarWFCDebugStepInterval.GetValueOnAnyThread();
				FVector CellALocation = Grid->GetCellWorldLocation(CellDir.CellIndex, true);
				FVector CellBLocation = Grid->GetCellWorldLocation(CellIndexToCheck, true);
				DrawDebugLine(GetWorld(), CellALocation, CellBLocation, FColor::White, false, DebugInterval, 0, 5.f);
				DrawDebugPoint(GetWorld(), CellBLocation, 10, FColor::Red, false, DebugInterval, 0);
			}
#endif

			Generator->BanMultiple(CellIndexToCheck, TileIdsToBan);
			INC_DWORD_STAT_BY(STAT_WFCAdjacencyConstraintNumBans, TileIdsToBan.Num());
			bDidMakeChanges = true;
		}

		if (bDidMakeChanges && bDebugNext)
		{
			break;
		}
	}

	INC_FLOAT_STAT_BY(STAT_WFCAdjacencyConstraintTime, (FPlatformTime::Seconds() - StartTime) * 1000);
	return bDidMakeChanges;
}
