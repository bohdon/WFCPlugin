// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCBoundaryConstraint.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"


DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Boundary Constraint - Mappings"), STAT_WFCBoundaryConstraintMappings, STATGROUP_WFC);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Boundary Constraint - Time (ms)"), STAT_WFCBoundaryConstraintTime, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Boundary Constraint - Checks"), STAT_WFCBoundaryConstraintNumChecks, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Boundary Constraint - Bans"), STAT_WFCBoundaryConstraintNumBans, STATGROUP_WFC);


void UWFCBoundaryConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	bDidApplyInitialConstraint = false;

	SCOPE_LOG_TIME_FUNC();
	SET_DWORD_STAT(STAT_WFCBoundaryConstraintMappings, 0);
	SET_FLOAT_STAT(STAT_WFCBoundaryConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCBoundaryConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCBoundaryConstraintNumBans, 0);

	const int32 NumDirections = Grid->GetNumDirections();

	for (FWFCTileId TileId = 0; TileId <= Model->GetMaxTileId(); ++TileId)
	{
		const FWFCModelAssetTile& Tile = Model->GetTileRef<FWFCModelAssetTile>(TileId);

		for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
		{
			if (!CanAssetTileBeNextToGridBoundary(Tile, Direction))
			{
				AddProhibitedAdjacentBoundaryMapping(TileId, Direction);
			}
		}
	}
}

void UWFCBoundaryConstraint::Reset()
{
	Super::Reset();

	bDidApplyInitialConstraint = false;

	SET_FLOAT_STAT(STAT_WFCBoundaryConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCBoundaryConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCBoundaryConstraintNumBans, 0);
}

void UWFCBoundaryConstraint::AddProhibitedAdjacentBoundaryMapping(FWFCTileId TileId, FWFCGridDirection Direction)
{
	INC_DWORD_STAT(STAT_WFCBoundaryConstraintMappings);
	TileBoundaryProhibitionMap.FindOrAdd(TileId).AddUnique(Direction);
}

bool UWFCBoundaryConstraint::CanAssetTileBeNextToGridBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction) const
{
	const UWFCTileAsset* TileAsset = Tile.TileAsset.Get();
	if (!TileAsset)
	{
		return false;
	}

	if (TileAsset->IsInteriorEdge(Tile.TileDefIndex, Direction))
	{
		// for large tiles, only exterior edges can be against the boundary
		return false;
	}

	// check edge type
	const FGameplayTag EdgeType = TileAsset->GetTileDefEdgeType(Tile.TileDefIndex, Direction);
	const FGameplayTagContainer EdgeTypeTags(EdgeType);
	if (!EdgeTypeQuery.IsEmpty() && !EdgeTypeQuery.Matches(EdgeTypeTags))
	{
		return false;
	}

	return true;
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
	STAT(const double StartTime = FPlatformTime::Seconds());
	SET_FLOAT_STAT(STAT_WFCBoundaryConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCBoundaryConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCBoundaryConstraintNumBans, 0);

	if (bDidApplyInitialConstraint)
	{
		// already applied the constraint
		return false;
	}

	bool bDidMakeChanges = false;

	// iterate all cells and ban tiles that can't be next to boundaries

	const int32 NumDirections = Grid->GetNumDirections();

	if (CachedTileBans.IsEmpty())
	{
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

					INC_DWORD_STAT(STAT_WFCBoundaryConstraintNumChecks);

					if (!CanTileBeAdjacentToBoundaries(TileId, BoundaryDirections))
					{
						TileIdsToBan.AddUnique(TileId);
					}
				}

				if (TileIdsToBan.Num() > 0)
				{
					// boundary constraint runs once on initialization, and is deterministic,
					// so cache the bans in case the constraint needs to be applied again
					CachedTileBans.Add(CellIndex, TileIdsToBan);
					Generator->BanMultiple(CellIndex, TileIdsToBan);
					INC_DWORD_STAT_BY(STAT_WFCBoundaryConstraintNumBans, TileIdsToBan.Num());
					bDidMakeChanges = true;
				}
			}
		}
	}
	else
	{
		// apply cached bans
		for (const auto& Elem : CachedTileBans)
		{
			Generator->BanMultiple(Elem.Key, Elem.Value);
			INC_DWORD_STAT_BY(STAT_WFCBoundaryConstraintNumBans, Elem.Value.Num());
			bDidMakeChanges = true;
		}
	}

	bDidApplyInitialConstraint = true;

	INC_FLOAT_STAT_BY(STAT_WFCBoundaryConstraintTime, (FPlatformTime::Seconds() - StartTime) * 1000);
	return bDidMakeChanges;
}
