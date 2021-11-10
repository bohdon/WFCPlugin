// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCGeneratorComponent.h"

#include "WFCAsset.h"
#include "WFCModule.h"
#include "WFCTileAsset.h"
#include "WFCTileSetAsset.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"


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
	WFCAsset->TileSet->GenerateTiles(Model->Tiles);

	// create and initialize the generator
	Generator = NewObject<UWFCGenerator>(this, WFCAsset->GeneratorClass);
	Generator->OnCellSelected.AddUObject(this, &UWFCGeneratorComponent::OnCellSelected);

	FWFCGeneratorConfig Config;
	Config.Grid = WFCAsset->Grid;
	Config.NumTiles = Model->GetNumTiles();
	Config.ConstraintClasses = WFCAsset->ConstraintClasses;

	Generator->Initialize(Config);

	// configure constraints
	// TODO: move into constraint?
	AddAdjacencyMappings();

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

EWFCGeneratorState UWFCGeneratorComponent::GetState() const
{
	return Generator ? Generator->State : EWFCGeneratorState::None;
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

void UWFCGeneratorComponent::AddAdjacencyMappings()
{
	if (!Generator)
	{
		return;
	}

	UWFCAdjacencyConstraint* AdjacencyConstraint = Generator->GetConstraint<UWFCAdjacencyConstraint>();
	if (!AdjacencyConstraint)
	{
		return;
	}

	const UWFCGrid* Grid = Generator->GetGrid();
	check(Grid != nullptr);

	// TODO: don't check adjacency for B -> A if A -> B has already been checked, change AddAdjacentTileMapping to include both

	// iterate over all tiles, comparing to all other tiles...
	for (FWFCTileId TileIdA = 0; TileIdA < Model->GetNumTiles(); ++TileIdA)
	{
		FWFCTile TileA = Model->GetTile(TileIdA);

		for (FWFCTileId TileIdB = 0; TileIdB < Model->GetNumTiles(); ++TileIdB)
		{
			FWFCTile TileB = Model->GetTile(TileIdB);

			UWFCTile2dAsset* Tile2dA = Cast<UWFCTile2dAsset>(TileA.Object.Get());
			UWFCTile2dAsset* Tile2dB = Cast<UWFCTile2dAsset>(TileB.Object.Get());
			if (Tile2dA && Tile2dB)
			{
				// for each direction, and check if socket type matches opposite direction on the other tile
				for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
				{
					// adjacency mappings represent 'incoming' directions,
					// so the adjacency mappings for A here represent the direction B -> A
					const int32 AInvRotation = (4 - TileA.Rotation) % 4;
					const int32 BInvRotation = (4 - TileB.Rotation) % 4;
					FWFCGridDirection AEdgeDirection = Grid->GetRotatedDirection(Grid->GetOppositeDirection(Direction), AInvRotation);
					FWFCGridDirection BEdgeDirection = Grid->GetRotatedDirection(Direction, BInvRotation);
					const int32 SocketTypeA = Tile2dA->EdgeSocketTypes[static_cast<EWFCTile2dEdge>(AEdgeDirection)];
					const int32 SocketTypeB = Tile2dB->EdgeSocketTypes[static_cast<EWFCTile2dEdge>(BEdgeDirection)];

					if (SocketTypeA == SocketTypeB)
					{
						AdjacencyConstraint->AddAdjacentTileMapping(TileIdA, Direction, TileIdB);
					}
				}
			}
		}
	}
}

void UWFCGeneratorComponent::OnCellSelected(int32 CellIndex)
{
	OnCellSelectedEvent_BP.Broadcast(CellIndex);
}
