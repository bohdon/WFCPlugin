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


// TODO: allow subclassing FWFCCell to extend with additional per-cell data for use with constraints?

/**
 * Stores the state of a single cell within a grid during WFC generation.
 * Most importantly it keeps track of all tile candidates still available for a cell.
 */
struct FWFCCell
{
	FWFCCell()
	{
	}

	/** The array of tile candidates for this cell. */
	TArray<FWFCTileId> TileCandidates;

	// TODO: consider moving these to reduce memory footprint

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


/**
 * Represents a tile that can be selected for a cell.
 * Maintains a weak reference to an object associated with the tile.
 */
USTRUCT(BlueprintType)
struct FWFCTile
{
	GENERATED_BODY()

	FWFCTile()
		: Rotation(0)
	{
	}

	FWFCTile(UObject* InObject, int32 InRotation = 0)
		: Object(InObject),
		  Rotation(InRotation)
	{
	}

	/** The object this tile represents. Could be an actor, texture, or other object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UObject> Object;

	/** The rotation of this tile relative to its definition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rotation;
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
