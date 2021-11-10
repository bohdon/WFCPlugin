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

	if (!WFCAsset->TileGenerator)
	{
		UE_LOG(LogWFC, Warning, TEXT("No TileGenerator was specified: %s"), *WFCAsset->GetName());
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

	const UWFCTileSetGenerator* TileGenerator = WFCAsset->TileGenerator;
	TileGenerator->GenerateTiles(WFCAsset->TileSet, Model->Tiles);

	// create and initialize the generator
	Generator = NewObject<UWFCGenerator>(this, WFCAsset->GeneratorClass);
	check(Generator != nullptr);
	Generator->OnCellSelected.AddUObject(this, &UWFCGeneratorComponent::OnCellSelected);

	FWFCGeneratorConfig Config;
	Config.NumTiles = Model->GetNumTiles();
	Config.GridConfig = WFCAsset->GridConfig;
	Config.ConstraintClasses = WFCAsset->ConstraintClasses;

	Generator->Initialize(Config);

	TileGenerator->ConfigureGeneratorForTiles(WFCAsset->TileSet, Model, Generator);

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

void UWFCGeneratorComponent::GetSelectedTile(int32 CellIndex, bool& bSuccess, FWFCTile& Tile) const
{
	Tile = FWFCTile();
	bSuccess = false;
	if (Generator)
	{
		const FWFCCell& Cell = Generator->GetCell(CellIndex);
		if (Cell.HasSelection())
		{
			const FWFCTileId TileId = Cell.GetSelectedTileId();
			Tile = Model->GetTile(TileId);
			bSuccess = true;
		}
	}
}

void UWFCGeneratorComponent::GetSelectedTiles(TArray<FWFCTile>& OutTiles) const
{
	if (Generator)
	{
		TArray<int32> TileIds;
		Generator->GetSelectedTileIds(TileIds);

		OutTiles.SetNum(TileIds.Num());
		for (int32 Idx = 0; Idx < TileIds.Num(); ++Idx)
		{
			OutTiles[Idx] = Model->GetTile(TileIds[Idx]);
		}
	}
	else
	{
		OutTiles.Reset();
	}
}

void UWFCGeneratorComponent::OnCellSelected(int32 CellIndex)
{
	OnCellSelectedEvent_BP.Broadcast(CellIndex);
}
