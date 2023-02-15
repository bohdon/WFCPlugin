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

	/** The size of one cell in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CellSize;
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
	UPROPERTY(BlueprintReadWrite)
	FIntVector Dimensions;

	/** The size of one cell in cm */
	UPROPERTY(BlueprintReadWrite)
	FVector CellSize;

	virtual int32 GetNumCells() const override;
	FORCEINLINE virtual int32 GetNumDirections() const override { return 6; }
	virtual FString GetDirectionName(int32 Direction) const override;
	virtual FString GetCellName(int32 CellIndex) const override;
	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const override;
	virtual FWFCGridDirection RotateDirection(FWFCGridDirection Direction, int32 Rotation) const override;
	virtual FWFCGridDirection InverseRotateDirection(FWFCGridDirection Direction, int32 Rotation) const override;
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const override;

	/** Return the cell index for a grid location */
	UFUNCTION(BlueprintPure)
	int32 GetCellIndexForLocation(FIntVector GridLocation) const;

	/** Return the grid location for a cell */
	UFUNCTION(BlueprintPure)
	FIntVector GetLocationForCellIndex(int32 CellIndex) const;

	/** Return the 2d vector for a direction */
	UFUNCTION(BlueprintPure)
	static FIntVector GetDirectionVector(int32 Direction);

	virtual FVector GetCellWorldLocation(int32 CellIndex, bool bCenter) const override;
};
