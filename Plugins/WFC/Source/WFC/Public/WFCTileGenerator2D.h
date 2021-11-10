// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCModel.h"
#include "WFCTileGenerator2D.generated.h"

class UWFCAdjacencyConstraint;
class UWFCGenerator;
class UWFCGrid;
class UWFCTile2DAsset;


/**
 * A 2D tile that keeps track of rotation and offset
 * from within the source tile asset.
 */
USTRUCT(BlueprintType)
struct FWFCModelTile2D : public FWFCModelTile
{
	GENERATED_BODY()

	FWFCModelTile2D()
		: Rotation(0),
		  TileDefIndex(0)
	{
	}

	/** The rotation of the tile, 0..3 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rotation;

	/** The tile asset representing this tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UWFCTile2DAsset> TileAsset;

	/** The index of the tile def within the asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileDefIndex;

	virtual FString ToString() const override;
};


/**
 * Generates a tile for each asset and possible rotation on a 2D grid.
 * TODO: also generate reflections
 */
UCLASS()
class WFC_API UWFCTileGenerator2D : public UWFCModel
{
	GENERATED_BODY()

public:
	UWFCTileGenerator2D();

	virtual void GenerateTiles() override;
	virtual void ConfigureGenerator(UWFCGenerator* Generator) override;

	void ConfigureAdjacencyConstraint(const UWFCGenerator* Generator, UWFCAdjacencyConstraint* AdjacencyConstraint) const;

	/**
	 * Return true if TileA and TileB can be adjacent to each other for a direction.
	 * @param Direction The incoming direction from B -> A
	 */
	bool CanTilesBeAdjacent(const FWFCModelTile2D& TileA, const FWFCModelTile2D& TileB,
	                        FWFCGridDirection Direction, const UWFCGrid* Grid) const;

	UFUNCTION(BlueprintPure)
	FWFCModelTile2D GetTileById(int32 TileId) const;
};
