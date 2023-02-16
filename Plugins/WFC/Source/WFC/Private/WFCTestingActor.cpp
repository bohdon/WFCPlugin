// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTestingActor.h"

#include "WFCGeneratorComponent.h"
#include "WFCRenderingComponent.h"


AWFCTestingActor::AWFCTestingActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WFCGenerator = CreateDefaultSubobject<UWFCGeneratorComponent>(TEXT("WFCGenerator"));
	RootComponent = WFCGenerator;

#if WITH_EDITORONLY_DATA
	WFCRendering = CreateEditorOnlyDefaultSubobject<UWFCRenderingComponent>(TEXT("WFCRendering"));
	if (WFCRendering)
	{
		WFCRendering->SetupAttachment(RootComponent);
	}
#endif
}
