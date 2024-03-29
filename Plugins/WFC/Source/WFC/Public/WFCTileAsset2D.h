﻿// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WFCTileAsset.h"
#include "WFCTileAsset2D.generated.h"


/** The edge types for a 2D tile */
UENUM(BlueprintType)
enum class EWFCTile2DEdge : uint8
{
	XPos UMETA(DisplayName = "+X"),
	YPos UMETA(DisplayName = "+Y"),
	XNeg UMETA(DisplayName = "-X"),
	YNeg UMETA(DisplayName = "-Y"),
};


/**
 * Definition of a tile within a group.
 */
USTRUCT(BlueprintType)
struct FWFCTileDef2D
{
	GENERATED_BODY()

	FWFCTileDef2D()
		: Location(FIntPoint::ZeroValue)
	{
		EdgeTypes = {
			{EWFCTile2DEdge::XPos, FGameplayTag::EmptyTag},
			{EWFCTile2DEdge::YPos, FGameplayTag::EmptyTag},
			{EWFCTile2DEdge::XNeg, FGameplayTag::EmptyTag},
			{EWFCTile2DEdge::YNeg, FGameplayTag::EmptyTag},
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
	TMap<EWFCTile2DEdge, FGameplayTag> EdgeTypes;
};


/**
 * A 2D tile with a socket type on each side for use with adjacency constraints.
 * References an actor to be spawned for each tile.
 */
UCLASS()
class WFC_API UWFCTileAsset2D : public UWFCTileAsset
{
	GENERATED_BODY()

public:
	UWFCTileAsset2D();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "Location"))
	TArray<FWFCTileDef2D> TileDefs;

	/** Return a tile def by location, as well as its index. */
	UFUNCTION(BlueprintPure)
	FWFCTileDef2D GetTileDefByLocation(FIntPoint Location, int32& Index) const;

	UFUNCTION(BlueprintPure)
	FWFCTileDef2D GetTileDefByIndex(int32 Index) const;

	virtual void GetAllowedRotations(TArray<int32>& OutRotations) const override;
	virtual int32 GetNumTileDefs() const override { return TileDefs.Num(); }
	virtual FGameplayTag GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const override;
	virtual int32 GetTileDefInDirection(int32 TileDefIndex, FWFCGridDirection Direction) const override;
	virtual TSubclassOf<AActor> GetTileDefActorClass(int32 TileDefIndex) const override;
	virtual bool IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) override;
#endif
};
