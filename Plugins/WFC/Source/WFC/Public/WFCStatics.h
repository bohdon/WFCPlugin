// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WFCStatics.generated.h"


/**
 * Static functions for working with WFC tiles, grids, and other features.
 */
UCLASS()
class WFC_API UWFCStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Snap a location to a non-uniform grid */
	UFUNCTION(BlueprintPure)
	static FVector SnapToNonUniformGrid(FVector Location, FVector GridSize);
};
