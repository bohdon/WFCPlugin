// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileAsset3D.h"
#include "Core/WFCConstraint.h"
#include "WFCFixedTileConstraint.generated.h"


struct FWFCFixedTileConstraintEntry
{
	FWFCFixedTileConstraintEntry()
		: CellIndex(INDEX_NONE),
		  TileId(INDEX_NONE)
	{
	}

	FWFCFixedTileConstraintEntry(FWFCCellIndex InCellIndex, FWFCTileId InTileId)
		: CellIndex(InCellIndex),
		  TileId(InTileId)
	{
	}

	FWFCCellIndex CellIndex;
	FWFCTileId TileId;
};


/**
 * A constraint that specifies exact tiles that must be used for specific cells.
 */
UCLASS(Abstract)
class WFC_API UWFCFixedTileConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void Reset() override;
	virtual bool Next() override;

	/** Add a tile constraint to be applied next time this constraint runs. */
	void AddFixedTileMapping(FWFCCellIndex CellIndex, FWFCTileId TileId);

protected:
	TArray<FWFCFixedTileConstraintEntry> FixedTileMappings;

	bool bDidApplyInitialConstraint;
};


// 3D Fixed Tile Constraints
// -------------------------

USTRUCT(BlueprintType)
struct FWFCFixedTileConstraint3DEntry
{
	GENERATED_BODY()

	FWFCFixedTileConstraint3DEntry()
		: CellLocation(FIntVector::ZeroValue),
		  TileAsset(nullptr),
		  TileRotation(0)
	{
	}

	/** The location of the cell to constrain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector CellLocation;

	/** The tile asset containing the tile to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCTileAsset3D> TileAsset;

	/** The rotation of the tile to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileRotation;
};


UCLASS(Abstract)
class WFC_API UWFCFixedTile3DConstraint : public UWFCFixedTileConstraint
{
	GENERATED_BODY()

public:
	/** The specific tiles to place and their locations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FWFCFixedTileConstraint3DEntry> FixedTiles;

	virtual void Initialize(UWFCGenerator* InGenerator) override;
};
