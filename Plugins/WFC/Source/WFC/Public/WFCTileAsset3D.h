// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileAsset.h"
#include "WFCTileAsset3D.generated.h"


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
 * Definition of a 3D tile unit.
 */
USTRUCT(BlueprintType)
struct FWFCTileDef3D
{
	GENERATED_BODY()

	FWFCTileDef3D()
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
class WFC_API UWFCTileAsset3D : public UWFCTileAsset
{
	GENERATED_BODY()

public:
	UWFCTileAsset3D();

	/**
	 * The dimensions of this tile. Tile assets can contain multiple actual tiles
	 * which will have fixed adjacency rules setup for them.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Dimensions;

	/** If true allow this tile to generate rotated variations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowRotation;

	/** The individual tiles that make up this group of tiles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWFCTileDef3D> TileDefs;

	UFUNCTION(BlueprintPure)
	FWFCTileDef3D GetTileDefByLocation(FIntVector Location) const;

	UFUNCTION(BlueprintPure)
	FWFCTileDef3D GetTileDefByIndex(int32 Index) const;
};
