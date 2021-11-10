// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCConstraint.h"


void UWFCConstraint::Initialize(UWFCGenerator* InGenerator)
{
	check(InGenerator != nullptr);

	Generator = InGenerator;
}

void UWFCConstraint::NotifyCellChanged(FWFCCellIndex CellIndex)
{
}

bool UWFCConstraint::Next()
{
	return false;
}
