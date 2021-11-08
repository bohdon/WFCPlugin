// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileSetAsset.h"

#include "WFCTileAsset.h"


void UWFCTileSetAsset::GetTiles(TArray<FWFCTile>& OutTiles)
{
	// TODO: support large size tiles and setup appropriate adjacency rules

	OutTiles.SetNum(TileAssets.Num());
	int32 Idx = 0;
	for (const TObjectPtr<UWFCTileAsset>& TileAsset : TileAssets)
	{
		OutTiles[Idx] = FWFCTile(TileAsset.Get());
		++Idx;
	}
}
