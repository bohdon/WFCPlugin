// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCAssetModel.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"
#include "Core/Constraints/WFCBoundaryConstraint.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Adjacency Constraint - Mapping Checks"), STAT_WFCAdjacencyConstraintMappingChecks, STATGROUP_WFC);


void UWFCAssetModel::AddTileFromAsset(const UWFCTileAsset* TileAsset, TSharedPtr<FWFCModelTile> Tile)
{
	if (Tile.IsValid())
	{
		AddTile(Tile);
		TileAssetIds.FindOrAdd(TileAsset).TileIds.AddUnique(Tile->Id);
	}
}

const UWFCTileSet* UWFCAssetModel::GetAssetTileSet() const
{
	return GetTileData<UWFCTileSet>();
}

FWFCModelAssetTile UWFCAssetModel::GetAssetTile(int32 TileId) const
{
	if (const FWFCModelAssetTile* Tile = GetTile<FWFCModelAssetTile>(TileId))
	{
		return *Tile;
	}
	return FWFCModelAssetTile();
}

FWFCTileIdArray UWFCAssetModel::GetTileIdsForAsset(const UWFCTileAsset* TileAsset) const
{
	return TileAssetIds.FindRef(TileAsset);
}

int32 UWFCAssetModel::GetTileIdForAssetTileDef(const UWFCTileAsset* TileAsset, int32 TileDefIndex, int32 Rotation) const
{
	FWFCTileIdArray TileIds = GetTileIdsForAsset(TileAsset);
	for (const FWFCTileId TileId : TileIds.TileIds)
	{
		if (const FWFCModelAssetTile* Tile = GetTile<FWFCModelAssetTile>(TileId))
		{
			if (Tile->TileDefIndex == TileDefIndex && Tile->Rotation == Rotation)
			{
				return TileId;
			}
		}
	}
	return INDEX_NONE;
}

void UWFCAssetModel::ConfigureGenerator(UWFCGenerator* Generator)
{
	if (UWFCAdjacencyConstraint* AdjacencyConstraint = Generator->GetConstraint<UWFCAdjacencyConstraint>())
	{
		ConfigureAdjacencyConstraint(Generator, AdjacencyConstraint);
	}
	if (UWFCBoundaryConstraint* BoundaryConstraint = Generator->GetConstraint<UWFCBoundaryConstraint>())
	{
		ConfigureBoundaryConstraint(Generator, BoundaryConstraint);
	}
}

FString UWFCAssetModel::GetTileDebugString(FWFCTileId TileId) const
{
	if (const FWFCModelAssetTile* AssetTile = GetTile<FWFCModelAssetTile>(TileId))
	{
		return FString::Printf(TEXT("Tile %d (%s)"), TileId, *AssetTile->ToString());
	}
	return Super::GetTileDebugString(TileId);
}

void UWFCAssetModel::ConfigureAdjacencyConstraint(const UWFCGenerator* Generator, UWFCAdjacencyConstraint* AdjacencyConstraint) const
{
	SCOPE_LOG_TIME(TEXT("UWFCAssetModel::ConfigureAdjacencyConstraint"), nullptr);
	SET_DWORD_STAT(STAT_WFCAdjacencyConstraintMappingChecks, 0);

	const UWFCGrid* Grid = Generator->GetGrid();
	check(Grid != nullptr);

	const int32 NumDirections = Grid->GetNumDirections();

	// TODO: blacklist interior tile/edge pairs and compare only externals, then iterate internals directly and add mappings

	// iterate over all distinct pairs of tiles, including reflectivity, comparing socket types for compatibility
	for (FWFCTileId TileIdA = 0; TileIdA <= GetMaxTileId(); ++TileIdA)
	{
		const FWFCModelAssetTile& TileA = GetTileRef<FWFCModelAssetTile>(TileIdA);

		// compare A <-> A for each direction
		for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
		{
			INC_DWORD_STAT(STAT_WFCAdjacencyConstraintMappingChecks);

			if (CanTilesBeAdjacent(TileA, TileA, Direction, Grid))
			{
				AdjacencyConstraint->AddAdjacentTileMapping(TileIdA, Direction, TileIdA);
			}
		}

		for (FWFCTileId TileIdB = TileIdA + 1; TileIdB <= GetMaxTileId(); ++TileIdB)
		{
			const FWFCModelAssetTile& TileB = GetTileRef<FWFCModelAssetTile>(TileIdB);

			// compare A <-> B for each direction
			for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
			{
				INC_DWORD_STAT(STAT_WFCAdjacencyConstraintMappingChecks);

				if (CanTilesBeAdjacent(TileA, TileB, Direction, Grid))
				{
					AdjacencyConstraint->AddAdjacentTileMapping(TileIdA, Direction, TileIdB);

					// add opposite directly as well
					const FWFCGridDirection OppositeDirection = Grid->GetOppositeDirection(Direction);
					AdjacencyConstraint->AddAdjacentTileMapping(TileIdB, OppositeDirection, TileIdA);
				}
			}
		}
	}

	AdjacencyConstraint->LogDebugInfo();
}

bool UWFCAssetModel::CanTilesBeAdjacent(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB, FWFCGridDirection Direction,
                                        const UWFCGrid* Grid) const
{
	return false;
}

void UWFCAssetModel::ConfigureBoundaryConstraint(const UWFCGenerator* Generator, UWFCBoundaryConstraint* BoundaryConstraint) const
{
	SCOPE_LOG_TIME(TEXT("UWFCAssetModel::ConfigureBoundaryConstraint"), nullptr);

	const UWFCGrid* Grid = Generator->GetGrid();
	check(Grid != nullptr);

	const int32 NumDirections = Grid->GetNumDirections();

	for (FWFCTileId TileId = 0; TileId <= GetMaxTileId(); ++TileId)
	{
		const FWFCModelAssetTile& Tile = GetTileRef<FWFCModelAssetTile>(TileId);

		for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
		{
			if (!CanTileBeAdjacentToGridBoundary(Tile, Direction, Grid, Generator))
			{
				BoundaryConstraint->AddProhibitedAdjacentBoundaryMapping(TileId, Direction);
			}
		}
	}
}

bool UWFCAssetModel::CanTileBeAdjacentToGridBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction,
                                                     const UWFCGrid* Grid, const UWFCGenerator* Generator) const
{
	return true;
}
