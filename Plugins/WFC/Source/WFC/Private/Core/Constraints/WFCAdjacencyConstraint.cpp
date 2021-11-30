// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCAdjacencyConstraint.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"


UWFCAdjacencyConstraint::UWFCAdjacencyConstraint()
	: bIgnoreContradictionCells(false)
{
}

void UWFCAdjacencyConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	Grid = Generator->GetGrid();
}

void UWFCAdjacencyConstraint::AddAdjacentTileMapping(FWFCTileId TileId, FWFCGridDirection Direction, FWFCTileId AllowedTileId)
{
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

void UWFCAdjacencyConstraint::NotifyCellChanged(FWFCCellIndex CellIndex)
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
			const FWFCTileId& TileId = CellToCheck.TileCandidates[Idx];
			TArray<FWFCTileId> AllowedIncomingTileIds = GetValidAdjacentTileIds(TileId, CellDir.Direction);
			if (!ChangedCell.HasAnyMatchingCandidate(AllowedIncomingTileIds))
			{
				TileIdsToBan.Add(TileId);
			}
		}
		if (TileIdsToBan.Num() > 0)
		{
			UE_LOG(LogWFC, VeryVerbose, TEXT("Banning %d tile(s) from Cell: %s after change in adjacent Cell: %s (Direction: %s)"),
			       TileIdsToBan.Num(), *Grid->GetCellName(CellIndexToCheck),
			       *Grid->GetCellName(CellDir.CellIndex), *Grid->GetDirectionName(CellDir.Direction));

			Generator->BanMultiple(CellIndexToCheck, TileIdsToBan);
		}

		// this next call is finished if any cell was selected
		if (CellToCheck.HasSelection())
		{
			// return true;
		}
	}
	return false;
}
