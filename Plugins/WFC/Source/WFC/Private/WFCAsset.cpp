﻿// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCAsset.h"

#include "Core/Generators/WFCEntropyGenerator.h"


UWFCAsset::UWFCAsset()
{
	GeneratorClass = UWFCEntropyGenerator::StaticClass();
}