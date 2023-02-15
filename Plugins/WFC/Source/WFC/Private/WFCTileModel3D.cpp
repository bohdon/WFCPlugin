// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileModel3D.h"

#include "WFCModule.h"
#include "WFCTileAsset3D.h"
#include "WFCTileSet.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"
#include "Core/Constraints/WFCBoundaryConstraint.h"
#include "Core/Grids/WFCGrid3D.h"


UWFCTileModel3D::UWFCTileModel3D()
{
}

void UWFCTileModel3D::GenerateTiles()
{
	const UWFCTileSet* TileSet = GetAssetTileSet();
	if (!TileSet)
	{
		UE_LOG(LogWFC, Warning, TEXT("UWFCTileGenerator3D::GenerateTiles, expected a WFCTileSet, got %s"),
		       *GetNameSafe(TileDataRef.Get()));
		return;
	}

	for (const FWFCTileSetEntry& TileSetEntry : TileSet->Tiles)
	{
		TObjectPtr<UWFCTileAsset> TileAsset = TileSetEntry.TileAsset;
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
						int32 TileDefIndex;
						FWFCTileDef3D TileDef = TileAsset3D->GetTileDefByLocation(FIntVector(X, Y, Z), TileDefIndex);

						TSharedPtr<FWFCModelAssetTile> Tile = MakeShared<FWFCModelAssetTile>();
						Tile->Weight = TileSet->GetTileWeight(TileSetEntry);
						Tile->TileAsset = TileAsset3D;
						Tile->Rotation = Rotation;
						Tile->TileDefIndex = TileDefIndex;

						AddTileFromAsset(TileAsset, Tile);
					}
				}
			}
		}
	}
}

bool UWFCTileModel3D::CanTilesBeAdjacent(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB,
                                         FWFCGridDirection Direction, const UWFCGrid* Grid) const
{
	const FWFCGridDirection InvDirection = Grid->GetOppositeDirection(Direction);

	const UWFCTileAsset3D* Tile3DAssetA = Cast<UWFCTileAsset3D>(TileA.TileAsset.Get());
	const UWFCTileAsset3D* Tile3DAssetB = Cast<UWFCTileAsset3D>(TileB.TileAsset.Get());

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
	const FGameplayTag SocketTypeA = TileDefA.EdgeTypes.FindRef(static_cast<EWFCTile3DEdge>(AEdgeDirection));
	if (!SocketTypeA.IsValid())
	{
		return false;
	}

	const FGameplayTag SocketTypeB = TileDefB.EdgeTypes.FindRef(static_cast<EWFCTile3DEdge>(BEdgeDirection));
	if (!SocketTypeB.IsValid())
	{
		return false;
	}

	if (SocketTypeA == SocketTypeB)
	{
		return true;
	}
	return false;
}

bool UWFCTileModel3D::CanTileBeAdjacentToGridBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction,
                                                      const UWFCGrid* Grid, const UWFCGenerator* Generator) const
{
	const UWFCBoundaryConstraint* BoundaryConstraint = Generator->GetConstraint<UWFCBoundaryConstraint>();

	// to support large tiles, interior large tile edges are not allowed to be adjacent to the grid boundary
	const UWFCTileAsset3D* Tile3DAsset = Cast<UWFCTileAsset3D>(Tile.TileAsset.Get());
	if (Tile3DAsset)
	{
		const FWFCTileDef3D TileDef = Tile3DAsset->GetTileDefByIndex(Tile.TileDefIndex);
		const FIntVector& Dimensions = Tile3DAsset->Dimensions;
		const FWFCGridDirection LocalDirection = Grid->InverseRotateDirection(Direction, Tile.Rotation);

		// check edge type
		if (BoundaryConstraint)
		{
			const FGameplayTag EdgeType = TileDef.EdgeTypes.FindRef(static_cast<EWFCTile3DEdge>(LocalDirection));
			const FGameplayTagContainer EdgeTypeTags(EdgeType);
			if (!BoundaryConstraint->EdgeTypeQuery.IsEmpty() && !BoundaryConstraint->EdgeTypeQuery.Matches(EdgeTypeTags))
			{
				return false;
			}
		}

		// only exterior edges can be adjacent to boundary
		const FIntVector LocationInDirection = TileDef.Location + UWFCGrid3D::GetDirectionVector(LocalDirection);
		if (!(LocationInDirection.X < 0 || LocationInDirection.X >= Dimensions.X ||
			LocationInDirection.Y < 0 || LocationInDirection.Y >= Dimensions.Y ||
			LocationInDirection.Z < 0 || LocationInDirection.Z >= Dimensions.Z))
		{
			return false;
		}
	}
	return true;
}
