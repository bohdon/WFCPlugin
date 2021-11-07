// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCModel.generated.h"


/**
 * A model which defines the rules for how tiles should be constrained.
 * @ee UWFCAdjacentModel for a more concrete example.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class WFC_API UWFCModel : public UObject
{
	GENERATED_BODY()

public:
	/** Array of all available tiles for use in this model */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FWFCTile> AvailableTiles;
};


/**
 * A model which requires adjacent tiles to have matching patterns.
 * Ex.
 *	- A 2d tile set has two patterns: grass or dirt.
 *	- Each tile defines a pattern for all 4 sides of the tile
 *	- Tiles can only be placed next to each other if their pattern matches (e.g. grass to grass or dirt to dirt)
 */
UCLASS()
class WFC_API UWFCAdjacentModel : public UWFCModel
{
	GENERATED_BODY()

public:
	/** All possible directions to consider for the tiles, e.g. up down left and right for a 2d tile set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, meta = (ShowOnlyInnerProperties))
	TArray<TObjectPtr<UWFCDirection>> Directions;
};
