// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCStatics.h"


FVector UWFCStatics::SnapToNonUniformGrid(FVector Location, FVector GridSize)
{
	return FVector(FMath::GridSnap(Location.X, GridSize.X),
	               FMath::GridSnap(Location.Y, GridSize.Y),
	               FMath::GridSnap(Location.Z, GridSize.Z));
}
