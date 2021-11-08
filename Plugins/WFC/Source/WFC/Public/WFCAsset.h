// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear)
	TSubclassOf<UWFCGenerator> GeneratorClass;

	/** The tile set to use, which contains all available tiles as well as relevant data specific to the set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileSetAsset> TileSet;

	/** The grid to populate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<UWFCGrid> Grid;
};
