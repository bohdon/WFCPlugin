// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WFCAsset.generated.h"

class UWFCModel;
class UWFCTileAssetSet;


/**
 * A data asset used to define the tiles and classes for use in a WFC generation.
 */
UCLASS(BlueprintType, Blueprintable)
class WFC_API UWFCAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The WFC model to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<UWFCModel> Model;

	/** The tile asset set to use as the available tiles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileAssetSet> TileSet;
};
