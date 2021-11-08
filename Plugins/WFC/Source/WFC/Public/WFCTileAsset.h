// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WFCTileAsset.generated.h"


/**
 * A data asset defining a WFC tile of any size or shape
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class WFC_API UWFCTileAsset : public UDataAsset
{
	GENERATED_BODY()

public:
};


/** The edge types for a 2d tile */
UENUM(BlueprintType)
enum class EWFCTile2dEdge : uint8
{
	XPos UMETA(DisplayName = "+X"),
	XNeg UMETA(DisplayName = "-X"),
	YPos UMETA(DisplayName = "+Y"),
	YNeg UMETA(DisplayName = "-Y"),
};


/**
 * A 2d tile with a socket type on each side for use with adjacency constraints.
 * References an actor to be spawned for each tile.
 */
UCLASS()
class WFC_API UWFCTile2dAsset : public UWFCTileAsset
{
	GENERATED_BODY()

public:
	UWFCTile2dAsset();

	/** The actor to spawn for this tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass;

	/** The socket types for all edges of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EWFCTile2dEdge, int32> EdgeSocketTypes;
};
