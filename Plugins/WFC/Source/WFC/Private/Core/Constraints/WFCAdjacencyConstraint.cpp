// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCAdjacencyConstraint.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"


DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Adjacency Constraint - Mappings"), STAT_WFCAdjacencyConstraintMappings, STATGROUP_WFC);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Adjacency Constraint - Time (ms)"), STAT_WFCAdjacencyConstraintTime, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Adjacency Constraint - Checks"), STAT_WFCAdjacencyConstraintNumChecks, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Adjacency Constraint - Bans"), STAT_WFCAdjacencyConstraintNumBans, STATGROUP_WFC);


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

void UWFCAdjacencyConstraint::MarkCellForAdjacencyCheck(FWFCCellIndex CellIndex)
{
	// apply adjacency constraints
	const int32 NumDirections = Grid->GetNumDirections();
	for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
	{
		FWFCCellIndexAndDirection CellDir(CellIndex, Direction);
		if (!AdjacentCellDirsToCheck.Contains(CellDir))
		{
			AdjacentCellDirsToCheck.Insert(CellDir, 0);
		}
	}
}

void UWFCAdjacencyConstraint::NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection)
{
	MarkCellForAdjacencyCheck(CellIndex);
}

bool UWFCAdjacencyConstraint::Next()
{
	STAT(const double StartTime = FPlatformTime::Seconds());
	SET_FLOAT_STAT(STAT_WFCAdjacencyConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCAdjacencyConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCAdjacencyConstraintNumBans, 0);

	bool bDidMakeChanges = false;
	AdjacenciesEnforcedThisUpdate.Reset();

	if (AdjacentCellDirsToCheck.IsEmpty())
	{
		// early out
		return false;
	}

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
			AdjacenciesEnforcedThisUpdate.Add(CellDir.CellIndex, CellIndexToCheck);

			Generator->BanMultiple(CellIndexToCheck, TileIdsToBan);
			INC_DWORD_STAT_BY(STAT_WFCAdjacencyConstraintNumBans, TileIdsToBan.Num());
			bDidMakeChanges = true;
		}

		if (bDebugNext)
		{
			break;
		}
	}

	INC_FLOAT_STAT_BY(STAT_WFCAdjacencyConstraintTime, (FPlatformTime::Seconds() - StartTime) * 1000);

	if (bDebugNext)
	{
		// always stop when debug next is enabled
		return true;
	}
	return bDidMakeChanges;
}

void UWFCAdjacencyConstraint::LogDebugInfo() const
{
	const float MemSize = TileAdjacencyMap.GetAllocatedSize() / 1024.f;
	UE_LOG(LogWFC, Verbose, TEXT("%s mappings: %d (%0.2fKB)"), *GetClass()->GetName(), TileAdjacencyMap.Num(), MemSize);

	const UWFCModel* Model = GetModel();
	if (!Model)
	{
		return;
	}

	for (auto& Elem : TileAdjacencyMap)
	{
		const FString TileStr = Model->GetTileDebugString(Elem.Key);
		UE_LOG(LogWFC, VeryVerbose, TEXT("%s allowed tiles..."), *TileStr);

		for (auto& MappingElem : Elem.Value.AllowedTiles)
		{
			const FString DirectionStr = Grid->GetDirectionName(MappingElem.Key);

			TArray<FString> TileStrs;
			for (const int32& TileId : MappingElem.Value)
			{
				TileStrs.Add(FString::FromInt(TileId));
			}

			UE_LOG(LogWFC, VeryVerbose, TEXT("    %s -> %s"), *DirectionStr, *FString::Join(TileStrs, TEXT(", ")));
		}
	}
}
