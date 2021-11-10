// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileSetGenerator.h"

#include "WFCTileAsset.h"
#include "WFCTileSet.h"


void UWFCTileSetGenerator::GenerateTiles_Implementation(const UWFCTileSet* TileSet, TArray<FWFCTile>& OutTiles) const
{
	OutTiles.SetNum(TileSet->TileAssets.Num());
	int32 Idx = 0;
	for (const TObjectPtr<UWFCTileAsset>& TileAsset : TileSet->TileAssets)
	{
		OutTiles[Idx] = FWFCTile(TileAsset.Get());
		++Idx;
	}
}

void UWFCTileSetGenerator::ConfigureGeneratorForTiles_Implementation(const UWFCTileSet* TileSet, const UWFCModel* Model,
                                                                     UWFCGenerator* Generator) const
{
}
