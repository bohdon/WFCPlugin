// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset2D.h"


UWFCTileAsset2D::UWFCTileAsset2D()
	: Dimensions(FIntPoint(1, 1)),
	  bAllowRotation(true)
{
}

FWFCTileDef2D UWFCTileAsset2D::GetTileDefByLocation(FIntPoint Location, int32& Index) const
{
	Index = INDEX_NONE;
	for (int32 Idx = 0; Idx < TileDefs.Num(); ++Idx)
	{
		if (TileDefs[Idx].Location == Location)
		{
			Index = Idx;
			return TileDefs[Idx];
		}
	}
	return FWFCTileDef2D();
}

FWFCTileDef2D UWFCTileAsset2D::GetTileDefByIndex(int32 Index) const
{
	return TileDefs.IsValidIndex(Index) ? TileDefs[Index] : FWFCTileDef2D();
}
