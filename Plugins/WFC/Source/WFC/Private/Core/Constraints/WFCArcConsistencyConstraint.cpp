// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCArcConsistencyConstraint.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"


DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Arc Consistency - Allowed Tile Entries"), STAT_WFCArcConsistencyEntries, STATGROUP_WFC);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Arc Consistency - Update Time (ms)"), STAT_WFCArcConstraintTime, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Arc Consistency - Checks"), STAT_WFCArcConstraintNumChecks, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Arc Consistency - Bans"), STAT_WFCArcConstraintNumBans, STATGROUP_WFC);


UWFCArcConsistencyConstraint::UWFCArcConsistencyConstraint()
	: bIgnoreContradictionCells(false),
	  bDebugNext(false)
{
}

void UWFCArcConsistencyConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	SET_DWORD_STAT(STAT_WFCArcConsistencyEntries, 0);
	SET_FLOAT_STAT(STAT_WFCArcConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumBans, 0);

	// initialize allowed tiles to empty list for each combination of [tile][direction].
	AllowedTiles.AddZeroed(Model->GetNumTiles());
	for (int32 TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
	{
		AllowedTiles[TileId].AddZeroed(Grid->GetNumDirections());
	}

	// initialize support counts to 0 for each [cell][tile][direction]
	SupportCounts.AddZeroed(Grid->GetNumCells());
	for (int32 CellIndex = 0; CellIndex < Grid->GetNumCells(); ++CellIndex)
	{
		SupportCounts[CellIndex].AddZeroed(Model->GetNumTiles());
		for (int32 TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
		{
			SupportCounts[CellIndex][TileId].AddZeroed(Grid->GetNumDirections());
		}
	}
}

void UWFCArcConsistencyConstraint::Reset()
{
	Super::Reset();

	bDidApplyInitialConstraint = false;
	AdjacentCellDirsToCheck.Reset();

	SET_DWORD_STAT(STAT_WFCArcConsistencyEntries, 0);
	SET_FLOAT_STAT(STAT_WFCArcConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumBans, 0);
}

void UWFCArcConsistencyConstraint::AddAllowedTileForDirection(FWFCTileId TileId, FWFCGridDirection Direction, FWFCTileId AllowedTileId)
{
	INC_DWORD_STAT(STAT_WFCArcConsistencyEntries);
	AllowedTiles[TileId][Direction].AddUnique(AllowedTileId);
}

TArray<FWFCTileId> UWFCArcConsistencyConstraint::GetValidAdjacentTileIds(FWFCTileId TileId, FWFCGridDirection Direction) const
{
	return AllowedTiles[TileId][Direction];
}

void UWFCArcConsistencyConstraint::MarkCellForAdjacencyCheck(FWFCCellIndex CellIndex)
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

void UWFCArcConsistencyConstraint::NotifyCellBan(FWFCCellIndex CellIndex, FWFCTileId BannedTileId)
{
	// TODO: propagate changes only affected by the banned tile id
	MarkCellForAdjacencyCheck(CellIndex);
}

bool UWFCArcConsistencyConstraint::Next()
{
	STAT(const double StartTime = FPlatformTime::Seconds());
	SET_FLOAT_STAT(STAT_WFCArcConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCArcConstraintNumBans, 0);

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
			INC_DWORD_STAT(STAT_WFCArcConstraintNumChecks);

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
			INC_DWORD_STAT_BY(STAT_WFCArcConstraintNumBans, TileIdsToBan.Num());
			bDidMakeChanges = true;
		}

		if (bDebugNext)
		{
			break;
		}
	}

	INC_FLOAT_STAT_BY(STAT_WFCArcConstraintTime, (FPlatformTime::Seconds() - StartTime) * 1000);

	if (bDebugNext)
	{
		// always stop when debug next is enabled
		return true;
	}
	return bDidMakeChanges;
}

void UWFCArcConsistencyConstraint::LogDebugInfo() const
{
	Super::LogDebugInfo();

	UE_LOG(LogWFC, Verbose, TEXT("%s AllowedTiles allocated size: %.3fKB"),
	       *GetClass()->GetName(), AllowedTiles.GetAllocatedSize() / 1024.f);
	UE_LOG(LogWFC, Verbose, TEXT("%s SupportCounts allocated size: %.3fKB"),
	       *GetClass()->GetName(), SupportCounts.GetAllocatedSize() / 1024.f);


	if (!Model)
	{
		return;
	}

	for (FWFCTileId TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
	{
		const FString TileStr = Model->GetTileDebugString(TileId);
		UE_LOG(LogWFC, VeryVerbose, TEXT("%s allowed tiles..."), *TileStr);

		const auto& AllowedDirections = AllowedTiles[TileId];
		for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
		{
			const FString DirectionStr = Grid->GetDirectionName(Direction);

			TArray<FString> TileStrs;
			const auto& ThisAllowedTiles = AllowedDirections[Direction];
			for (const int32& AllowedTileId : ThisAllowedTiles)
			{
				TileStrs.Add(FString::FromInt(AllowedTileId));
			}

			UE_LOG(LogWFC, VeryVerbose, TEXT("    %s -> %s"), *DirectionStr, *FString::Join(TileStrs, TEXT(", ")));
		}
	}
}
