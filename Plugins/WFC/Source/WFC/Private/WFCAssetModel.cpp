// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCAssetModel.h"

#include "WFCAsset.h"
#include "WFCModule.h"
#include "WFCTileAsset.h"
#include "WFCTileSet.h"
#include "WFCTileSetConfig.h"


const UWFCAsset* UWFCAssetModel::GetWFCAsset() const
{
	return GetTileData<UWFCAsset>();
}

void UWFCAssetModel::GetAllTileAssets(TArray<UWFCTileAsset*>& TileAssets) const
{
	TileAssets.Reset();

	const UWFCAsset* WFCAsset = GetWFCAsset();
	if (!WFCAsset)
	{
		return;
	}

	for (const UWFCTileSet* TileSet : WFCAsset->TileSets)
	{
		if (!TileSet)
		{
			continue;
		}

		for (UWFCTileAsset* TileAsset : TileSet->TileAssets)
		{
			if (ShouldIncludeTileAsset(TileAsset))
			{
				TileAssets.Add(TileAsset);
			}
		}
	}
}

FWFCModelAssetTile UWFCAssetModel::GetAssetTile(int32 TileId) const
{
	if (const FWFCModelAssetTile* Tile = GetTile<FWFCModelAssetTile>(TileId))
	{
		return *Tile;
	}
	return FWFCModelAssetTile();
}

TArray<int32> UWFCAssetModel::GetTileIdsForAsset(const UWFCTileAsset* TileAsset) const
{
	if (!CachedTileIdsByAsset.Contains(TileAsset))
	{
		return TArray<int32>();
	}
	TArray<int32> Result;
	const TArray<TArray<FWFCTileId>>& TileDefArrays = CachedTileIdsByAsset[TileAsset];
	for (const TArray<FWFCTileId>& Elem : TileDefArrays)
	{
		Result.Append(Elem);
	}
	return Result;
}

TArray<int32> UWFCAssetModel::GetTileIdsForAssetAndDef(const UWFCTileAsset* TileAsset, int32 TileDefIndex) const
{
	// TODO: make sure this works with weak ptr index
	if (!CachedTileIdsByAsset.Contains(TileAsset))
	{
		return TArray<int32>();
	}
	const TArray<TArray<FWFCTileId>>& TileDefArrays = CachedTileIdsByAsset[TileAsset];
	if (!TileDefArrays.IsValidIndex(TileDefIndex))
	{
		return TArray<int32>();
	}
	return TileDefArrays[TileDefIndex];
}

int32 UWFCAssetModel::GetTileIdForAssetAndRotation(const UWFCTileAsset* TileAsset, int32 TileDefIndex, int32 Rotation) const
{
	TArray<FWFCTileId> TileIds = GetTileIdsForAssetAndDef(TileAsset, TileDefIndex);
	for (const FWFCTileId TileId : TileIds)
	{
		if (const FWFCModelAssetTile* Tile = GetTile<FWFCModelAssetTile>(TileId))
		{
			if (Tile->Rotation == Rotation)
			{
				return TileId;
			}
		}
	}
	return INDEX_NONE;
}

bool UWFCAssetModel::ShouldIncludeTileAsset_Implementation(UWFCTileAsset* TileAsset) const
{
	if (!TileAsset)
	{
		return false;
	}
	if (const UWFCAsset* WFCAsset = GetWFCAsset())
	{
		return WFCAsset->TileTagQuery.IsEmpty() || WFCAsset->TileTagQuery.Matches(TileAsset->OwnedTags);
	}
	// no wfc asset so just include everything
	return true;
}

void UWFCAssetModel::GenerateTiles()
{
	Super::GenerateTiles();

	SCOPE_LOG_TIME_FUNC();

	const UWFCAsset* WFCAsset = GetWFCAsset();

	TArray<UWFCTileAsset*> TileAssets;
	GetAllTileAssets(TileAssets);

	const UWFCTileSetTagWeightsConfig* TagWeights = WFCAsset->GetTileConfig<UWFCTileSetTagWeightsConfig>();

	for (const UWFCTileAsset* TileAsset : TileAssets)
	{
		const float Weight = TagWeights->GetTileWeight(TileAsset);

		int32 NumTilesGenerated = 0;
		TArray<int32> AllowedRotations;
		// currently only supporting yaw rotation
		TileAsset->GetAllowedRotations(AllowedRotations);

		// for each possible tile asset rotation...
		for (const int32& Rotation : AllowedRotations)
		{
			// for each tile def in the asset
			const int32 NumDefs = TileAsset->GetNumTileDefs();
			// iterate all tiles of the asset
			for (int32 TileDefIdx = 0; TileDefIdx < NumDefs; ++TileDefIdx)
			{
				// TODO: add virtual func to construct tiles to support subclasses
				TSharedPtr<FWFCModelAssetTile> Tile = MakeShared<FWFCModelAssetTile>();
				if (TagWeights)
				{
					Tile->Weight = Weight;
				}
				Tile->TileAsset = TileAsset;
				Tile->Rotation = Rotation;
				Tile->TileDefIndex = TileDefIdx;

				AddTile(Tile);
				++NumTilesGenerated;
			}
		}

		UE_LOG(LogWFC, Verbose, TEXT("Generated %d tiles from %s (Weight: %f)"), NumTilesGenerated, *TileAsset->GetName(), Weight);
	}

	CacheAssetTileLookup();
}

FString UWFCAssetModel::GetTileDebugString(FWFCTileId TileId) const
{
	if (const FWFCModelAssetTile* AssetTile = GetTile<FWFCModelAssetTile>(TileId))
	{
		return FString::Printf(TEXT("Tile %d (%s)"), TileId, *AssetTile->ToString());
	}
	return Super::GetTileDebugString(TileId);
}

void UWFCAssetModel::CacheAssetTileLookup()
{
	TArray<UWFCTileAsset*> TileAssets;
	GetAllTileAssets(TileAssets);

	// cached ids is indexed by [TileAsset][TileDefIdx], which then contains an array of all tile def ids
	// initialize the cache with all tiles and enough room for all their defs
	CachedTileIdsByAsset.Empty(TileAssets.Num());
	for (const UWFCTileAsset* TileAsset : TileAssets)
	{
		const int32 NumDefs = TileAsset->GetNumTileDefs();
		CachedTileIdsByAsset.Add(TWeakObjectPtr<const UWFCTileAsset>(TileAsset)).AddZeroed(NumDefs);
	}

	// iterate all tiles and assign ids
	for (const TSharedPtr<FWFCModelTile>& Tile : GetTiles())
	{
		const FWFCModelAssetTile* AssetTile = static_cast<FWFCModelAssetTile*>(Tile.Get());
		check(AssetTile != nullptr);
		check(AssetTile->TileAsset.IsValid());

		CachedTileIdsByAsset[AssetTile->TileAsset][AssetTile->TileDefIndex].Add(AssetTile->Id);
	}
}
