// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileModel2D.h"

#include "WFCModule.h"
#include "WFCTileAsset2D.h"
#include "WFCTileSet.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"
#include "Core/Constraints/WFCBoundaryConstraint.h"
#include "Core/Grids/WFCGrid2D.h"


UWFCTileModel2D::UWFCTileModel2D()
{
}

void UWFCTileModel2D::GenerateTiles()
{
	const UWFCTileSet* TileSet = GetAssetTileSet();
	if (!TileSet)
	{
		UE_LOG(LogWFC, Warning, TEXT("UWFCTileGenerator2D::GenerateTiles, expected a WFCTileSet, got %s"),
		       *GetNameSafe(TileDataRef.Get()));
		return;
	}

	for (const FWFCTileSetEntry& TileSetEntry : TileSet->Tiles)
	{
		const TObjectPtr<UWFCTileAsset> TileAsset = TileSetEntry.TileAsset;
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
					Tile->Weight = TileSet->GetTileWeight(TileSetEntry);
					Tile->TileAsset = TileAsset2D;
					Tile->Rotation = Rotation;
					Tile->TileDefIndex = TileDefIndex;

					AddTileFromAsset(TileAsset, Tile);
				}
			}
		}
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
	const FGameplayTag SocketTypeA = TileDefA.EdgeTypes.FindRef(static_cast<EWFCTile2DEdge>(AEdgeDirection));
	if (!SocketTypeA.IsValid())
	{
		return false;
	}

	const FGameplayTag SocketTypeB = TileDefB.EdgeTypes.FindRef(static_cast<EWFCTile2DEdge>(BEdgeDirection));
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

bool UWFCTileModel2D::CanTileBeAdjacentToGridBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction,
                                                      const UWFCGrid* Grid, const UWFCGenerator* Generator) const
{
	const UWFCBoundaryConstraint* BoundaryConstraint = Generator->GetConstraint<UWFCBoundaryConstraint>();

	// to support large tiles, interior large tile edges are not allowed to be adjacent to the grid boundary
	const UWFCTileAsset2D* Tile2DAsset = Cast<UWFCTileAsset2D>(Tile.TileAsset.Get());
	if (Tile2DAsset)
	{
		const FWFCTileDef2D TileDef = Tile2DAsset->GetTileDefByIndex(Tile.TileDefIndex);
		const FIntPoint& Dimensions = Tile2DAsset->Dimensions;
		const FWFCGridDirection LocalDirection = Grid->InverseRotateDirection(Direction, Tile.Rotation);

		// check edge type
		if (BoundaryConstraint)
		{
			const FGameplayTag EdgeType = TileDef.EdgeTypes.FindRef(static_cast<EWFCTile2DEdge>(LocalDirection));
			const FGameplayTagContainer EdgeTypeTags(EdgeType);
			if (!BoundaryConstraint->EdgeTypeQuery.IsEmpty() && !BoundaryConstraint->EdgeTypeQuery.Matches(EdgeTypeTags))
			{
				return false;
			}
		}

		// only exterior edges can be adjacent to boundary
		const FIntPoint LocationInDirection = TileDef.Location + UWFCGrid2D::GetDirectionVector(LocalDirection);
		if (!(LocationInDirection.X < 0 || LocationInDirection.X >= Dimensions.X ||
			LocationInDirection.Y < 0 || LocationInDirection.Y >= Dimensions.Y))
		{
			return false;
		}
	}
	return true;
}
