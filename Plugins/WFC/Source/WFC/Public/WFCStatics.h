// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WFCStatics.generated.h"


class UWFCAsset;
class UWFCGenerator;
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
	UFUNCTION(BlueprintPure, Category = "WFC")
	static FVector SnapToNonUniformGrid(FVector Location, FVector GridSize);

	/** Return the interval in seconds between steps when interactively running a WFC generator */
	UFUNCTION(BlueprintPure, Category = "WFC")
	static float GetDebugStepInterval();

	/** Return a random color seeded by an integer. */
	UFUNCTION(BlueprintPure, Meta = (AdvancedDisplay = "1"), Category = "WFC")
	static FLinearColor GetRandomDebugColor(int32 Seed, float Saturation = 0.9f, float Value = 0.7f);

	/** Create and initialize a WFC generator from a WFC Asset. */
	UFUNCTION(BlueprintCallable)
	static UWFCGenerator* CreateWFCGenerator(UObject* Outer, UWFCAsset* WFCAsset);
};
