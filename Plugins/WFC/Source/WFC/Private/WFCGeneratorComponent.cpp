// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCGeneratorComponent.h"

#include "WFCAsset.h"
#include "WFCModule.h"
#include "WFCTileSet.h"
#include "WFCTileSetGenerator.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"


UWFCGeneratorComponent::UWFCGeneratorComponent()
	: StepLimit(100000),
	  bAutoRun(true)
{
}

void UWFCGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRun)
	{
		Run();
	}
}

bool UWFCGeneratorComponent::InitializeGenerator()
{
	if (!WFCAsset)
	{
		UE_LOG(LogWFC, Warning, TEXT("No WFCAsset was specified: %s"), *GetNameSafe(GetOwner()));
		return false;
	}

	if (!WFCAsset->TileSet)
	{
		UE_LOG(LogWFC, Warning, TEXT("No TileSet was specified: %s"), *WFCAsset->GetName());
		return false;
	}

	if (WFCAsset->TileSet->TileAssets.Num() == 0)
	{
		UE_LOG(LogWFC, Warning, TEXT("TileSet has no tiles: %s"), *WFCAsset->TileSet->GetName());
		return false;
	}

	if (!WFCAsset->ModelClass)
	{
		UE_LOG(LogWFC, Warning, TEXT("No ModelClass was specified: %s"), *WFCAsset->GetName());
		return false;
	}

	if (!WFCAsset->GeneratorClass)
	{
		UE_LOG(LogWFC, Warning, TEXT("No GeneratorClass was specified: %s"), *WFCAsset->GetName());
		return false;
	}

	// create and initialize the model and generate all tiles
	Model = NewObject<UWFCModel>(this, WFCAsset->ModelClass);
	check(Model != nullptr);
	Model->Initialize(WFCAsset->TileSet);
	Model->GenerateTiles();

	// create and initialize the generator
	Generator = NewObject<UWFCGenerator>(this, WFCAsset->GeneratorClass);
	check(Generator != nullptr);
	Generator->OnCellSelected.AddUObject(this, &UWFCGeneratorComponent::OnCellSelected);

	FWFCGeneratorConfig Config;
	Config.Model = Model;
	Config.GridConfig = WFCAsset->GridConfig;
	Config.ConstraintClasses = WFCAsset->ConstraintClasses;

	Generator->Initialize(Config);

	Model->ConfigureGenerator(Generator);

	return true;
}

void UWFCGeneratorComponent::Run()
{
	InitializeGenerator();

	if (Generator)
	{
		Generator->Run(StepLimit);
	}
}

const UWFCGrid* UWFCGeneratorComponent::GetGrid() const
{
	return Generator ? Generator->GetGrid() : nullptr;
}

EWFCGeneratorState UWFCGeneratorComponent::GetState() const
{
	return Generator ? Generator->State : EWFCGeneratorState::None;
}

void UWFCGeneratorComponent::GetSelectedTileId(int32 CellIndex, bool& bSuccess, int32& TileId) const
{
	TileId = INDEX_NONE;
	bSuccess = false;
	if (Generator)
	{
		const FWFCCell& Cell = Generator->GetCell(CellIndex);
		if (Cell.HasSelection())
		{
			TileId = Cell.GetSelectedTileId();
			bSuccess = true;
		}
	}
}

void UWFCGeneratorComponent::GetSelectedTileIds(TArray<int32>& TileIds) const
{
	if (Generator)
	{
		Generator->GetSelectedTileIds(TileIds);
	}
	else
	{
		TileIds.Reset();
	}
}

void UWFCGeneratorComponent::OnCellSelected(int32 CellIndex)
{
	OnCellSelectedEvent_BP.Broadcast(CellIndex);
}
