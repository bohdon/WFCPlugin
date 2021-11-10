// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCGrid.h"
#include "WFCGrid3D.generated.h"


/**
 * A 3D grid configuration.
 */
UCLASS()
class WFC_API UWFCGrid3DConfig : public UWFCGridConfig
{
	GENERATED_BODY()

public:
	UWFCGrid3DConfig();

	/** The dimensions of the grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Dimensions;
};


/**
 * A 3D grid
 */
UCLASS()
class WFC_API UWFCGrid3D : public UWFCGrid
{
	GENERATED_BODY()

public:
	UWFCGrid3D();

	virtual void Initialize(const UWFCGridConfig* Config) override;

	/** The dimensions of the grid */
	UPROPERTY(BlueprintReadOnly)
	FIntVector Dimensions;

	virtual int32 GetNumCells() const override;
	FORCEINLINE virtual int32 GetNumDirections() const override { return 6; }
	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const override;
	virtual FWFCGridDirection GetRotatedDirection(FWFCGridDirection Direction, int32 Rotation) const override;
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const override;

	/** Return the cell index for a grid location */
	UFUNCTION(BlueprintPure)
	int32 GetCellIndexForLocation(FIntVector GridLocation) const;

	/** Return the grid location for a cell */
	UFUNCTION(BlueprintPure)
	FIntVector GetLocationForCellIndex(int32 CellIndex) const;

	/** Return the 2d vector for a direction */
	static FIntVector GetDirectionVector(FWFCGridDirection Direction)
	{
		switch (Direction)
		{
		case 0:
			return FIntVector(1, 0, 0);
		case 1:
			return FIntVector(0, 1, 0);
		case 2:
			return FIntVector(-1, 0, 0);
		case 3:
			return FIntVector(0, -1, 0);
		case 4:
			return FIntVector(0, 0, 1);
		case 5:
			return FIntVector(0, 0, -1);
		default:
			return FIntVector();
		}
	}
};
