// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCCellSelector.generated.h"

class UWFCGenerator;


/**
 * Handles the selection of cells to collapse next when running a WFC generator.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class WFC_API UWFCCellSelector : public UObject
{
	GENERATED_BODY()

public:
	/** Return the generator that owns this selector */
	FORCEINLINE UWFCGenerator* GetGenerator() const { return Generator; }

	/** Initialize the selector for a generator */
	virtual void Initialize(UWFCGenerator* InGenerator);

	/** Reset the selector to its initialized state */
	virtual void Reset();

	/** Select and return the next best cell to collapse. */
	virtual FWFCCellIndex SelectNextCell();

protected:
	UPROPERTY(Transient)
	TObjectPtr<UWFCGenerator> Generator;
};

/**
 * Selects an unbiased random uncollapsed cell.
 */
UCLASS()
class WFC_API UWFCRandomCellSelector : public UWFCCellSelector
{
	GENERATED_BODY()

public:
	virtual FWFCCellIndex SelectNextCell() override;
};
