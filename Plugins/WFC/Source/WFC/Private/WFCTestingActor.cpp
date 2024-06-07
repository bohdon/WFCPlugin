// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTestingActor.h"

#include "WFCGeneratorComponent.h"
#include "WFCRenderingComponent.h"
#include "WFCTileAsset3D.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/WFCModel.h"
#include "LevelInstance/WFCTileLevelInstance.h"


AWFCTestingActor::AWFCTestingActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  bLoadTileLevelsOnFinish(false),
	  bSpawnOnError(true),
	  bSpawnOnCellSelection(false),
	  bLoadTileLevelsOnSelection(false)
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

const FWFCModelAssetTile* AWFCTestingActor::GetAssetTileForCell(int32 CellIndex) const
{
	const UWFCGenerator* Generator = WFCGenerator->GetGenerator();
	if (!Generator || !Generator->IsValidCellIndex(CellIndex))
	{
		return nullptr;
	}

	const FWFCCell& Cell = Generator->GetCell(CellIndex);
	if (!Cell.HasSelection())
	{
		return nullptr;
	}

	const FWFCTileId SelectedTileId = Cell.GetSelectedTileId();
	if (!Generator->IsValidTileId(SelectedTileId))
	{
		return nullptr;
	}

	return Generator->GetModel()->GetTile<FWFCModelAssetTile>(SelectedTileId);
}

FTransform AWFCTestingActor::GetCellTransform(int32 CellIndex, int32 Rotation) const
{
	FTransform ActorTransform = GetActorTransform();

	if (!WFCGenerator->IsInitialized())
	{
		return ActorTransform;
	}

	const UWFCGrid* Grid = WFCGenerator->GetGrid();
	if (!Grid || !Grid->IsValidCellIndex(CellIndex))
	{
		return ActorTransform;
	}

	return Grid->GetCellWorldTransform(CellIndex, Rotation) * ActorTransform;
}

void AWFCTestingActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		WFCGenerator->OnCellSelectedEvent.AddUObject(this, &AWFCTestingActor::OnCellSelected);
		WFCGenerator->OnFinishedEvent.AddUObject(this, &AWFCTestingActor::OnGeneratorFinished);
	}
}

void AWFCTestingActor::OnCellSelected(int32 CellIndex)
{
	if (bSpawnOnCellSelection)
	{
		SpawnActorForCell(CellIndex, bLoadTileLevelsOnSelection);
	}
}

void AWFCTestingActor::OnGeneratorFinished(EWFCGeneratorState State)
{
	if (State == EWFCGeneratorState::Finished ||
		(State == EWFCGeneratorState::Error && bSpawnOnError))
	{
		// spawn tile actors (if they're not already)
		SpawnActorsForAllCells();

		if (bLoadTileLevelsOnFinish)
		{
			// ensure all tile levels are loaded
			LoadAllTileActorLevels();
		}
	}
}

AActor* AWFCTestingActor::SpawnActorForCell(int32 CellIndex, bool bAutoLoad)
{
	const FWFCModelAssetTile* AssetTile = GetAssetTileForCell(CellIndex);
	if (!AssetTile)
	{
		return nullptr;
	}

	const UWFCTileAsset* TileAsset = Cast<UWFCTileAsset>(AssetTile->TileAsset.Get());
	if (!TileAsset)
	{
		return nullptr;
	}

	TWeakObjectPtr<AActor> AlreadySpawnedActor = SpawnedTileActors.FindRef(CellIndex);
	if (AlreadySpawnedActor.IsValid())
	{
		// actor already spawned for this cell
		return AlreadySpawnedActor.Get();
	}

	const TSubclassOf<AActor> ActorClass = TileAsset->GetTileDefActorClass(AssetTile->TileDefIndex);
	if (!ActorClass)
	{
		return nullptr;
	}

	const FTransform Transform = GetCellTransform(CellIndex, AssetTile->Rotation);

	// TODO: use generic tile instance objects that can do whatever they want, spawn actors, load level instances, etc
	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags = RF_Transient;
	AActor* TileActor = GetWorld()->SpawnActor<AActor>(ActorClass, Transform, SpawnParams);
	if (!TileActor)
	{
		return nullptr;
	}

	SpawnedTileActors.Add(CellIndex, TileActor);

	// handle level instance tiles
	if (AWFCTileLevelInstance* LevelInstanceTile = Cast<AWFCTileLevelInstance>(TileActor))
	{
		LevelInstanceTile->bAutoLoad = bAutoLoad;
		LevelInstanceTile->SetGeneratorComp(WFCGenerator);
		LevelInstanceTile->SetTileAndCell(AssetTile, CellIndex);
	}

	return TileActor;
}

AActor* AWFCTestingActor::GetActorForCell(int32 CellIndex) const
{
	return SpawnedTileActors.FindRef(CellIndex).Get();
}

void AWFCTestingActor::SpawnActorsForAllCells()
{
	const UWFCGenerator* Generator = WFCGenerator->GetGenerator();
	if (!Generator)
	{
		return;
	}

	for (int32 CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
	{
		SpawnActorForCell(CellIndex, false);
	}
}

void AWFCTestingActor::LoadAllTileActorLevels()
{
	for (const auto& Elem : SpawnedTileActors)
	{
		TWeakObjectPtr<AActor> TileActor = Elem.Value;
		if (AWFCTileLevelInstance* LevelInstanceTile = Cast<AWFCTileLevelInstance>(TileActor.Get()))
		{
			LevelInstanceTile->LoadLevel();
		}
	}
}

void AWFCTestingActor::DestroyAllSpawnedActors()
{
	for (auto& Elem : SpawnedTileActors)
	{
		if (Elem.Value.IsValid())
		{
			Elem.Value->Destroy();
		}
	}
	SpawnedTileActors.Reset();
}
