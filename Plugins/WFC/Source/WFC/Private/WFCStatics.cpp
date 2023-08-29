// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCStatics.h"

#include "WFCAsset.h"
#include "WFCModule.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"


TAutoConsoleVariable<float> CVarWFCDebugStepInterval(
	TEXT("wfc.DebugStepInterval"), 0.05f,
	TEXT("Interval between debug steps when interactively running a WFC generator."));


FVector UWFCStatics::SnapToNonUniformGrid(FVector Location, FVector GridSize)
{
	return FVector(FMath::GridSnap(Location.X, GridSize.X),
	               FMath::GridSnap(Location.Y, GridSize.Y),
	               FMath::GridSnap(Location.Z, GridSize.Z));
}

float UWFCStatics::GetDebugStepInterval()
{
	return CVarWFCDebugStepInterval.GetValueOnAnyThread();
}

FLinearColor UWFCStatics::GetRandomDebugColor(int32 Seed, float Saturation, float Value)
{
	const FRandomStream Stream(Seed);
	// default random hue is not great, spread it out a bit over a wider range then modulo
	const float RandHue = FMath::Fmod(Stream.FRand() * 5000.f, 360.f);
	// random again for the value
	const float RandValue = Stream.FRandRange(Value * 0.5f, Value);
	const FLinearColor HSV(RandHue, Saturation, RandValue, 1.f);
	return HSV.HSVToLinearRGB();
}

UWFCGenerator* UWFCStatics::CreateWFCGenerator(UObject* Outer, UWFCAsset* WFCAsset)
{
	if (!WFCAsset)
	{
		return nullptr;
	}

	if (!WFCAsset->ModelClass)
	{
		UE_LOG(LogWFC, Warning, TEXT("No ModelClass was specified: %s"), *WFCAsset->GetName());
		return nullptr;
	}

	if (!WFCAsset->GeneratorClass)
	{
		UE_LOG(LogWFC, Warning, TEXT("No GeneratorClass was specified: %s"), *WFCAsset->GetName());
		return nullptr;
	}

	// create and initialize the model, but don't generate tiles, that will be run by the generator
	UWFCModel* Model = NewObject<UWFCModel>(Outer, WFCAsset->ModelClass);
	check(Model != nullptr);
	Model->Initialize(WFCAsset);

	// create and configure the generator
	UWFCGenerator* Generator = NewObject<UWFCGenerator>(Outer, WFCAsset->GeneratorClass);

	FWFCGeneratorConfig Config;
	Config.Model = Model;
	Config.GridConfig = WFCAsset->GridConfig;
	Config.ConstraintClasses = WFCAsset->ConstraintClasses;
	Config.CellSelectorClasses = WFCAsset->CellSelectorClasses;

	Generator->Configure(Config);

	return Generator;
}
