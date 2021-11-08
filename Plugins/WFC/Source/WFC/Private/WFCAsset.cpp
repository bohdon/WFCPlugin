// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCAsset.h"

#include "Generators/WFCEntropyGenerator.h"


UWFCAsset::UWFCAsset()
{
	GeneratorClass = UWFCEntropyGenerator::StaticClass();
}
