// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset3D.h"


UWFCTileAsset3D::UWFCTileAsset3D()
	: Dimensions(FIntVector(1, 1, 1)),
	  bAllowRotation(true)
{
}

FWFCTileDef3D UWFCTileAsset3D::GetTileDefByLocation(FIntVector Location) const
{
	for (const FWFCTileDef3D& TileDef : TileDefs)
	{
		if (TileDef.Location == Location)
		{
			return TileDef;
		}
	}
	return FWFCTileDef3D();
}

FWFCTileDef3D UWFCTileAsset3D::GetTileDefByIndex(int32 Index) const
{
	return TileDefs.IsValidIndex(Index) ? TileDefs[Index] : FWFCTileDef3D();
}
