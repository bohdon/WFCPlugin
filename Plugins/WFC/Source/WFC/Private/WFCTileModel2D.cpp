// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileModel2D.h"

#include "WFCModule.h"
#include "WFCTileAsset2D.h"
#include "WFCTileSet.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"
#include "Core/Grids/WFCGrid2D.h"


UWFCTileModel2D::UWFCTileModel2D()
{
}

void UWFCTileModel2D::GenerateTiles()
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
		UWFCTileAsset2D* TileAsset2D = Cast<UWFCTileAsset2D>(TileAsset);
		if (!TileAsset2D)
		{
			// unsupported tile asset
			continue;
		}

		const int32 NumRotations = TileAsset2D->bAllowRotation ? 4 : 1;

		// for each possible tile (asset) rotation...
		for (int32 Rotation = 0; Rotation < NumRotations; ++Rotation)
		{
			// iterate all tiles of the asset
			for (int32 X = 0; X < TileAsset2D->Dimensions.X; ++X)
			{
				for (int32 Y = 0; Y < TileAsset2D->Dimensions.Y; ++Y)
				{
					int32 TileDefIndex;
					FWFCTileDef2D Tile2DDef = TileAsset2D->GetTileDefByLocation(FIntPoint(X, Y), TileDefIndex);

					TSharedPtr<FWFCModelAssetTile> Tile = MakeShared<FWFCModelAssetTile>();
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

void UWFCTileModel2D::ConfigureGenerator(UWFCGenerator* Generator)
{
	if (UWFCAdjacencyConstraint* AdjacencyConstraint = Generator->GetConstraint<UWFCAdjacencyConstraint>())
	{
		ConfigureAdjacencyConstraint(Generator, AdjacencyConstraint);
	}
}

bool UWFCTileModel2D::CanTilesBeAdjacent(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB,
                                         FWFCGridDirection Direction, const UWFCGrid* Grid) const
{
	const FWFCGridDirection InvDirection = Grid->GetOppositeDirection(Direction);

	const UWFCTileAsset2D* Tile2DAssetA = Cast<UWFCTileAsset2D>(TileA.TileAsset.Get());
	const UWFCTileAsset2D* Tile2DAssetB = Cast<UWFCTileAsset2D>(TileB.TileAsset.Get());

	const FWFCTileDef2D& TileDefA = Tile2DAssetA->GetTileDefByIndex(TileA.TileDefIndex);
	const FWFCTileDef2D& TileDefB = Tile2DAssetB->GetTileDefByIndex(TileB.TileDefIndex);

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
	if (SocketTypeA <= 0)
	{
		return false;
	}

	const int32 SocketTypeB = TileDefB.EdgeSocketTypes.FindRef(static_cast<EWFCTile2DEdge>(BEdgeDirection));
	if (SocketTypeB <= 0)
	{
		return false;
	}

	if (SocketTypeA == SocketTypeB)
	{
		return true;
	}
	return false;
}
