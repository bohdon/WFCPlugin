// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCConstraint.h"
#include "Engine/DataAsset.h"
#include "WFCAsset.generated.h"

class UWFCGenerator;
class UWFCGrid;
class UWFCModel;
class UWFCTileSetAsset;


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

	/** The grid to populate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<UWFCGrid> Grid;

	/** The tile set to use, which contains all available tiles as well as relevant data specific to the set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileSetAsset> TileSet;
};
