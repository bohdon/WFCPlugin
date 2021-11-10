// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileGenerator2D.h"

#include "WFCModule.h"
#include "WFCTileAsset.h"
#include "WFCTileSet.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"
#include "Core/Grids/WFCGrid2D.h"


FString FWFCModelTile2D::ToString() const
{
	return FString::Printf(TEXT("%s:%s.%d"), *Super::ToString(), *GetNameSafe(TileAsset.Get()), TileDefIndex);
}

UWFCTileGenerator2D::UWFCTileGenerator2D()
{
}

void UWFCTileGenerator2D::GenerateTiles()
{
	const UWFCTileSet* TileSet = GetTileData<UWFCTileSet>();
	if (!TileSet)
	{
		UE_LOG(LogWFC, Warning, TEXT("UWFCTileGenerator2D::GenerateTiles, expected a WFCTileSet, got %s"),
		       *GetNameSafe(TileDataRef.Get()));
		return;
	}

	for (const TObjectPtr<UWFCTileAsset>& TileAsset : TileSet->TileAssets)
	{
		UWFCTile2DAsset* TileAsset2D = Cast<UWFCTile2DAsset>(TileAsset);
		if (!TileAsset2D)
		{
			// unsupported tile asset
			continue;
		}

		const int32 NumRotations = TileAsset2D->bAllowRotation ? 4 : 1;

		// for each possible tile (asset) rotation...
		for (int32 Rotation = 0; Rotation < NumRotations; ++Rotation)
		{
			// iterate all tile defs (subtiles) of the tile asset
			for (int32 X = 0; X < TileAsset2D->Dimensions.X; ++X)
			{
				for (int32 Y = 0; Y < TileAsset2D->Dimensions.Y; ++Y)
				{
					FWFCTileDef2D Tile2DDef = TileAsset2D->GetTileDefByLocation(FIntPoint(X, Y));
					const int32 TileDefIndex = X + (Y * TileAsset2D->Dimensions.X);

					TSharedPtr<FWFCModelTile2D> Tile = MakeShared<FWFCModelTile2D>();
					Tile->Weight = TileAsset->Weight;
					Tile->TileAsset = TileAsset2D;
					Tile->Rotation = Rotation;
					Tile->TileDefIndex = TileDefIndex;

					AddTile(Tile);
				}
			}
		}
	}
}

void UWFCTileGenerator2D::ConfigureGenerator(UWFCGenerator* Generator)
{
	if (UWFCAdjacencyConstraint* AdjacencyConstraint = Generator->GetConstraint<UWFCAdjacencyConstraint>())
	{
		ConfigureAdjacencyConstraint(Generator, AdjacencyConstraint);
	}
}

void UWFCTileGenerator2D::ConfigureAdjacencyConstraint(const UWFCGenerator* Generator, UWFCAdjacencyConstraint* AdjacencyConstraint) const
{
	const UWFCGrid* Grid = Generator->GetGrid();
	check(Grid != nullptr);

	const int32 NumDirections = Grid->GetNumDirections();

	// TODO: don't check adjacency for B -> A if A -> B has already been checked, change AddAdjacentTileMapping to include both

	// iterate over all tiles, comparing socket types for compatibility
	for (FWFCTileId TileIdA = 0; TileIdA <= GetMaxTileId(); ++TileIdA)
	{
		const FWFCModelTile2D& TileA = GetTileRef<FWFCModelTile2D>(TileIdA);

		for (FWFCTileId TileIdB = 0; TileIdB <= GetMaxTileId(); ++TileIdB)
		{
			const FWFCModelTile2D& TileB = GetTileRef<FWFCModelTile2D>(TileIdB);

			if (TileA.TileAsset.IsValid() && TileB.TileAsset.IsValid())
			{
				// for each direction, check if socket type matches opposite direction on the other tile
				for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
				{
					if (CanTilesBeAdjacent(TileA, TileB, Direction, Grid))
					{
						UE_LOG(LogWFC, VeryVerbose, TEXT("Allowing adjacency: %s < Dir %d < %s"),
						       *TileA.ToString(), Direction, *TileB.ToString());
						AdjacencyConstraint->AddAdjacentTileMapping(TileIdA, Direction, TileIdB);
					}
				}
			}
		}
	}
}

bool UWFCTileGenerator2D::CanTilesBeAdjacent(const FWFCModelTile2D& TileA, const FWFCModelTile2D& TileB,
                                             FWFCGridDirection Direction, const UWFCGrid* Grid) const
{
	const FWFCGridDirection InvDirection = Grid->GetOppositeDirection(Direction);

	const FWFCTileDef2D& TileDefA = TileA.TileAsset->GetTileDefByIndex(TileA.TileDefIndex);
	const FWFCTileDef2D& TileDefB = TileB.TileAsset->GetTileDefByIndex(TileB.TileDefIndex);

	// check if tiles are from the same asset and should have fixed adjacency
	if (TileA.TileAsset == TileB.TileAsset && TileA.Rotation == TileB.Rotation && TileA.TileDefIndex != TileB.TileDefIndex)
	{
		const FWFCGridDirection LocalDirection = Grid->InverseRotateDirection(InvDirection, TileA.Rotation);
		const FIntPoint NeighborLocation = TileDefA.Location + UWFCGrid2D::GetDirectionVector(LocalDirection);
		if (TileDefB.Location == NeighborLocation)
		{
			return true;
		}
	}

	// convert the incoming direction to local space directions to the relevant edge of the tiles
	FWFCGridDirection AEdgeDirection = Grid->InverseRotateDirection(InvDirection, TileA.Rotation);
	FWFCGridDirection BEdgeDirection = Grid->InverseRotateDirection(Direction, TileB.Rotation);

	// compare edge socket types
	const int32 SocketTypeA = TileDefA.EdgeSocketTypes.FindRef(static_cast<EWFCTile2DEdge>(AEdgeDirection));
	if (SocketTypeA == INDEX_NONE)
	{
		return false;
	}

	const int32 SocketTypeB = TileDefB.EdgeSocketTypes.FindRef(static_cast<EWFCTile2DEdge>(BEdgeDirection));
	if (SocketTypeB == INDEX_NONE)
	{
		return false;
	}

	if (SocketTypeA == SocketTypeB)
	{
		return true;
	}
	return false;
}

FWFCModelTile2D UWFCTileGenerator2D::GetTileById(int32 TileId) const
{
	if (const FWFCModelTile2D* Tile = GetTile<FWFCModelTile2D>(TileId))
	{
		return *Tile;
	}
	return FWFCModelTile2D();
}
