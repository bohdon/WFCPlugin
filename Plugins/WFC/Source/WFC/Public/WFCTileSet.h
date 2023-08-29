// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WFCTileSet.generated.h"

class UWFCTileAsset;


/**
 * A set of assets for generating tiles in a model.
 * Each asset is used as the object reference of each tile.
 */
UCLASS(BlueprintType)
class WFC_API UWFCTileSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The array of tile entries in this set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UWFCTileAsset>> TileAssets;

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
};
