// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCConstraint.h"

#include "Core/WFCGenerator.h"


void UWFCConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Generator = InGenerator;
	check(Generator != nullptr);

	Grid = Generator->GetGrid();
	check(Grid != nullptr);

	Model = Generator->GetModel();
	check(Model != nullptr);
}

void UWFCConstraint::Reset()
{
}

void UWFCConstraint::NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection)
{
}

void UWFCConstraint::NotifyCellBan(FWFCCellIndex CellIndex, FWFCTileId BannedTileId)
{
}

bool UWFCConstraint::Next()
{
	return false;
}

void UWFCConstraint::LogDebugInfo() const
{
}

UWFCConstraintSnapshot* UWFCConstraint::CreateSnapshot(UObject* Outer) const
{
	return nullptr;
}

void UWFCConstraint::ApplySnapshot(const UWFCConstraintSnapshot* Snapshot)
{
}
