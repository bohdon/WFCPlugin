// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Core/WFCTypes.h"
#include "Engine/DataAsset.h"
#include "WFCTileAsset.generated.h"

class UWFCTilePreviewData;
class UWFCTileAsset;


/**
 * A model tile that has a source asset, rotation,
 * and index into the source tile for large tile support.
 */
USTRUCT(BlueprintType)
struct WFC_API FWFCModelAssetTile : public FWFCModelTile
{
	GENERATED_BODY()

	FWFCModelAssetTile()
		: Rotation(0),
		  TileDefIndex(0)
	{
	}

	/** The rotation of the tile, 0..3 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rotation;

	/** The tile asset representing this tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<const UWFCTileAsset> TileAsset;

	/** The index of the tile def within the asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileDefIndex;

	virtual FString ToString() const override;
};


/**
 * A data asset defining a WFC tile of any size or shape.
 * A large tile asset will contain multiple tile definitions.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class WFC_API UWFCTileAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UWFCTileAsset();

	/** Tags that this tile asset has. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer OwnedTags;

	/**
	 * Return the number of allowed rotations.
	 * Note that the rotations may not be 0..NumRotations, and GetAllowedRotations should be
	 * used when iterating over the exact list of rotations.
	 */
	virtual int32 GetNumRotations() const;

	/** Return the valid rotations that can be used for this tile. */
	virtual void GetAllowedRotations(TArray<int32>& OutRotations) const;

	/** Return the number of tile definitions in this tile asset. */
	virtual int32 GetNumTileDefs() const;

	/** Return the edge type for a tile def and direction. */
	virtual FGameplayTag GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const;

	/** Return the index of a neighbor tile def in this asset for a direction. */
	virtual int32 GetTileDefInDirection(int32 TileDefIndex, FWFCGridDirection Direction) const;

	/** Return the actor class to spawn for a tile def. */
	virtual TSubclassOf<AActor> GetTileDefActorClass(int32 TileDefIndex) const;

	/** Return true if an edge is interior to this tile, meaning it faces another tile in the same asset. */
	virtual bool IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const;

	/** Return the preview data for a tile def. */
	virtual const UWFCTilePreviewData* GetTileDefPreviewData(int32 TileDefIndex) const; 
};
