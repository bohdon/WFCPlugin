// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCAssetModel.h"
#include "WFCTileModel3D.generated.h"


/**
 * Generates a tile for each asset and possible 3D rotations.
 */
UCLASS()
class WFC_API UWFCTileModel3D : public UWFCAssetModel
{
	GENERATED_BODY()

public:
	UWFCTileModel3D();
};
