// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCAdjacencyConstraint.h"

#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"


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
		// (iterate in reverse to support banning)
		for (int32 Idx = CellToCheck.TileCandidates.Num() - 1; Idx >= 0; --Idx)
		{
			const FWFCTileId& TileId = CellToCheck.TileCandidates[Idx];
			TArray<FWFCTileId> AllowedIncomingTileIds = GetValidAdjacentTileIds(TileId, CellDir.Direction);
			if (!ChangedCell.HasAnyMatchingCandidate(AllowedIncomingTileIds))
			{
				Generator->Ban(CellIndexToCheck, TileId);
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
