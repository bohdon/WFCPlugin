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
	virtual FWFCCellIndex SelectNextCellIndex() override;
	virtual FWFCTileId SelectNextTileForCell(FWFCCellIndex Index) override;
};
