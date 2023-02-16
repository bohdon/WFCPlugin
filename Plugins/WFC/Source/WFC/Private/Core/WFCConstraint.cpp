// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCConstraint.h"


void UWFCConstraint::Initialize(UWFCGenerator* InGenerator)
{
	check(InGenerator != nullptr);

	Generator = InGenerator;
}

void UWFCConstraint::Reset()
{
	// implement in subclass
}

void UWFCConstraint::NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection)
{
	// implement in subclass to respond to cell changes
}

bool UWFCConstraint::Next()
{
	return false;
}
