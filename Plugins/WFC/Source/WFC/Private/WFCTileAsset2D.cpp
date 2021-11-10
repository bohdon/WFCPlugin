// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset2D.h"


UWFCTileAsset2D::UWFCTileAsset2D()
	: Dimensions(FIntPoint(1, 1)),
	  bAllowRotation(true)
{
}

FWFCTileDef2D UWFCTileAsset2D::GetTileDefByLocation(FIntPoint Location) const
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

FWFCTileDef2D UWFCTileAsset2D::GetTileDefByIndex(int32 Index) const
{
	return TileDefs.IsValidIndex(Index) ? TileDefs[Index] : FWFCTileDef2D();
}
