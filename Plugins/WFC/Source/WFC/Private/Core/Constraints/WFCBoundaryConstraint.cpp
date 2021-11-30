// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCBoundaryConstraint.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"


void UWFCBoundaryConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	bDidApplyInitialConstraint = false;

	Grid = Generator->GetGrid();
}

void UWFCBoundaryConstraint::AddProhibitedAdjacentBoundaryMapping(FWFCTileId TileId, FWFCGridDirection Direction)
{
	TileBoundaryProhibitionMap.FindOrAdd(TileId).AddUnique(Direction);
}

bool UWFCBoundaryConstraint::CanTileBeAdjacentToBoundaries(FWFCTileId TileId, const TArray<FWFCGridDirection>& BoundaryDirections) const
{
	const TArray<FWFCGridDirection>* ProhibitedDirectionsPtr = TileBoundaryProhibitionMap.Find(TileId);
	if (ProhibitedDirectionsPtr)
	{
		for (const FWFCGridDirection& Direction : BoundaryDirections)
		{
			if (ProhibitedDirectionsPtr->Contains(Direction))
			{
				return false;
			}
		}
	}
	return true;
}

bool UWFCBoundaryConstraint::Next()
{
	if (bDidApplyInitialConstraint)
	{
		// already applied the constraint
		return false;
	}

	// iterate all cells and ban tiles that can't be next to boundaries

	const int32 NumDirections = Grid->GetNumDirections();

	for (FWFCCellIndex CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
	{
		// this cell is adjacent to the boundary for the outgoing Direction
		const FWFCCell& CellToCheck = Generator->GetCell(CellIndex);
		if (CellToCheck.HasSelection())
		{
			// don't change cells that are already selected
			continue;
		}

		// find all boundary directions first to reduce some iterations
		TArray<FWFCGridDirection> BoundaryDirections;
		for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
		{
			const FWFCCellIndex AdjacentCellIndex = Grid->GetCellIndexInDirection(CellIndex, Direction);
			if (!Grid->IsValidCellIndex(AdjacentCellIndex))
			{
				BoundaryDirections.AddUnique(Direction);
			}
		}

		if (BoundaryDirections.Num() > 0)
		{
			// cell is next to one or more boundaries
			TArray<FWFCTileId> TileIdsToBan;
			for (int32 Idx = 0; Idx < CellToCheck.TileCandidates.Num(); ++Idx)
			{
				// check each tile for any prohibited boundary directions
				const FWFCTileId& TileId = CellToCheck.TileCandidates[Idx];

				if (!CanTileBeAdjacentToBoundaries(TileId, BoundaryDirections))
				{
					TileIdsToBan.AddUnique(TileId);
				}
			}

			if (TileIdsToBan.Num() > 0)
			{
				UE_LOG(LogWFC, VeryVerbose, TEXT("Banning %d tile(s) from Cell due to boundary constraints"),
				       TileIdsToBan.Num(), *Grid->GetCellName(CellIndex));

				Generator->BanMultiple(CellIndex, TileIdsToBan);
			}
		}
	}

	bDidApplyInitialConstraint = true;

	return false;
}
