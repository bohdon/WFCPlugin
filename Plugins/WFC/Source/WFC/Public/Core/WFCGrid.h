// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCGrid.generated.h"

class UWFCDirection;


/**
 * A configuration object associated with a grid.
 * Contains all the settings needed to create and initialize a grid.
 */
UCLASS(Abstract, BlueprintType, DefaultToInstanced, EditInlineNew)
class WFC_API UWFCGridConfig : public UObject
{
	GENERATED_BODY()

public:
	UWFCGridConfig();

	/** The grid class to spawn for this config */
	TSubclassOf<class UWFCGrid> GridClass;
};


/**
 * Base class for any type of WFC grid.
 * Grids contain cells, which are then populated with the parts of a tile during generation.
 */
UCLASS(Abstract, BlueprintType)
class WFC_API UWFCGrid : public UObject
{
	GENERATED_BODY()

public:
	UWFCGrid();

	/** Create a new grid object and initialize it using a config */
	static UWFCGrid* NewGrid(UObject* Outer, const UWFCGridConfig* Config);

	/** Initialize the grid */
	virtual void Initialize(const UWFCGridConfig* Config);

	/** Return the total number of cells in this grid */
	UFUNCTION(BlueprintPure)
	virtual int32 GetNumCells() const { return 0; }

	/** Return true if a cell index is valid */
	FORCEINLINE bool IsValidCellIndex(FWFCCellIndex CellIndex) const
	{
		return CellIndex >= 0 && CellIndex < GetNumCells();
	}

	/** Return the number of possible directions, and therefore also neighbors, for a cell in the grid. */
	UFUNCTION(BlueprintPure)
	virtual int32 GetNumDirections() const { return 0; }

	/** Return true if a direction is valid for a cell */
	FORCEINLINE bool IsValidDirection(FWFCGridDirection Direction) const
	{
		return Direction >= 0 && Direction < GetNumDirections();
	}

	/** Return the direction that goes the opposite way of a direction. */
	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const;

	/** Return a rotated direction. */
	virtual FWFCGridDirection RotateDirection(FWFCGridDirection Direction, int32 Rotation) const;

	/** Return a direction applying the inverse of the given rotation. */
	virtual FWFCGridDirection InverseRotateDirection(FWFCGridDirection Direction, int32 Rotation) const;

	/** Return a rotation combined with a delta rotation. */
	virtual int32 CombineRotations(int32 RotationA, int32 RotationB) const;

	/** Return the index of the cell that is one unit in a direction from another cell. */
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const;

	/** Return a readable name for a direction for debugging purposes */
	UFUNCTION(BlueprintPure)
	virtual FString GetDirectionName(int32 Direction) const;

	/** Return a readable name for a cell for debugging purposes */
	UFUNCTION(BlueprintPure)
	virtual FString GetCellName(int32 CellIndex) const;

	/**
	 * Return the world location of a cell.
	 * @param CellIndex The index of a cell
	 * @param bCenter If true, return the center location of the cell, otherwise return the origin that lines up with tile placement
	 */
	UFUNCTION(BlueprintPure)
	virtual FVector GetCellWorldLocation(int32 CellIndex, bool bCenter) const;

	/**
	 * Return the transform of a tile within a cell, considering rotation.
	 * @param CellIndex The index of a cell.
	 * @param Rotation The rotation of the tile.
	 * @return The tile transform, placed at origin of the tile, e.g. the corner at 0,0,0, with rotation.
	 */
	UFUNCTION(BlueprintPure)
	virtual FTransform GetCellWorldTransform(int32 CellIndex, int32 Rotation) const;


	/** Return the transform used to apply a rotation to a tile. */
	UFUNCTION(BlueprintPure)
	virtual FTransform GetRotationTransform(int32 Rotation) const;

	/** Return the 3d vector for a direction, if applicable for this grid. */
	UFUNCTION(BlueprintPure)
	virtual FIntVector GetDirectionVector(int32 Direction) const;
};
