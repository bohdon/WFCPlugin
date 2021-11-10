// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCTypes.h"
#include "Engine/DataAsset.h"
#include "WFCTileAsset.generated.h"


/**
 * A model tile that has a source asset, rotation,
 * and index into the source tile for large tile support.
 */
USTRUCT(BlueprintType)
struct FWFCModelAssetTile : public FWFCModelTile
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
	TWeakObjectPtr<class UWFCTileAsset> TileAsset;

	/** The index of the tile def within the asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileDefIndex;

	virtual FString ToString() const override;
};


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
