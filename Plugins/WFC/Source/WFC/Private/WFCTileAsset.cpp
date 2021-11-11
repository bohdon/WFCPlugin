// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset.h"


FString FWFCModelAssetTile::ToString() const
{
	return FString::Printf(TEXT("%s:%s.%d(R%d)"), *Super::ToString(), *GetNameSafe(TileAsset.Get()), TileDefIndex, Rotation);
}

UWFCTileAsset::UWFCTileAsset()
	: Weight(1.f)
{
}
