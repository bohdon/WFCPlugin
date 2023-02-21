// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCGeneratorComponent.h"

#include "WFCAsset.h"
#include "WFCModule.h"
#include "WFCTileSet.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"


UWFCGeneratorComponent::UWFCGeneratorComponent()
	: StepLimit(100000),
	  bAutoRun(true),
	  EditorGridColor(FLinearColor::White)
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

bool UWFCGeneratorComponent::Initialize()
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

	if (WFCAsset->TileSet->TileAssets.IsEmpty())
	{
		// TODO: move to asset verify, don't warn here
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

	SCOPE_LOG_TIME(TEXT("UWFCGeneratorComponent::InitializeGenerator"), nullptr);

	// create and initialize the model and generate all tiles
	Model = NewObject<UWFCModel>(this, WFCAsset->ModelClass);
	check(Model != nullptr);
	Model->Initialize(WFCAsset->TileSet);
	{
		SCOPE_LOG_TIME(*FString::Printf(TEXT("%s::GenerateTiles"), *Model->GetClass()->GetName()), nullptr);
		Model->GenerateTiles();
	}

	// create and initialize the generator
	Generator = NewObject<UWFCGenerator>(this, WFCAsset->GeneratorClass);
	check(Generator != nullptr);
	Generator->OnCellSelected.AddUObject(this, &UWFCGeneratorComponent::OnCellSelected);
	Generator->OnStateChanged.AddUObject(this, &UWFCGeneratorComponent::OnStateChanged);

	FWFCGeneratorConfig Config;
	Config.Model = Model;
	Config.GridConfig = WFCAsset->GridConfig;
	Config.ConstraintClasses = WFCAsset->ConstraintClasses;
	Config.CellSelectorClasses = WFCAsset->CellSelectorClasses;

	{
		SCOPE_LOG_TIME(*FString::Printf(TEXT("%s::Initialize"), *Generator->GetClass()->GetName()), nullptr);
		Generator->Initialize(Config);
	}

	return true;
}

bool UWFCGeneratorComponent::IsInitialized() const
{
	return Generator && Generator->IsInitialized();
}

void UWFCGeneratorComponent::ResetGenerator()
{
	if (Generator)
	{
		Generator->Reset();
	}
}

void UWFCGeneratorComponent::Run()
{
	if (!IsInitialized())
	{
		Initialize();
	}

	if (IsInitialized())
	{
		Generator->Run(StepLimit);
	}
}

void UWFCGeneratorComponent::Next(bool bBreakAfterConstraints)
{
	if (!IsInitialized())
	{
		Initialize();
	}
	else
	{
		Generator->Next(bBreakAfterConstraints);
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
	OnCellSelectedEvent.Broadcast(CellIndex);
	OnCellSelectedEvent_BP.Broadcast(CellIndex);
}

void UWFCGeneratorComponent::OnStateChanged(EWFCGeneratorState State)
{
	OnStateChangedEvent_BP.Broadcast(State);

	if (State == EWFCGeneratorState::Finished || State == EWFCGeneratorState::Error)
	{
		OnFinishedEvent_BP.Broadcast(State == EWFCGeneratorState::Finished);
		OnFinishedEvent.Broadcast(State);
	}
}
