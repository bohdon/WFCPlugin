// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileSet.h"

#include "WFCModule.h"
#include "WFCTileAsset.h"
#include "UObject/ObjectSaveContext.h"


// UWFCTileSetTagWeightsConfig
// ---------------------------

float UWFCTileSetTagWeightsConfig::GetTileWeight(const UWFCTileAsset* TileAsset) const
{
	if (!TileAsset)
	{
		return 0.0f;
	}
	const FWFCTileTagWeight* WeightRule = Weights.FindByPredicate([TileAsset](const FWFCTileTagWeight& WeightRule)
	{
		return TileAsset->OwnedTags.HasTag(WeightRule.Tag);
	});

	if (WeightRule)
	{
		UE_LOG(LogWFC, VeryVerbose, TEXT("Tile '%s' matches tag '%s', using Weight: '%f'"),
		       *TileAsset->GetName(), *WeightRule->Tag.ToString(), WeightRule->Weight);

		return WeightRule->Weight;
	}
	return 1.f;
}


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
