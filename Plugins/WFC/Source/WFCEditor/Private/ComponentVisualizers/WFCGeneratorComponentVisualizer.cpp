// Copyright Bohdon Sayre. All Rights Reserved.


#include "ComponentVisualizers/WFCGeneratorComponentVisualizer.h"

#include "WFCAsset.h"
#include "WFCGeneratorComponent.h"

IMPLEMENT_HIT_PROXY(HWFCGeneratorVisProxy, HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HWFCCellProxy, HWFCGeneratorVisProxy);


FWFCGeneratorComponentVisualizer::FWFCGeneratorComponentVisualizer()
{
}

FWFCGeneratorComponentVisualizer::~FWFCGeneratorComponentVisualizer()
{
}

void FWFCGeneratorComponentVisualizer::OnRegister()
{
	FComponentVisualizer::OnRegister();
}

void FWFCGeneratorComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
                                                         FPrimitiveDrawInterface* PDI)
{
	const UWFCGeneratorComponent* GeneratorComponent = Cast<UWFCGeneratorComponent>(Component);
	if (!GeneratorComponent)
	{
		return;
	}

	const UWFCAsset* WFCAsset = GeneratorComponent->WFCAsset;
	if (!WFCAsset)
	{
		return;
	}
}
