// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "WFCTileSetConfig.generated.h"

class UWFCTileAsset;


/**
 * Base class for a configuration that defines properties for a set of tiles.
 * These are used for properties that may change from one WFC to the next,
 * and therefore shouldn't be defined on the tiles themselves, e.g. weights.
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew)
class UWFCTileSetConfig : public UObject
{
	GENERATED_BODY()
};


/** Defines a tile weight that should be applied to all tiles with a matching tag. */
USTRUCT(BlueprintType)
struct FWFCTileTagWeight
{
	GENERATED_BODY()

	FWFCTileTagWeight()
		: Weight(1.0f)
	{
	}

	/** Designer notes about this weight rule. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignerNotes;

	/** Tiles with this weight must match this tag query. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagQuery TagQuery;

	/** The tile weight for tiles matching this tag query. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (UIMin = "0", UIMax = "10"))
	float Weight;
};


/**
 * Defines selection probability weights for tiles by tag. */
UCLASS(DisplayName = "Tag Weights")
class UWFCTileSetTagWeightsConfig : public UWFCTileSetConfig
{
	GENERATED_BODY()

public:
	/**
	 * Weights that should be applied to any tiles with matching tags.
	 * The first matching tag weight will be used if a tile matches multiple tags.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "{DesignerNotes} ({Weight})"), Category = "TagWeights")
	TArray<FWFCTileTagWeight> Weights;

	float GetTileWeight(const UWFCTileAsset* TileAsset) const;
};
