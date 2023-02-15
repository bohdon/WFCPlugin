// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileSet.h"
#include "Core/WFCConstraint.h"
#include "WFCCountConstraint.generated.h"


/**
 * Limit the max count of tiles.
 */
UCLASS(Abstract)
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

	/** Return the number of max count constraint groups that are defined. */
	int32 GetNumMaxCountGroups() const { return TileGroupMaxCounts.Num(); }

protected:
	/** Array of tile ids in each tile group, and their maximum count. */
	TArray<FWFCCountConstraintTileGroup> TileGroupMaxCounts;

	/** Map of tile groups indexed by tile id for fast lookup. */
	TMap<int32, int32> TileIdsToGroups;

	/** The number of times each tile group has had a tile selected. */
	TArray<int32> TileGroupCurrentCounts;

	/** Tile groups that have reached their limit and should be removed during the next update. */
	TArray<int32> TileGroupsToBan;

	/** Tile groups have already been banned. */
	TArray<int32> BannedGroups;
};


/** Defines a max count that should be applied to all tiles with a matching tag. */
USTRUCT(BlueprintType)
struct FWFCTileTagMaxCount
{
	GENERATED_BODY()

	FWFCTileTagMaxCount()
		: MaxCount(1)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (UIMin = "0", UIMax = "10"))
	int32 MaxCount;
};


/**
 * Defines a maximum count for tiles by tag.
 */
UCLASS(DisplayName = "Tag Max Counts")
class WFC_API UWFCTileSetTagMaxCountsConfig : public UWFCTileSetConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "{Tag} = {MaxCount}"), Category = "TagMaxCounts")
	TArray<FWFCTileTagMaxCount> MaxCounts;

	int32 GetTileMaxCount(const UWFCTileAsset* TileAsset) const;
};


/**
 * Limit the max count of tiles using a Tax Max Counts config defined in the tile set.
 */
UCLASS(DisplayName = "Tile Asset Max Count Constraint Config")
class WFC_API UWFCTileAssetCountConstraintConfig : public UWFCCountConstraintConfig
{
	GENERATED_BODY()

public:
	virtual void Configure(UWFCConstraint* Constraint) const override;
};
