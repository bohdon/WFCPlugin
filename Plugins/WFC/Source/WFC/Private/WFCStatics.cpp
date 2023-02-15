// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCStatics.h"

TAutoConsoleVariable<float> CVarWFCDebugStepInterval(
	TEXT("wfc.DebugStepInterval"), 0.05f,
	TEXT("Interval between debug steps when interactively running a WFC generator."));


FVector UWFCStatics::SnapToNonUniformGrid(FVector Location, FVector GridSize)
{
	return FVector(FMath::GridSnap(Location.X, GridSize.X),
	               FMath::GridSnap(Location.Y, GridSize.Y),
	               FMath::GridSnap(Location.Z, GridSize.Z));
}

float UWFCStatics::GetDebugStepInterval()
{
	return CVarWFCDebugStepInterval.GetValueOnAnyThread();
}
