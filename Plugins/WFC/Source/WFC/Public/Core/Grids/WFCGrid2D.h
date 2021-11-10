// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCGrid.h"
#include "WFCGrid2D.generated.h"


/**
 * A 2D grid
 */
UCLASS()
class WFC_API UWFCGrid2D : public UWFCGrid
{
	GENERATED_BODY()

public:
	UWFCGrid2D();

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
