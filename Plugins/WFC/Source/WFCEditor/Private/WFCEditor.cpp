// Copyright Bohdon Sayre. All Rights Reserved.

#include "WFCEditor.h"

#include "ShowFlags.h"
#include "UnrealEdGlobals.h"
#include "WFCGeneratorComponent.h"
#include "ComponentVisualizers/WFCGeneratorComponentVisualizer.h"
#include "Editor/UnrealEdEngine.h"

#define LOCTEXT_NAMESPACE "FWFCEditorModule"

TCustomShowFlag<> ShowZoneGraph(TEXT("WFC"), true /*DefaultEnabled*/, SFG_Developer, LOCTEXT("ShowWFC", "WFC"));

void FWFCEditorModule::StartupModule()
{
	if (GUnrealEd != nullptr)
	{
		const TSharedPtr<FWFCGeneratorComponentVisualizer> Visualizer = MakeShareable(new FWFCGeneratorComponentVisualizer);

		if (Visualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UWFCGeneratorComponent::StaticClass()->GetFName(), Visualizer);
			Visualizer->OnRegister();
		}
	}
}

void FWFCEditorModule::ShutdownModule()
{
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->UnregisterComponentVisualizer(UWFCGeneratorComponent::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWFCEditorModule, WFCEditor)
