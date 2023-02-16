// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset.h"


FString FWFCModelAssetTile::ToString() const
{
	return FString::Printf(TEXT("%s:%s-r%d[%d]"), *Super::ToString(), *GetNameSafe(TileAsset.Get()), Rotation, TileDefIndex);
}

UWFCTileAsset::UWFCTileAsset()
{
}
