// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCAssetModel.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"
#include "Core/Constraints/WFCBoundaryConstraint.h"


FWFCModelAssetTile UWFCAssetModel::GetTileById(int32 TileId) const
{
	if (const FWFCModelAssetTile* Tile = GetTile<FWFCModelAssetTile>(TileId))
	{
		return *Tile;
	}
	return FWFCModelAssetTile();
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

void UWFCAssetModel::ConfigureAdjacencyConstraint(const UWFCGenerator* Generator, UWFCAdjacencyConstraint* AdjacencyConstraint) const
{
	const UWFCGrid* Grid = Generator->GetGrid();
	check(Grid != nullptr);

	const int32 NumDirections = Grid->GetNumDirections();

	// TODO: don't check adjacency for B -> A if A -> B has already been checked, change AddAdjacentTileMapping to include both

	// iterate over all tiles, comparing socket types for compatibility
	for (FWFCTileId TileIdA = 0; TileIdA <= GetMaxTileId(); ++TileIdA)
	{
		const FWFCModelAssetTile& TileA = GetTileRef<FWFCModelAssetTile>(TileIdA);

		for (FWFCTileId TileIdB = 0; TileIdB <= GetMaxTileId(); ++TileIdB)
		{
			const FWFCModelAssetTile& TileB = GetTileRef<FWFCModelAssetTile>(TileIdB);

			for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
			{
				if (CanTilesBeAdjacent(TileA, TileB, Direction, Grid))
				{
					UE_LOG(LogWFC, VeryVerbose, TEXT("Allowing adjacency: %s < Dir %s < %s"),
					       *TileA.ToString(), *Grid->GetDirectionName(Direction), *TileB.ToString());

					AdjacencyConstraint->AddAdjacentTileMapping(TileIdA, Direction, TileIdB);
				}
			}
		}
	}
}

bool UWFCAssetModel::CanTilesBeAdjacent(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB, FWFCGridDirection Direction,
                                        const UWFCGrid* Grid) const
{
	return false;
}

void UWFCAssetModel::ConfigureBoundaryConstraint(const UWFCGenerator* Generator, UWFCBoundaryConstraint* BoundaryConstraint) const
{
	const UWFCGrid* Grid = Generator->GetGrid();
	check(Grid != nullptr);

	const int32 NumDirections = Grid->GetNumDirections();

	for (FWFCTileId TileId = 0; TileId <= GetMaxTileId(); ++TileId)
	{
		const FWFCModelAssetTile& Tile = GetTileRef<FWFCModelAssetTile>(TileId);

		for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
		{
			if (!CanTileBeAdjacentToGridBoundary(Tile, Direction, Grid))
			{
				UE_LOG(LogWFC, VeryVerbose, TEXT("Prohibiting boundary adjacency: %s > Dir %s"),
				       *Tile.ToString(), *Grid->GetDirectionName(Direction));

				BoundaryConstraint->AddProhibitedAdjacentBoundaryMapping(TileId, Direction);
			}
		}
	}
}

bool UWFCAssetModel::CanTileBeAdjacentToGridBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction,
                                                      const UWFCGrid* Grid) const
{
	return true;
}
