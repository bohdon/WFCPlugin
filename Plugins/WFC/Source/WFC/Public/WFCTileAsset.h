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
	UWFCTileAsset();

	/** A value that determines how likely this tile is to be selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight;
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
 * Definition of a tile within a group.
 */
USTRUCT(BlueprintType)
struct FWFCTileDef2D
{
	GENERATED_BODY()

	FWFCTileDef2D()
	{
		EdgeSocketTypes = {
			{EWFCTile2DEdge::XPos, -1},
			{EWFCTile2DEdge::YPos, -1},
			{EWFCTile2DEdge::XNeg, -1},
			{EWFCTile2DEdge::YNeg, -1},
		};
	}

	/** The relative location of this tile within the group. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Location;

	/** The actor to spawn for this tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass;

	/** The socket types for all edges of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EWFCTile2DEdge, int32> EdgeSocketTypes;
};


/**
 * A 2D tile with a socket type on each side for use with adjacency constraints.
 * References an actor to be spawned for each tile.
 */
UCLASS()
class WFC_API UWFCTile2DAsset : public UWFCTileAsset
{
	GENERATED_BODY()

public:
	UWFCTile2DAsset();

	/**
	 * The dimensions of this tile. Tile assets can contain multiple actual tiles
	 * which will have fixed adjacency rules setup for them.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Dimensions;

	/** Can this piece be rotated? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowRotation;

	/** The definitions for each tile within this asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWFCTileDef2D> TileDefs;

	UFUNCTION(BlueprintPure)
	FWFCTileDef2D GetTileDefByLocation(FIntPoint Location) const;

	UFUNCTION(BlueprintPure)
	FWFCTileDef2D GetTileDefByIndex(int32 Index) const;
};


/**
 * Definition of a cell within a 3D tile asset.
 */
USTRUCT(BlueprintType)
struct FWFCTile3DCell
{
	GENERATED_BODY()

	FWFCTile3DCell()
	{
		EdgeSocketTypes = {
			{EWFCTile3DEdge::XPos, -1},
			{EWFCTile3DEdge::YPos, -1},
			{EWFCTile3DEdge::XNeg, -1},
			{EWFCTile3DEdge::YNeg, -1},
			{EWFCTile3DEdge::ZPos, -1},
			{EWFCTile3DEdge::ZNeg, -1},
		};
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Location;

	/** The actor to spawn for this cell */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass;

	/** The socket types for all edges of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EWFCTile3DEdge, int32> EdgeSocketTypes;
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

	/** If true allow this tile to generate rotated variations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowRotations;

	/** Cells that make up this tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWFCTile3DCell> Cells;
};
