// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCGrid.generated.h"

class UWFCDirection;


/**
 * Base class for any type of WFC grid.
 * Grids contain cells, which are then populated with the parts of a tile during generation.
 */
UCLASS(Abstract, BlueprintType, DefaultToInstanced, EditInlineNew)
class WFC_API UWFCGrid : public UObject
{
	GENERATED_BODY()

public:
	UWFCGrid();

	/** Return the total number of cells in this grid */
	virtual int32 GetNumCells() const { return 0; }

	/** Return true if a cell index is valid */
	FORCEINLINE bool IsValidCellIndex(FWFCCellIndex CellIndex) const
	{
		return CellIndex >= 0 && CellIndex < GetNumCells();
	}

	/** Return the number of possible directions, and therefore also neighbors, for a cell in the grid. */
	virtual int32 GetNumDirections() const { return 0; }

	/** Return true if a direction is valid for a cell */
	FORCEINLINE bool IsValidDirection(FWFCGridDirection Direction) const
	{
		return Direction >= 0 && Direction < GetNumDirections();
	}

	/** Return the direction that goes the opposite way of a direction. */
	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const { return INDEX_NONE; }

	/** Return a rotated direction. */
	virtual FWFCGridDirection GetRotatedDirection(FWFCGridDirection Direction, int32 Rotation) const { return INDEX_NONE; }

	/** Return the index of the cell that is one unit in a direction from another cell. */
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const { return INDEX_NONE; }
};


/**
 * A 2d grid
 */
UCLASS()
class WFC_API UWFCGrid2d : public UWFCGrid
{
	GENERATED_BODY()

public:
	UWFCGrid2d();

	/** The size of the grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint GridDimensions;

	FORCEINLINE virtual int32 GetNumCells() const override { return GridDimensions.X * GridDimensions.Y; }
	FORCEINLINE virtual int32 GetNumDirections() const override { return 4; }
	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const override;
	virtual FWFCGridDirection GetRotatedDirection(FWFCGridDirection Direction, int32 Rotation) const override;
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const override;

	/** Return the cell index for a grid location */
	UFUNCTION(BlueprintPure)
	int32 GetCellIndexForLocation(FIntPoint GridLocation) const;

	/** Return the grid location for a cell */
	UFUNCTION(BlueprintPure)
	FIntPoint GetLocationForCellIndex(int32 CellIndex) const;

	/** Return the 2d vector for a direction */
	static FIntPoint GetDirectionVector(FWFCGridDirection Direction)
	{
		switch (Direction)
		{
		case 0:
			return FIntPoint(1, 0);
		case 1:
			return FIntPoint(0, 1);
		case 2:
			return FIntPoint(-1, 0);
		case 3:
			return FIntPoint(0, -1);
		default:
			return FIntPoint();
		}
	}
};
