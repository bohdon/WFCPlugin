// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileAsset.h"
#include "Core/WFCModel.h"
#include "WFCAssetModel.generated.h"

class UWFCTileSet;


/**
 * A model for use with WFC tile sets and assets.
 */
UCLASS()
class WFC_API UWFCAssetModel : public UWFCModel
{
	GENERATED_BODY()

public:
	/** Return the tile set as a UWFCTileSet. */
	UFUNCTION(BlueprintPure)
	const UWFCTileSet* GetAssetTileSet() const;

	/** Return a FWFCModelAssetTile by id */
	UFUNCTION(BlueprintPure)
	FWFCModelAssetTile GetAssetTile(int32 TileId) const;

	/** Return all tile ids that were generated from a tile asset and tile def index. */
	UFUNCTION(BlueprintCallable)
	TArray<int32> GetTileIdsForAsset(const UWFCTileAsset* TileAsset) const;

	/** Return all tile ids that were generated from a tile asset and tile def index. */
	UFUNCTION(BlueprintCallable)
	TArray<int32> GetTileIdsForAssetAndDef(const UWFCTileAsset* TileAsset, int32 TileDefIndex) const;

	/** Return the tile id that matches a tile asset, tile def index, and rotation */
	UFUNCTION(BlueprintCallable)
	int32 GetTileIdForAssetAndRotation(const UWFCTileAsset* TileAsset, int32 TileDefIndex, int32 Rotation) const;

	virtual void GenerateTiles() override;

	virtual FString GetTileDebugString(FWFCTileId TileId) const override;

protected:
	/** Map of all tile ids indexed by tile asset and tile def index. */
	TMap<TWeakObjectPtr<const UWFCTileAsset>, TArray<TArray<int32>>> CachedTileIdsByAsset;

	/** Update the cached map that allows fast lookup of tile ids by asset and tile def index. */
	void CacheAssetTileLookup();
};
