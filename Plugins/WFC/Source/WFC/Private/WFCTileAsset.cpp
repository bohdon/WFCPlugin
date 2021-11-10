// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset.h"


UWFCTile2DAsset::UWFCTile2DAsset()
	: Dimensions(FIntPoint(1, 1)),
	  bAllowRotation(true)
{
}

FWFCTileDef2D UWFCTile2DAsset::GetTileDefByLocation(FIntPoint Location) const
{
	for (const FWFCTileDef2D& TileDef : TileDefs)
	{
		if (TileDef.Location == Location)
		{
			return TileDef;
		}
	}
	return FWFCTileDef2D();
}

FWFCTileDef2D UWFCTile2DAsset::GetTileDefByIndex(int32 Index) const
{
	return TileDefs.IsValidIndex(Index) ? TileDefs[Index] : FWFCTileDef2D();
}

UWFCTile3DAsset::UWFCTile3DAsset()
	: bAllowRotations(true)
{
}
