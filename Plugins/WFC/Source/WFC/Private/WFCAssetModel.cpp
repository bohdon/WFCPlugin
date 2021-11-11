// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCAssetModel.h"

#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"


FWFCModelAssetTile UWFCAssetModel::GetTileById(int32 TileId) const
{
	if (const FWFCModelAssetTile* Tile = GetTile<FWFCModelAssetTile>(TileId))
	{
		return *Tile;
	}
	return FWFCModelAssetTile();
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
