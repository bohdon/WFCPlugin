// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCGrid.h"


UWFCGridConfig::UWFCGridConfig()
{
	GridClass = UWFCGrid::StaticClass();
}


UWFCGrid::UWFCGrid()
{
}

UWFCGrid* UWFCGrid::NewGrid(UObject* Outer, const UWFCGridConfig* Config)
{
	if (Config && Config->GridClass)
	{
		UWFCGrid* NewGrid = NewObject<UWFCGrid>(Outer, Config->GridClass);
		NewGrid->Initialize(Config);
		return NewGrid;
	}
	return nullptr;
}

void UWFCGrid::Initialize(const UWFCGridConfig* Config)
{
}

FWFCGridDirection UWFCGrid::GetOppositeDirection(FWFCGridDirection Direction) const
{
	unimplemented();
	return INDEX_NONE;
}

FWFCGridDirection UWFCGrid::RotateDirection(FWFCGridDirection Direction, int32 Rotation) const
{
	unimplemented();
	return INDEX_NONE;
}

FWFCGridDirection UWFCGrid::InverseRotateDirection(FWFCGridDirection Direction, int32 Rotation) const
{
	unimplemented();
	return INDEX_NONE;
}

FWFCCellIndex UWFCGrid::GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const
{
	unimplemented();
	return INDEX_NONE;
}

FString UWFCGrid::GetDirectionName(int32 Direction) const
{
	return FString::FromInt(Direction);
}

FString UWFCGrid::GetCellName(int32 CellIndex) const
{
	return FString::FromInt(CellIndex);
}

FVector UWFCGrid::GetCellWorldLocation(int32 CellIndex, bool bCenter) const
{
	return FVector::ZeroVector;
}

FTransform UWFCGrid::GetCellWorldTransform(int32 CellIndex, int32 Rotation) const
{
	return FTransform();
}

FTransform UWFCGrid::GetRotationTransform(int32 Rotation) const
{
	return FTransform();
}

FIntVector UWFCGrid::GetDirectionVector(int32 Direction) const
{
	return FIntVector::ZeroValue;
}
