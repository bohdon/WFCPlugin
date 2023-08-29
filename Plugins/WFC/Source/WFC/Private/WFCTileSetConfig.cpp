// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileSetConfig.h"

#include "WFCModule.h"
#include "WFCTileAsset.h"


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
		return !WeightRule.TagQuery.IsEmpty() && WeightRule.TagQuery.Matches(TileAsset->OwnedTags);
	});

	if (WeightRule)
	{
		UE_LOG(LogWFC, VeryVerbose, TEXT("Tile '%s' matches tag query '%s', using Weight: '%f'"),
		       *TileAsset->GetName(), *WeightRule->TagQuery.GetDescription(), WeightRule->Weight);

		return WeightRule->Weight;
	}
	return 1.f;
}
