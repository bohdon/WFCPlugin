// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileAsset.h"
#include "Core/WFCConstraint.h"
#include "WFCCountConstraint.generated.h"


/**
 * Configuration for a count constraint.
 */
UCLASS()
class WFC_API UWFCCountConstraintConfig : public UWFCConstraintConfig
{
	GENERATED_BODY()

public:
	virtual TSubclassOf<UWFCConstraint> GetConstraintClass() const override;
};


USTRUCT()
struct FWFCCountConstraintTileGroup
{
	GENERATED_BODY()

	FWFCCountConstraintTileGroup()
		: MaxCount(0)
	{
	}

	FWFCCountConstraintTileGroup(TArray<FWFCTileId> InTileIds, int32 InMaxCount)
		: TileIds(InTileIds),
		  MaxCount(InMaxCount)
	{
	}

	TArray<FWFCTileId> TileIds;

	/** Maximum number of times this group have a tile selected */
	int32 MaxCount;
};


/**
 * Limits the number of times a tile can be selected.
 */
UCLASS()
class WFC_API UWFCCountConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void Reset() override;
	virtual void NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection) override;
	virtual bool Next() override;

	/** Set the maximum number of times that a set of tiles can be used. */
	void AddTileGroupMaxCountMapping(const TArray<FWFCTileId>& TileIds, int32 MaxCount);

protected:
	/** Array of tile ids in each tile group. */
	TArray<FWFCCountConstraintTileGroup> TileGroups;

	/** Array of tile groups indexed by tile id for fast lookup. */
	TArray<int32> TileIdGroups;

	/** The number of times each tile group has had a tile selected. */
	TArray<int32> TileGroupCounts;

	/** Tile groups that have reached their limit and should be removed during the next update. */
	TArray<int32> TileGroupsToBan;
};


// Tile Asset Count Constraint
// ---------------------------

/**
 * Configuration for a count constraint using tile assets.
 */
UCLASS()
class WFC_API UWFCTileAssetCountConstraintConfig : public UWFCCountConstraintConfig
{
	GENERATED_BODY()

public:
	virtual void Configure(UWFCConstraint* Constraint) const override;
};
