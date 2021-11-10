// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileSetGenerator.h"
#include "WFCTileSet3DGenerator.generated.h"

class UWFCAdjacencyConstraint;


/**
 * Generates a tile for each asset and possible 3D rotations.
 */
UCLASS()
class WFC_API UWFCTileSet3DGenerator : public UWFCTileSetGenerator
{
	GENERATED_BODY()

public:
	UWFCTileSet3DGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowYawRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowPitchRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowRollRotation;

	virtual void GenerateTiles_Implementation(const UWFCTileSet* TileSet, TArray<FWFCTile>& OutTiles) const override;
	virtual void ConfigureGeneratorForTiles_Implementation(const UWFCTileSet* TileSet, const UWFCModel* Model,
	                                                       UWFCGenerator* Generator) const override;

	void ConfigureAdjacencyConstraint(const UWFCModel* Model, const UWFCGenerator* Generator,
	                                  UWFCAdjacencyConstraint* AdjacencyConstraint) const;
};
