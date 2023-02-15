// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileSet.h"

#include "WFCModule.h"
#include "WFCTileAsset.h"
#include "UObject/ObjectSaveContext.h"


float UWFCTileSet::GetTileWeight(const FWFCTileSetEntry& TileSetEntry) const
{
	if (!TileSetEntry.TileAsset)
	{
		return 0.0f;
	}
	for (const FWFCTileSetTagWeight& WeightRule : TagWeights)
	{
		if (TileSetEntry.TileAsset->OwnedTags.HasTag(WeightRule.Tag))
		{
			UE_LOG(LogWFC, VeryVerbose, TEXT("Tile '%s' matches tag '%s', using tag weight: '%f'"),
			       *TileSetEntry.TileAsset->GetName(), *WeightRule.Tag.ToString(), WeightRule.Weight);

			return WeightRule.Weight;
		}
	}
	return TileSetEntry.Weight;
}

void UWFCTileSet::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	if (!SaveContext.IsProceduralSave())
	{
		// remove invalid entries
		Tiles.RemoveAll([](const FWFCTileSetEntry& Tile)
		{
			return !IsValid(Tile.TileAsset);
		});

		// sort tiles by name
		Tiles.Sort([](const FWFCTileSetEntry& TileA, const FWFCTileSetEntry& TileB)
		{
			return TileA.TileAsset->GetName() < TileB.TileAsset->GetName();
		});
	}
}
