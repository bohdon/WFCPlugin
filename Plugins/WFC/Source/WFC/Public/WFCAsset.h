// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WFCAsset.generated.h"

class UWFCCellSelector;
class UWFCConstraint;
class UWFCGenerator;
class UWFCGridConfig;
class UWFCModel;
class UWFCTileSet;


/**
 * A data asset used to define the tiles and classes for use in a WFC generation.
 */
UCLASS(BlueprintType, Blueprintable)
class WFC_API UWFCAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UWFCAsset();

	/** The generator class to use, which handles applying all constraints and running cell and tile selection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, DisplayName = "Generator")
	TSubclassOf<UWFCGenerator> GeneratorClass;

	/** The constraints to apply, in order of priority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Constraints")
	TArray<TSubclassOf<UWFCConstraint>> ConstraintClasses;

	/** The cell selectors to use, in order of priority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Cell Selectors")
	TArray<TSubclassOf<UWFCCellSelector>> CellSelectorClasses;

	/**
	 * The model class to use, which will generate all tiles and map them to tile ids.
	 * The model is also able to configure the generator and all constraints as desired.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Model")
	TSubclassOf<UWFCModel> ModelClass;

	/** The grid and configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<UWFCGridConfig> GridConfig;

	/** The tile set to use, which contains all available tiles as well as relevant data specific to the set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileSet> TileSet;
};
