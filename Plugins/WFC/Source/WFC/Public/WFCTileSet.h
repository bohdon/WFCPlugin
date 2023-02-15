// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "WFCTileSet.generated.h"

class UWFCTileAsset;


/** Defines a tile weight that should be applied to all tiles with a matching tag. */
USTRUCT(BlueprintType)
struct FWFCTileTagWeight
{
	GENERATED_BODY()

	FWFCTileTagWeight()
		: Weight(1.0f)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (UIMin = "0", UIMax = "10"))
	float Weight;
};

/**
 * Base class for a configuration defined in a tile set that can be
 * used by the generator or constraints.
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew)
class UWFCTileSetConfig : public UObject
{
	GENERATED_BODY()
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "{Tag} = {Weight}"), Category = "TagWeights")
	TArray<FWFCTileTagWeight> Weights;

	float GetTileWeight(const UWFCTileAsset* TileAsset) const;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<TObjectPtr<UWFCTileSetConfig>> Configs;

	/** The array of tile entries in this set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UWFCTileAsset>> TileAssets;

	/** Return a config by class, or nullptr if it does not exist. */
	template <class T>
	T* GetConfig() const
	{
		for (UWFCTileSetConfig* Config : Configs)
		{
			if (Config->IsA<T>())
			{
				return Cast<T>(Config);
			}
		}
		return nullptr;
	}

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
};
