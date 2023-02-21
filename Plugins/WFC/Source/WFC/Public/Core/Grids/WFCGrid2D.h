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

	/** The size of one cell in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D CellSize;
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

	/** The size of one cell in cm */
	UPROPERTY(BlueprintReadOnly)
	FVector2D CellSize;

	virtual int32 GetNumCells() const override;
	FORCEINLINE virtual int32 GetNumDirections() const override { return 4; }
	virtual FString GetDirectionName(int32 Direction) const override;
	virtual FString GetCellName(int32 CellIndex) const override;
	virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const override;
	virtual FWFCGridDirection RotateDirection(FWFCGridDirection Direction, int32 Rotation) const override;
	virtual FWFCGridDirection InverseRotateDirection(FWFCGridDirection Direction, int32 Rotation) const override;
	virtual int32 CombineRotations(int32 RotationA, int32 RotationB) const override;
	virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const override;

	/** Return the cell index for a grid location */
	UFUNCTION(BlueprintPure)
	int32 GetCellIndexForLocation(FIntPoint GridLocation) const;

	/** Return the grid location for a cell */
	UFUNCTION(BlueprintPure)
	FIntPoint GetLocationForCellIndex(int32 CellIndex) const;

	virtual FVector GetCellWorldLocation(int32 CellIndex, bool bCenter) const override;
	virtual FTransform GetCellWorldTransform(int32 CellIndex, int32 Rotation) const override;
	virtual FTransform GetRotationTransform(int32 Rotation) const override;
	virtual FIntVector GetDirectionVector(int32 Direction) const override;

	static FIntPoint GetDirectionVectorStatic(int32 Direction);
};
