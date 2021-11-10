// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileSet2DGenerator.h"

#include "WFCTileAsset.h"
#include "WFCTileSet.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"


UWFCTileSet2DGenerator::UWFCTileSet2DGenerator()
{
}

void UWFCTileSet2DGenerator::GenerateTiles_Implementation(const UWFCTileSet* TileSet, TArray<FWFCTile>& OutTiles) const
{
	// add each tile for each 90 degree rotation
	constexpr int32 NumRotations = 4;

	OutTiles.SetNum(TileSet->TileAssets.Num() * NumRotations);
	int32 Idx = 0;
	for (const TObjectPtr<UWFCTileAsset>& TileAsset : TileSet->TileAssets)
	{
		for (int32 Rotation = 0; Rotation < NumRotations; ++Rotation)
		{
			OutTiles[Idx] = FWFCTile(TileAsset.Get(), Rotation);
			++Idx;
		}
	}
}

void UWFCTileSet2DGenerator::ConfigureGeneratorForTiles_Implementation(const UWFCTileSet* TileSet, const UWFCModel* Model,
                                                                       UWFCGenerator* Generator) const
{
	if (UWFCAdjacencyConstraint* AdjacencyConstraint = Generator->GetConstraint<UWFCAdjacencyConstraint>())
	{
		ConfigureAdjacencyConstraint(Model, Generator, AdjacencyConstraint);
	}
}

void UWFCTileSet2DGenerator::ConfigureAdjacencyConstraint(const UWFCModel* Model, const UWFCGenerator* Generator,
                                                          UWFCAdjacencyConstraint* AdjacencyConstraint) const
{
	const UWFCGrid* Grid = Generator->GetGrid();
	check(Grid != nullptr);

	const int32 NumDirections = Grid->GetNumDirections();

	// TODO: don't check adjacency for B -> A if A -> B has already been checked, change AddAdjacentTileMapping to include both

	// iterate over all tiles, comparing to all other tiles...
	for (FWFCTileId TileIdA = 0; TileIdA < Generator->GetNumTiles(); ++TileIdA)
	{
		FWFCTile TileA = Model->GetTile(TileIdA);

		for (FWFCTileId TileIdB = 0; TileIdB < Generator->GetNumTiles(); ++TileIdB)
		{
			FWFCTile TileB = Model->GetTile(TileIdB);

			UWFCTile2DAsset* Tile2DA = Cast<UWFCTile2DAsset>(TileA.Object.Get());
			UWFCTile2DAsset* Tile2DB = Cast<UWFCTile2DAsset>(TileB.Object.Get());
			if (Tile2DA && Tile2DB)
			{
				// for each direction, and check if socket type matches opposite direction on the other tile
				for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
				{
					// adjacency mappings represent 'incoming' directions,
					// so the adjacency mappings for A here represent the direction B -> A
					const int32 AInvRotation = (4 - TileA.Rotation) % 4;
					const int32 BInvRotation = (4 - TileB.Rotation) % 4;
					FWFCGridDirection AEdgeDirection = Grid->GetRotatedDirection(Grid->GetOppositeDirection(Direction), AInvRotation);
					FWFCGridDirection BEdgeDirection = Grid->GetRotatedDirection(Direction, BInvRotation);
					const int32 SocketTypeA = Tile2DA->EdgeSocketTypes.FindRef(static_cast<EWFCTile2DEdge>(AEdgeDirection));
					const int32 SocketTypeB = Tile2DB->EdgeSocketTypes.FindRef(static_cast<EWFCTile2DEdge>(BEdgeDirection));

					if (SocketTypeA == SocketTypeB)
					{
						AdjacencyConstraint->AddAdjacentTileMapping(TileIdA, Direction, TileIdB);
					}
				}
			}
		}
	}
}
