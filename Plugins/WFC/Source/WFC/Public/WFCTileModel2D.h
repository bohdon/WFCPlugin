// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCAssetModel.h"
#include "WFCTileModel2D.generated.h"


/**
 * Generates a tile for each asset and possible rotation on a 2D grid.
 * TODO: also generate reflections
 */
UCLASS()
class WFC_API UWFCTileModel2D : public UWFCAssetModel
{
	GENERATED_BODY()

public:
	UWFCTileModel2D();
};
