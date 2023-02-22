// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.generated.h"


/** Represents a direction pointing from one cell in a grid to another. */
typedef int32 FWFCGridDirection;

/** Represents the id of a cell within a grid. */
typedef int32 FWFCCellIndex;

/** Represents the id of a tile that can be selected for one cell of a grid. */
typedef int32 FWFCTileId;


UENUM(BlueprintType)
enum class EWFCGeneratorState : uint8
{
	/** Not yet started */
	None,
	/** A tile or tiles has not yet been selected. */
	InProgress,
	/** One or all tiles in question have been selected successfully. */
	Finished,
	/** A contradiction occured preventing the selection of a tile. */
	Error,
};


/** The different phases when iterating a WFCGenerator. */
UENUM(BlueprintType)
enum class EWFCGeneratorStepPhase : uint8
{
	None,
	Constraints,
	Selection,
};


/**
 * Stores the state of a single cell within a grid during WFC generation.
 * Most importantly it keeps track of all tile candidates still available for a cell.
 */
struct FWFCCell
{
	FWFCCell()
		: CollapsePhase(EWFCGeneratorStepPhase::Selection)
	{
	}

	/** The array of tile candidates for this cell. */
	TArray<FWFCTileId> TileCandidates;

	/** The phase during which this cell was fully collapsed. */
	EWFCGeneratorStepPhase CollapsePhase;

	FORCEINLINE bool HasNoCandidates() const { return TileCandidates.Num() == 0; }

	/** Return true if this cell has one valid tile selected for it */
	FORCEINLINE bool HasSelection() const { return TileCandidates.Num() == 1; }

	FORCEINLINE bool HasSelectionOrNoCandidates() const { return TileCandidates.Num() <= 1; }

	/** @return True if the candidates were changed */
	bool AddCandidate(FWFCTileId TileId);

	/** @return True if the candidates were changed */
	bool RemoveCandidate(FWFCTileId TileId);

	/** Return the selected tile id, or INDEX_NONE if not selected */
	FORCEINLINE FWFCTileId GetSelectedTileId() const { return TileCandidates.Num() == 1 ? TileCandidates[0] : INDEX_NONE; }

	/** Return true if any of the tile ids are a candidate for a cell. */
	bool HasAnyMatchingCandidate(const TArray<FWFCTileId>& TileIds) const;
};


struct FWFCCellIndexAndDirection
{
	FWFCCellIndexAndDirection()
		: CellIndex(INDEX_NONE),
		  Direction(INDEX_NONE)
	{
	}

	FWFCCellIndexAndDirection(FWFCCellIndex InCellIndex, FWFCGridDirection InDirection)
		: CellIndex(InCellIndex),
		  Direction(InDirection)
	{
	}

	FWFCCellIndex CellIndex;

	FWFCGridDirection Direction;

	bool operator==(const FWFCCellIndexAndDirection& Other) const
	{
		return CellIndex == Other.CellIndex && Direction == Other.Direction;
	}

	bool operator!=(const FWFCCellIndexAndDirection& Other) const
	{
		return !(operator==(Other));
	}

	friend uint32 GetTypeHash(const FWFCCellIndexAndDirection& IndexAndDirection)
	{
		return HashCombine(GetTypeHash(IndexAndDirection.CellIndex), GetTypeHash(IndexAndDirection.Direction));
	}
};


struct FWFCCellIndexAndTileId
{
	FWFCCellIndexAndTileId()
		: CellIndex(INDEX_NONE),
		  TileId(INDEX_NONE)
	{
	}

	FWFCCellIndexAndTileId(FWFCCellIndex InCellIndex, FWFCGridDirection InTileId)
		: CellIndex(InCellIndex),
		  TileId(InTileId)
	{
	}

	FWFCCellIndex CellIndex;

	FWFCTileId TileId;

	bool operator==(const FWFCCellIndexAndTileId& Other) const
	{
		return CellIndex == Other.CellIndex && TileId == Other.TileId;
	}

	bool operator!=(const FWFCCellIndexAndTileId& Other) const
	{
		return !(operator==(Other));
	}

	friend uint32 GetTypeHash(const FWFCCellIndexAndTileId& IndexAndDirection)
	{
		return HashCombine(GetTypeHash(IndexAndDirection.CellIndex), GetTypeHash(IndexAndDirection.TileId));
	}
};


/**
 * Contains all relevant data about a tile needed to select tiles
 * and generate final output from the model once selected.
 * Designed to be subclassed to add any additional or relevant info about each tile.
 */
USTRUCT(BlueprintType)
struct FWFCModelTile
{
	GENERATED_BODY()

	FWFCModelTile()
		: Id(INDEX_NONE),
		  Weight(1.f)
	{
	}

	virtual ~FWFCModelTile()
	{
	}

	/** The id of this tile */
	UPROPERTY(BlueprintReadOnly)
	int32 Id;

	/** The probability weight of this tile */
	UPROPERTY(BlueprintReadOnly)
	float Weight;

	virtual FString ToString() const { return FString::Printf(TEXT("[%d]"), Id); }
};
