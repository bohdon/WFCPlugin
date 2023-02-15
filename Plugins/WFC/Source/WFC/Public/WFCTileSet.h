// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "WFCTileSet.generated.h"

class UWFCTileAsset;


/** Defines a tile weight that should be applied to all tiles with a matching tag. */
USTRUCT(BlueprintType)
struct FWFCTileSetTagWeight
{
	GENERATED_BODY()

	FWFCTileSetTagWeight()
		: Weight(1.0f)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (UIMin = "0", UIMax = "10"))
	float Weight;
};


USTRUCT(BlueprintType)
struct FWFCTileSetEntry
{
	GENERATED_BODY()

	FWFCTileSetEntry()
		: TileAsset(nullptr),
		  Weight(1.0f),
		  MaxCount(-1)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileAsset> TileAsset;

	/** A value that determines how likely this tile is to be selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (UIMin = "0", UIMax = "10"))
	float Weight;

	/** If > 0, the maximum amount of times this tile can be selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCount;
};

/**
 * A set of assets for generating tiles in a model.
 * Each asset is used as the object reference of each tile.
 */
UCLASS(BlueprintType)
class WFC_API UWFCTileSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Weights that should be applied to any tiles with matching tags.
	 * The first matching tag weight will be used if a tile matches multiple tags.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "{Tag} = {Weight}"))
	TArray<FWFCTileSetTagWeight> TagWeights;

	/** The array of tile entries in this set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "{TileAsset} (Weight: {Weight}, Max: {MaxCount})"))
	TArray<FWFCTileSetEntry> Tiles;

	float GetTileWeight(const FWFCTileSetEntry& TileSetEntry) const;

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
};
