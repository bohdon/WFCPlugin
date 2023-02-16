// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCAsset.h"

#include "Core/WFCCellSelector.h"
#include "Core/WFCGenerator.h"


UWFCAsset::UWFCAsset()
{
	GeneratorClass = UWFCGenerator::StaticClass();
	CellSelectorClasses = {UWFCRandomCellSelector::StaticClass()};
}
