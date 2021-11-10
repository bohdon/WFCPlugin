// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileSetGenerator.h"
#include "WFCTileSet2DGenerator.generated.h"

class UWFCAdjacencyConstraint;


/**
 * Generates a tile for each asset and possible rotation on a 2D grid.
 * TODO: also generate reflections
 */
UCLASS()
class WFC_API UWFCTileSet2DGenerator : public UWFCTileSetGenerator
{
	GENERATED_BODY()

public:
	UWFCTileSet2DGenerator();

	virtual void GenerateTiles_Implementation(const UWFCTileSet* TileSet, TArray<FWFCTile>& OutTiles) const override;
	virtual void ConfigureGeneratorForTiles_Implementation(const UWFCTileSet* TileSet, const UWFCModel* Model,
	                                                       UWFCGenerator* Generator) const override;
	void ConfigureAdjacencyConstraint(const UWFCModel* Model, UWFCGenerator* Generator, UWFCAdjacencyConstraint* AdjacencyConstraint) const;
};
