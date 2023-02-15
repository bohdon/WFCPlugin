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
 * Configuration for a fixed tile constraint.
 */
UCLASS(Abstract)
class WFC_API UWFCFixedTileConstraintConfig : public UWFCConstraintConfig
{
	GENERATED_BODY()

public:
	virtual TSubclassOf<UWFCConstraint> GetConstraintClass() const override;
};


/**
 * A constraint that specifies exact tiles that must be used for specific cells.
 */
UCLASS()
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


/**
 * Configuration for a 3d fixed tile constraint.
 */
UCLASS()
class WFC_API UWFCFixedTile3DConstraintConfig : public UWFCFixedTileConstraintConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWFCFixedTileConstraint3DEntry> FixedTiles;

	virtual void Configure(UWFCConstraint* Constraint) const override;
};
