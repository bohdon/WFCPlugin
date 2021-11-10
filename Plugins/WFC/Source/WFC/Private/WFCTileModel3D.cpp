// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileModel3D.h"

#include "WFCModule.h"
#include "WFCTileAsset3D.h"
#include "WFCTileSet.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"
#include "Core/Grids/WFCGrid3D.h"


UWFCTileModel3D::UWFCTileModel3D()
{
}

void UWFCTileModel3D::GenerateTiles()
{
	const UWFCTileSet* TileSet = GetTileData<UWFCTileSet>();
	if (!TileSet)
	{
		UE_LOG(LogWFC, Warning, TEXT("UWFCTileGenerator3D::GenerateTiles, expected a WFCTileSet, got %s"),
		       *GetNameSafe(TileDataRef.Get()));
		return;
	}

	for (const TObjectPtr<UWFCTileAsset>& TileAsset : TileSet->TileAssets)
	{
		UWFCTileAsset3D* TileAsset3D = Cast<UWFCTileAsset3D>(TileAsset);
		if (!TileAsset3D)
		{
			// unsupported tile asset
			continue;
		}

		// TODO: support pitch and roll rotation

		// currently only supporting yaw rotation
		const int32 NumRotations = TileAsset3D->bAllowRotation ? 4 : 1;
		const FIntVector Dimensions = TileAsset3D->Dimensions;

		// for each possible tile (asset) rotation...
		for (int32 Rotation = 0; Rotation < NumRotations; ++Rotation)
		{
			// iterate all tiles of the asset
			for (int32 X = 0; X < Dimensions.X; ++X)
			{
				for (int32 Y = 0; Y < Dimensions.Y; ++Y)
				{
					for (int32 Z = 0; Z < Dimensions.Z; ++Z)
					{
						FWFCTileDef3D TileDef = TileAsset3D->GetTileDefByLocation(FIntVector(X, Y, Z));
						// TODO: make util for this
						const int32 TileDefIndex = X + (Y * Dimensions.X) + (Z * Dimensions.X * Dimensions.Y);

						TSharedPtr<FWFCModelAssetTile> Tile = MakeShared<FWFCModelAssetTile>();
						Tile->Weight = TileAsset->Weight;
						Tile->TileAsset = TileAsset3D;
						Tile->Rotation = Rotation;
						Tile->TileDefIndex = TileDefIndex;

						AddTile(Tile);
					}
				}
			}
		}
	}
}

void UWFCTileModel3D::ConfigureGenerator(UWFCGenerator* Generator)
{
	if (UWFCAdjacencyConstraint* AdjacencyConstraint = Generator->GetConstraint<UWFCAdjacencyConstraint>())
	{
		ConfigureAdjacencyConstraint(Generator, AdjacencyConstraint);
	}
}

bool UWFCTileModel3D::CanTilesBeAdjacent(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB,
                                         FWFCGridDirection Direction, const UWFCGrid* Grid) const
{
	const FWFCGridDirection InvDirection = Grid->GetOppositeDirection(Direction);

	const UWFCTileAsset3D* Tile3DAssetA = Cast<UWFCTileAsset3D>(TileB.TileAsset.Get());
	const UWFCTileAsset3D* Tile3DAssetB = Cast<UWFCTileAsset3D>(TileA.TileAsset.Get());

	const FWFCTileDef3D& TileDefA = Tile3DAssetA->GetTileDefByIndex(TileA.TileDefIndex);
	const FWFCTileDef3D& TileDefB = Tile3DAssetB->GetTileDefByIndex(TileB.TileDefIndex);

	// check if tiles are from the same asset and should have fixed adjacency
	if (TileA.TileAsset == TileB.TileAsset && TileA.Rotation == TileB.Rotation && TileA.TileDefIndex != TileB.TileDefIndex)
	{
		const FWFCGridDirection LocalDirection = Grid->InverseRotateDirection(InvDirection, TileA.Rotation);
		const FIntVector NeighborLocation = TileDefA.Location + UWFCGrid3D::GetDirectionVector(LocalDirection);
		if (TileDefB.Location == NeighborLocation)
		{
			return true;
		}
	}

	// convert the incoming direction to local space directions to the relevant edge of the tiles
	FWFCGridDirection AEdgeDirection = Grid->InverseRotateDirection(InvDirection, TileA.Rotation);
	FWFCGridDirection BEdgeDirection = Grid->InverseRotateDirection(Direction, TileB.Rotation);

	// compare edge socket types
	const int32 SocketTypeA = TileDefA.EdgeSocketTypes.FindRef(static_cast<EWFCTile3DEdge>(AEdgeDirection));
	if (SocketTypeA == INDEX_NONE)
	{
		return false;
	}

	const int32 SocketTypeB = TileDefB.EdgeSocketTypes.FindRef(static_cast<EWFCTile3DEdge>(BEdgeDirection));
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
