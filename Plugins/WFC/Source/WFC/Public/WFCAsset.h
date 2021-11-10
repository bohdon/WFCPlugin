// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCConstraint.h"
#include "Engine/DataAsset.h"
#include "WFCAsset.generated.h"

class UWFCGenerator;
class UWFCGridConfig;
class UWFCModel;
class UWFCTileSet;
class UWFCTileSetGenerator;


/**
 * A data asset used to define the tiles and classes for use in a WFC generation.
 */
UCLASS(BlueprintType, Blueprintable)
class WFC_API UWFCAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UWFCAsset();

	/** The generator class to use, which controls the rules of how each cell is selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, meta = (DisplayName = "Generator"))
	TSubclassOf<UWFCGenerator> GeneratorClass;

	/** The constraints to apply, in order of priority, during generation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Constraints"))
	TArray<TSubclassOf<UWFCConstraint>> ConstraintClasses;

	/** The model class to use, which will contain all tile definitions and control the mapping of tile ids to tiles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, meta = (DisplayName = "Model"))
	TSubclassOf<UWFCModel> ModelClass;

	/** The grid and configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<UWFCGridConfig> GridConfig;

	/** The tile set to use, which contains all available tiles as well as relevant data specific to the set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileSet> TileSet;

	/**
	 * The generator to use for converting the tile sets into tiles within the model,
	 * as well as configuring the generator and any constraints in ways specific to the tile set.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<UWFCTileSetGenerator> TileGenerator;
};
