// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.generated.h"


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


// TODO: allow subclassing FWFCCell to extend with additional per-cell data?

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

	FORCEINLINE bool HasNoCandidates() const { return TileCandidates.Num() == 0; }

	/** Return true if this cell has one valid tile selected for it */
	FORCEINLINE bool HasSelection() const { return TileCandidates.Num() == 1; }

	FORCEINLINE bool HasSelectionOrNoCandidates() const { return TileCandidates.Num() <= 1; }

	void AddCandidate(FWFCTileId TileId);
	void RemoveCandidate(FWFCTileId TileId);

	/** Return the selected tile id, or INDEX_NONE if not selected */
	FORCEINLINE FWFCTileId GetSelectedTileId() const { return TileCandidates.Num() == 1 ? TileCandidates[0] : INDEX_NONE; }
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
	{
	}

	FWFCTile(UObject* InObject)
		: Object(InObject)
	{
	}

	/** The object this tile represents. Could be an actor, texture, or other object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UObject> Object;
};


/**
 * An abstract direction that informs how to locate one tile from another.
 */
UCLASS(Abstract, BlueprintType, DefaultToInstanced, EditInlineNew)
class WFC_API UWFCDirection : public UObject
{
	GENERATED_BODY()

public:
	/** Return the string representation of the direction */
	virtual FString ToString() const { return FString(); }
};

/**
 * A two dimensional direction
 */
UCLASS()
class WFC_API UWFCDirection2D : public UWFCDirection
{
	GENERATED_BODY()

public:
	/** The delta location implied by this direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Delta;
};


/**
 * A three dimensional direction
 */
UCLASS()
class WFC_API UWFCDirection3D : public UWFCDirection
{
	GENERATED_BODY()

public:
	/** The delta location implied by this direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Delta;
};
