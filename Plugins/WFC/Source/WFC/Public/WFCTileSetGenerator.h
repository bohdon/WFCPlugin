// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCTypes.h"
#include "UObject/Object.h"
#include "WFCTileSetGenerator.generated.h"

class UWFCGenerator;
class UWFCModel;
class UWFCTileSet;


/**
 * The class responsible for converting a tile set asset into
 * an array of tiles for use in a WFCModel.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class UWFCTileSetGenerator : public UObject
{
	GENERATED_BODY()

public:
	/** Create and return an array of FWFCTiles representing each tile asset in the set */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void GenerateTiles(const UWFCTileSet* TileSet, TArray<FWFCTile>& OutTiles) const;

	/**
	 * Configure a generator for use with this tile set.
	 * Tile sets can use their own logic for setting up constraints or other initialization here.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ConfigureGeneratorForTiles(const UWFCTileSet* TileSet, const UWFCModel* Model, UWFCGenerator* Generator) const;
};
