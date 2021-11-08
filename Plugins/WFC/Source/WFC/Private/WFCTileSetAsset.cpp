// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileSetAsset.h"

#include "WFCTileAsset.h"


void UWFCTileSetAsset::GetTiles(TArray<FWFCTile>& OutTiles)
{
	// add each tile for each 90 degree rotation
	OutTiles.SetNum(TileAssets.Num() * 4);
	int32 Idx = 0;
	for (const TObjectPtr<UWFCTileAsset>& TileAsset : TileAssets)
	{
		for (int32 Rotation = 0; Rotation < 4; ++Rotation)
		{
			OutTiles[Idx] = FWFCTile(TileAsset.Get(), Rotation);
			++Idx;
		}
	}
}
