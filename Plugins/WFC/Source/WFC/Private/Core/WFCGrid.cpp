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
