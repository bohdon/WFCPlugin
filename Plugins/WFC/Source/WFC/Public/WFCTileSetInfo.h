// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WFCTileSetInfo.generated.h"


/**
 * Base class for any information that is associated with a set
 * of tiles and is relevant for how they will be selected.
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class WFC_API UWFCTileSetInfo : public UObject
{
	GENERATED_BODY()
};


/**
 * Tile set info for tiles that have distinct socket types used for tile adjacency constraints.
 */
UCLASS()
class WFC_API UWFCSocketedTileSetInfo : public UWFCTileSetInfo
{
	GENERATED_BODY()

public:
	typedef int32 FSocketType;

	/** The number of possible socket types for any tile in the set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumSocketTypes;
};
