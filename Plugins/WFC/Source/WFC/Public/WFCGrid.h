// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

	/**
	 * All possible directions to consider for navigating between cells in this grid.
	 * Ex. Up, Down, Left and Right for a 2d grid.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<TObjectPtr<UWFCDirection>> Directions;

	/** Return the total number of cells in this grid */
	virtual int32 GetNumCells() const { return 0; }
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

	virtual int32 GetNumCells() const override;

	/** Return the grid location for a cell */
	UFUNCTION(BlueprintPure)
	FIntPoint GetCellLocation(int32 CellIndex) const;
};
