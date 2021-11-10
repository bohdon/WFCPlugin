// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCGrid.h"
#include "WFCGrid2D.generated.h"


/**
 * A 2D grid configuration.
 */
UCLASS()
class WFC_API UWFCGrid2DConfig : public UWFCGridConfig
{
	GENERATED_BODY()

public:
	UWFCGrid2DConfig();

	/** The dimensions of the grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Dimensions;
};


/**
 * A 2D grid
 */
UCLASS()
class WFC_API UWFCGrid2D : public UWFCGrid
{
	GENERATED_BODY()

public:
	UWFCGrid2D();

	virtual void Initialize(const UWFCGridConfig* Config) override;

	/** The dimensions of the grid */
	UPROPERTY(BlueprintReadOnly)
	FIntPoint Dimensions;

	virtual int32 GetNumCells() const override;
	FORCEINLINE virtual int32 GetNumDirections() const override { return 4; }
	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const override;
	virtual FWFCGridDirection RotateDirection(FWFCGridDirection Direction, int32 Rotation) const override;
	virtual FWFCGridDirection InverseRotateDirection(FWFCGridDirection Direction, int32 Rotation) const override;
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const override;

	/** Return the cell index for a grid location */
	UFUNCTION(BlueprintPure)
	int32 GetCellIndexForLocation(FIntPoint GridLocation) const;

	/** Return the grid location for a cell */
	UFUNCTION(BlueprintPure)
	FIntPoint GetLocationForCellIndex(int32 CellIndex) const;

	/** Return the 2d vector for a direction */
	static FIntPoint GetDirectionVector(FWFCGridDirection Direction);
};
