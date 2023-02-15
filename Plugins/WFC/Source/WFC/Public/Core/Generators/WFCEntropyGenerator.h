// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCGenerator.h"
#include "WFCEntropyGenerator.generated.h"


/**
 * A generator that randomly selects cells biasing those with the minimum entropy,
 * e.g. the cells that are already the most constrained, and more likely to cause conflicts later.
 */
UCLASS()
class WFC_API UWFCEntropyGenerator : public UWFCGenerator
{
	GENERATED_BODY()

public:
	/** Return the entropy of a cell. Intended for debugging only. */
	UFUNCTION(BlueprintPure)
	float GetCellEntropy(int32 CellIndex) const;

	virtual FWFCCellIndex SelectNextCellIndex() override;
	virtual FWFCTileId SelectNextTileForCell(FWFCCellIndex Index) override;

	virtual float CalculateShannonEntropy(const FWFCCell& Cell) const;
};
