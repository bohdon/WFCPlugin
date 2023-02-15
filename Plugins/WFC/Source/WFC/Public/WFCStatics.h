// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WFCStatics.generated.h"


extern TAutoConsoleVariable<float> CVarWFCDebugStepInterval;


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

	/** Return the interval in seconds between steps when interactively running a WFC generator */
	UFUNCTION(BlueprintPure)
	static float GetDebugStepInterval();
};
