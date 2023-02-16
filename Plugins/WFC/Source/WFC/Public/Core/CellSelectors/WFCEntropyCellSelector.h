// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCCellSelector.h"
#include "WFCEntropyCellSelector.generated.h"


/**
 * Returns a random cell biasing those with the lowest entropy,
 * roughly equivalent to those with the least number of candidates remaining.
 */
UCLASS()
class WFC_API UWFCEntropyCellSelector : public UWFCCellSelector
{
	GENERATED_BODY()

public:
	UWFCEntropyCellSelector();

	/** Random deviation used to vary selection of cells with the same entropy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (UIMin = "0.0001", UIMax = "1"))
	float RandomDeviation;

	/** Return the entropy of a cell. Intended for debugging only. */
	UFUNCTION(BlueprintPure)
	float GetCellEntropy(int32 CellIndex) const;

	virtual FWFCCellIndex SelectNextCell() override;

protected:
	virtual float CalculateShannonEntropy(const FWFCCell& Cell) const;
};
