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

	SCOPE_LOG_TIME_FUNC();
	SET_DWORD_STAT(STAT_WFCBoundaryConstraintMappings, 0);
	SET_FLOAT_STAT(STAT_WFCBoundaryConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCBoundaryConstraintNumChecks, 0);
	SET_DWORD_STAT(STAT_WFCBoundaryConstraintNumBans, 0);

	if (bIsInitialized)
	{
		return;
	}

	bDidApplyInitialConstraint = false;

	const int32 NumDirections = Grid->GetNumDirections();

	for (FWFCTileId TileId = 0; TileId < Model->GetNumTiles(); ++TileId)
	{
		const FWFCModelAssetTile& Tile = Model->GetTileRef<FWFCModelAssetTile>(TileId);

		for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
		{
			if (!CanTileBeNextToBoundary(Tile, Direction))
			{
				AddProhibitedAdjacentBoundaryMapping(TileId, Direction);
			}
		}
	}

	bIsInitialized = true;
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

bool UWFCBoundaryConstraint::CanTileBeNextToBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction) const
{
	const UWFCTileAsset* TileAsset = Tile.TileAsset.Get();
	if (!TileAsset)
	{
		return false;
	}

	// convert grid direction to local space for checking edge types within the tile asset
	const FWFCGridDirection LocalDirection = Grid->InverseRotateDirection(Direction, Tile.Rotation);

	if (TileAsset->IsInteriorEdge(Tile.TileDefIndex, LocalDirection))
	{
		// for large tiles, only exterior edges can be against the boundary
		return false;
	}

	// check edge type
	const FGameplayTag EdgeType = TileAsset->GetTileDefEdgeType(Tile.TileDefIndex, LocalDirection);
	const FGameplayTagContainer EdgeTypeTags(EdgeType);
	if (!EdgeTypeQuery.IsEmpty() && !EdgeTypeQuery.Matches(EdgeTypeTags))
	{
		return false;
	}

	return true;
}

bool UWFCBoundaryConstraint::IsTileBoundaryDirectionProhibited(FWFCTileId TileId, const TArray<FWFCGridDirection>& BoundaryDirections) const
{
	const TArray<FWFCGridDirection>* ProhibitedDirectionsPtr = TileBoundaryProhibitionMap.Find(TileId);
	if (ProhibitedDirectionsPtr)
	{
		return ProhibitedDirectionsPtr->ContainsByPredicate([BoundaryDirections](const FWFCGridDirection& Direction)
		{
			return BoundaryDirections.Contains(Direction);
		});
	}
	return false;
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

	// if tiles to ban is already filled out, don't recalculate it, since it
	// will be the same each time this constraint is first run.
	if (TilesToBan.IsEmpty())
	{
		for (FWFCCellIndex CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
		{
			const FWFCCell& CellToCheck = Generator->GetCell(CellIndex);
			if (CellToCheck.HasSelection())
			{
				// don't change cells that are already selected
				continue;
			}

			// find all boundary directions
			TArray<FWFCGridDirection> BoundaryDirections;
			for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
			{
				const FWFCCellIndex AdjacentCellIndex = Grid->GetCellIndexInDirection(CellIndex, Direction);
				if (!Grid->IsValidCellIndex(AdjacentCellIndex))
				{
					BoundaryDirections.AddUnique(Direction);
				}
			}

			if (BoundaryDirections.IsEmpty())
			{
				// cell is not next to a boundary
				continue;
			}

			TArray<FWFCTileId> TileIdsToBan;
			for (const FWFCTileId& TileId : CellToCheck.TileCandidates)
			{
				// check each tile for any prohibited boundary directions
				INC_DWORD_STAT(STAT_WFCBoundaryConstraintNumChecks);

				if (IsTileBoundaryDirectionProhibited(TileId, BoundaryDirections))
				{
					TileIdsToBan.Add(TileId);
				}
			}

			if (TileIdsToBan.Num() > 0)
			{
				TilesToBan.Add(CellIndex, TileIdsToBan);
			}
		}
	}

	// apply bans
	if (!TilesToBan.IsEmpty())
	{
		for (const auto& Elem : TilesToBan)
		{
			INC_DWORD_STAT_BY(STAT_WFCBoundaryConstraintNumBans, Elem.Value.Num());
			if (Generator->BanMultiple(Elem.Key, Elem.Value))
			{
				// contradiction
				return true;
			}
		}
		bDidMakeChanges = true;
	}


	bDidApplyInitialConstraint = true;

	INC_FLOAT_STAT_BY(STAT_WFCBoundaryConstraintTime, (FPlatformTime::Seconds() - StartTime) * 1000);
	return bDidMakeChanges;
}

UWFCConstraintSnapshot* UWFCBoundaryConstraint::CreateSnapshot(UObject* Outer) const
{
	UWFCBoundaryConstraintSnapshot* Snapshot = NewObject<UWFCBoundaryConstraintSnapshot>(Outer);
	Snapshot->bDidApplyInitialConstraint = bDidApplyInitialConstraint;
	return Snapshot;
}

void UWFCBoundaryConstraint::ApplySnapshot(const UWFCConstraintSnapshot* Snapshot)
{
	const UWFCBoundaryConstraintSnapshot* BoundarySnapshot = Cast<UWFCBoundaryConstraintSnapshot>(Snapshot);
	bDidApplyInitialConstraint = BoundarySnapshot->bDidApplyInitialConstraint;
}
