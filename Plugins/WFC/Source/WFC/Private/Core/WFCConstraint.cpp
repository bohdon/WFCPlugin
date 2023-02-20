// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCConstraint.h"

#include "Core/WFCGenerator.h"


void UWFCConstraint::Initialize(UWFCGenerator* InGenerator)
{
	check(InGenerator != nullptr);

	Generator = InGenerator;
	Grid = Generator->GetGrid();
	Model = Generator->GetModel();
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

void UWFCConstraint::LogDebugInfo() const
{
}
