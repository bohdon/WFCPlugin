// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset3D.h"


UWFCTileAsset3D::UWFCTileAsset3D()
	: Dimensions(FIntVector(1, 1, 1)),
	  bAllowRotation(true)
{
}

FWFCTileDef3D UWFCTileAsset3D::GetTileDefByLocation(FIntVector Location, int32& Index) const
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
	return FWFCTileDef3D();
}

FWFCTileDef3D UWFCTileAsset3D::GetTileDefByIndex(int32 Index) const
{
	return TileDefs.IsValidIndex(Index) ? TileDefs[Index] : FWFCTileDef3D();
}
