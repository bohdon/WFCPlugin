// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileSet3DGenerator.h"

#include "WFCTileAsset.h"
#include "WFCTileSet.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"


UWFCTileSet3DGenerator::UWFCTileSet3DGenerator()
	: bAllowYawRotation(true),
	  bAllowPitchRotation(false),
	  bAllowRollRotation(false)
{
}


void UWFCTileSet3DGenerator::GenerateTiles_Implementation(const UWFCTileSet* TileSet, TArray<FWFCTile>& OutTiles) const
{
	if (!bAllowYawRotation)
	{
		Super::GenerateTiles_Implementation(TileSet, OutTiles);
		return;
	}

	// add each tile for each 90 degree yaw rotation
	// TODO: support pitch and roll rotations
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

void UWFCTileSet3DGenerator::ConfigureGeneratorForTiles_Implementation(const UWFCTileSet* TileSet, const UWFCModel* Model,
                                                                       UWFCGenerator* Generator) const
{
	if (UWFCAdjacencyConstraint* AdjacencyConstraint = Generator->GetConstraint<UWFCAdjacencyConstraint>())
	{
		ConfigureAdjacencyConstraint(Model, Generator, AdjacencyConstraint);
	}
}

void UWFCTileSet3DGenerator::ConfigureAdjacencyConstraint(const UWFCModel* Model, const UWFCGenerator* Generator,
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

			UWFCTile3DAsset* Tile3DA = Cast<UWFCTile3DAsset>(TileA.Object.Get());
			UWFCTile3DAsset* Tile3DB = Cast<UWFCTile3DAsset>(TileB.Object.Get());
			if (Tile3DA && Tile3DB)
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
					const int32 SocketTypeA = Tile3DA->EdgeSocketTypes.FindRef(static_cast<EWFCTile3DEdge>(AEdgeDirection));
					const int32 SocketTypeB = Tile3DB->EdgeSocketTypes.FindRef(static_cast<EWFCTile3DEdge>(BEdgeDirection));

					if (SocketTypeA == SocketTypeB)
					{
						AdjacencyConstraint->AddAdjacentTileMapping(TileIdA, Direction, TileIdB);
					}
				}
			}
		}
	}
}
