// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileSet.h"

#include "WFCTileAsset.h"
#include "UObject/ObjectSaveContext.h"


// UWFCTileSet
// -----------

void UWFCTileSet::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	if (!SaveContext.IsProceduralSave())
	{
		// remove invalid entries
		TileAssets.RemoveAll([](const UWFCTileAsset* TileAsset)
		{
			return TileAsset == nullptr;
		});

		// sort tiles by name
		TileAssets.Sort([](const UWFCTileAsset& TileAssetA, const UWFCTileAsset& TileAssetB)
		{
			return TileAssetA.GetName() < TileAssetB.GetName();
		});
	}
}
