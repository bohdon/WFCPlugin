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
};


/**
 * A tile asset that references an actor class to spawn for each tile.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class WFC_API UWFCTileActorAsset : public UWFCTileAsset
{
	GENERATED_BODY()

public:
	/** The actor to spawn for this tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass;
};


/** The edge types for a 2D tile */
UENUM(BlueprintType)
enum class EWFCTile2DEdge : uint8
{
	XPos UMETA(DisplayName = "+X"),
	YPos UMETA(DisplayName = "+Y"),
	XNeg UMETA(DisplayName = "-X"),
	YNeg UMETA(DisplayName = "-Y"),
};


/** The edge types for a 3D tile */
UENUM(BlueprintType)
enum class EWFCTile3DEdge : uint8
{
	XPos UMETA(DisplayName = "+X"),
	YPos UMETA(DisplayName = "+Y"),
	XNeg UMETA(DisplayName = "-X"),
	YNeg UMETA(DisplayName = "-Y"),
	ZPos UMETA(DisplayName = "+Z"),
	ZNeg UMETA(DisplayName = "-Z"),
};


/**
 * A 2D tile with a socket type on each side for use with adjacency constraints.
 * References an actor to be spawned for each tile.
 */
UCLASS()
class WFC_API UWFCTile2DAsset : public UWFCTileActorAsset
{
	GENERATED_BODY()

public:
	UWFCTile2DAsset();

	/** The socket types for all edges of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EWFCTile2DEdge, int32> EdgeSocketTypes;
};


/**
 * A 3D tile with a socket type on each side for use with adjacency constraints.
 * References an actor to be spawned for each tile.
 */
UCLASS()
class WFC_API UWFCTile3DAsset : public UWFCTileActorAsset
{
	GENERATED_BODY()

public:
	UWFCTile3DAsset();

	/** The socket types for all edges of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EWFCTile3DEdge, int32> EdgeSocketTypes;
};
