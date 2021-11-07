// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WFCTileAssetSet.generated.h"

class UWFCTileAsset;


/**
 * A set of WFCTileAssets for use in a model
 */
UCLASS(BlueprintType)
class WFC_API UWFCTileAssetSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The array of tile assets in this set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UWFCTileAsset>> TileAssets;
};
