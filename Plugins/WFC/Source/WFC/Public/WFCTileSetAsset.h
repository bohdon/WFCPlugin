// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCTypes.h"
#include "Engine/DataAsset.h"
#include "WFCTileSetAsset.generated.h"

class UWFCTileAsset;
class UWFCTileSetInfo;


/**
 * A set of WFCTileAssets for use in a model
 */
UCLASS(BlueprintType)
class WFC_API UWFCTileSetAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Information about the set of tiles being used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<UWFCTileSetInfo> TileSetInfo;

	/** The array of tile assets in this set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UWFCTileAsset>> TileAssets;

	/** Create and return an array of FWFCTiles representing each tile asset in the set */
	UFUNCTION(BlueprintCallable)
	void GenerateTiles(TArray<FWFCTile>& OutTiles);
};
