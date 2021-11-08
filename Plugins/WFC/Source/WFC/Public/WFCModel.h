// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCModel.generated.h"


/**
 * A model which contains all possible tiles and tile set info.
 */
UCLASS(BlueprintType)
class WFC_API UWFCModel : public UObject
{
	GENERATED_BODY()

public:
	/** All available tiles in the model */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWFCTile> Tiles;

	/** Get a tile by id */
	FORCEINLINE FWFCTile GetTile(FWFCTileId TileId) const { return Tiles.IsValidIndex(TileId) ? Tiles[TileId] : FWFCTile(); }
};
