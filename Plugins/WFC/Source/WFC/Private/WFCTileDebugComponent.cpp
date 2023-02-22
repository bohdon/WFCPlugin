// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileDebugComponent.h"

#include "WFCAsset.h"
#include "WFCAssetModel.h"
#include "WFCGeneratorComponent.h"
#include "WFCRenderingComponent.h"
#include "WFCTileActorInterface.h"
#include "WFCTileAsset3D.h"
#include "WFCTileSet.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"
#include "Core/Constraints/WFCArcConsistencyConstraint.h"
#include "Core/Grids/WFCGrid3D.h"


UWFCTileDebugComponent::UWFCTileDebugComponent()
	: bAutoSpawn(true),
	  AutoSpawnDelay(0.5f),
	  Spacing(0.25f)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWFCTileDebugComponent::SetTileId(int32 NewTileId)
{
	const int32 NewClampedTileId = FMath::Clamp(NewTileId, 0, GetMaxTileId());
	if (NewClampedTileId != TileId)
	{
		TileId = NewTileId;
		OnTileIdChanged();
	}
}

int32 UWFCTileDebugComponent::GetMaxTileId() const
{
	if (const UWFCGeneratorComponent* GeneratorComp = GetGeneratorComp())
	{
		if (GeneratorComp->IsInitialized())
		{
			return GeneratorComp->GetGenerator()->GetNumTiles() - 1;
		}
		if (GeneratorComp->WFCAsset && GeneratorComp->WFCAsset->TileSet)
		{
			int32 NumTiles = 0;
			for (const UWFCTileAsset* TileAsset : GeneratorComp->WFCAsset->TileSet->TileAssets)
			{
				if (TileAsset)
				{
					NumTiles += TileAsset->GetNumTileDefs() * TileAsset->GetNumRotations();
				}
			}
			return NumTiles;
		}
	}
	return 0;
}

FVector UWFCTileDebugComponent::GetCellSize() const
{
	if (const UWFCGeneratorComponent* GeneratorComp = GetGeneratorComp())
	{
		if (GeneratorComp->IsInitialized())
		{
			const UWFCGenerator* Generator = GeneratorComp->GetGenerator();
			const UWFCGrid3D* Grid3D = Generator ? Generator->GetGrid<UWFCGrid3D>() : nullptr;
			if (Grid3D)
			{
				return Grid3D->CellSize;
			}
		}
		else if (GeneratorComp->WFCAsset && GeneratorComp->WFCAsset->GridConfig)
		{
			if (const UWFCGrid3DConfig* Grid3DConfig = Cast<UWFCGrid3DConfig>(GeneratorComp->WFCAsset->GridConfig))
			{
				return Grid3DConfig->CellSize;
			}
		}
	}
	return FVector::OneVector * 100.f;
}

void UWFCTileDebugComponent::PostInitProperties()
{
	Super::PostInitProperties();

	TileId = FMath::Clamp(TileId, 0, GetMaxTileId());
}

UWFCGeneratorComponent* UWFCTileDebugComponent::GetGeneratorComp() const
{
	return GeneratorActor ? GeneratorActor->FindComponentByClass<UWFCGeneratorComponent>() : nullptr;
}

UWFCGenerator* UWFCTileDebugComponent::GetGenerator() const
{
	if (const UWFCGeneratorComponent* GeneratorComp = GetGeneratorComp())
	{
		return GeneratorComp->GetGenerator();
	}
	return nullptr;
}

const UWFCAssetModel* UWFCTileDebugComponent::GetAssetModel() const
{
	if (const UWFCGenerator* Generator = GetGenerator())
	{
		return Generator->GetModel<UWFCAssetModel>();
	}
	return nullptr;
}

FString UWFCTileDebugComponent::GetTileDebugString() const
{
	if (const UWFCAssetModel* Model = GetAssetModel())
	{
		return Model->GetTileDebugString(TileId);
	}
	return FString();
}

void UWFCTileDebugComponent::GetDebugTileInstances(TArray<FWFCTileDebugInstance>& OutTileInstances) const
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	const FVector ActorLocation = GetOwner()->GetActorLocation();

	const UWFCGenerator* Generator = GetGenerator();
	if (!Generator)
	{
		return;
	}

	const UWFCAssetModel* AssetModel = Generator->GetModel<UWFCAssetModel>();
	const UWFCGrid* Grid = Generator->GetGrid();
	if (!AssetModel || !Grid)
	{
		return;
	}

	const FWFCModelAssetTile* AssetTile = AssetModel->GetTile<FWFCModelAssetTile>(TileId);
	if (!AssetTile)
	{
		return;
	}

	// add the current tile being debugged
	OutTileInstances.Emplace(ActorLocation, TileId);

	// add all arc consistent tiles in each direction
	const UWFCArcConsistencyConstraint* Arc = Generator->GetConstraint<UWFCArcConsistencyConstraint>();
	if (Arc)
	{
		for (FWFCGridDirection Direction = 0; Direction < Grid->GetNumDirections(); ++Direction)
		{
			const TArray<FWFCTileId>& AllowedTileIds = Arc->GetAllowedTileIds(AssetTile->Id, Direction);
			for (int32 Idx = 0; Idx < AllowedTileIds.Num(); ++Idx)
			{
				const FVector DirectionVector = FVector(Grid->GetDirectionVector(Direction));
				const FVector Location = ActorLocation + DirectionVector * CachedCellSize * (1.f + Spacing) * (Idx + 1);
				OutTileInstances.Emplace(Location, AllowedTileIds[Idx]);
			}
		}
	}
}

void UWFCTileDebugComponent::SpawnTileActors()
{
	ClearTileActors();

	const UWFCAssetModel* AssetModel = GetAssetModel();
	if (!AssetModel)
	{
		return;
	}

	TArray<FWFCTileDebugInstance> TileInstances;
	CachedCellSize = GetCellSize();
	GetDebugTileInstances(TileInstances);

	for (const FWFCTileDebugInstance& TileInstance : TileInstances)
	{
		const FWFCModelAssetTile* AssetTile = AssetModel->GetTile<FWFCModelAssetTile>(TileInstance.TileId);
		SpawnTileActor(AssetTile, TileInstance.Location);
	}
}

void UWFCTileDebugComponent::ClearTileActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Reset();
}

void UWFCTileDebugComponent::OnTileIdChanged_Implementation()
{
	MarkRenderStateDirty();

	if (bAutoSpawn)
	{
		GetWorld()->GetTimerManager().SetTimer(AutoSpawnTimer, this, &UWFCTileDebugComponent::SpawnTileActors, 0.5f);
	}
}

#if UE_ENABLE_DEBUG_DRAWING
FDebugRenderSceneProxy* UWFCTileDebugComponent::CreateDebugSceneProxy()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	const UWFCAssetModel* AssetModel = GetAssetModel();
	if (!AssetModel)
	{
		return nullptr;
	}

	FWFCDebugSceneProxy* DebugProxy = new FWFCDebugSceneProxy(this);

	TArray<FWFCTileDebugInstance> TileInstances;
	CachedCellSize = GetCellSize();
	GetDebugTileInstances(TileInstances);
	for (const FWFCTileDebugInstance& TileInstance : TileInstances)
	{
		const FWFCModelAssetTile* AssetTile = AssetModel->GetTile<FWFCModelAssetTile>(TileInstance.TileId);
		AddTileSceneProxy(DebugProxy, AssetTile, TileInstance.Location);
	}

	return DebugProxy;
}

void UWFCTileDebugComponent::AddTileSceneProxy(FDebugRenderSceneProxy* DebugProxy, const FWFCModelAssetTile* AssetTile, FVector Location)
{
	const FBox Box(Location, Location + CachedCellSize);
	DebugProxy->Boxes.Emplace(Box, FColor::White);
	TArray<FString> TextLines;
	TextLines.Add(FString::FromInt(AssetTile->Id));
	TextLines.Add(AssetTile->ToString());
	DebugProxy->Texts.Emplace(FString::Join(TextLines, TEXT("\n")), Box.GetCenter(), FColor::White);
}
#endif

FBoxSphereBounds UWFCTileDebugComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	TArray<FWFCTileDebugInstance> TileInstances;
	GetDebugTileInstances(TileInstances);

	if (TileInstances.IsEmpty())
	{
		return Super::CalcBounds(LocalToWorld);
	}

	TArray<FVector> Locations;
	for (const FWFCTileDebugInstance& TileInstance : TileInstances)
	{
		Locations.Add(TileInstance.Location);
	}

	const FBoxSphereBounds NewBounds(Locations);
	return NewBounds;
}

#if WITH_EDITOR
void UWFCTileDebugComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None);
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UWFCTileDebugComponent, TileId))
	{
		TileId = FMath::Clamp(TileId, 0, GetMaxTileId());
		MarkRenderStateDirty();
	}
}
#endif

void UWFCTileDebugComponent::SpawnTileActor(const FWFCModelAssetTile* AssetTile, const FVector& Location)
{
	if (!AssetTile || !GetWorld())
	{
		return;
	}

	const UWFCGenerator* Generator = GetGenerator();
	const UWFCGrid* Grid = Generator ? Generator->GetGrid() : nullptr;
	if (!Grid)
	{
		return;
	}

	const UWFCTileAsset* TileAsset = AssetTile->TileAsset.Get();
	if (!TileAsset)
	{
		return;
	}

	const TSubclassOf<AActor> ActorClass = TileAsset->GetTileDefActorClass(AssetTile->TileDefIndex);
	if (!ActorClass)
	{
		return;
	}

	const FTransform Transform = FTransform(GetComponentRotation(), Location);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* TileActor = GetWorld()->SpawnActor<AActor>(ActorClass, Transform, SpawnParams);
	if (IWFCTileActorInterface* TileActorInterface = Cast<IWFCTileActorInterface>(TileActor))
	{
		TileActorInterface->SetGeneratorComp(GetGeneratorComp());
		TileActorInterface->SetCell(INDEX_NONE);
		TileActorInterface->SetTile(AssetTile);
		TileActorInterface->InitializeTile();
	}

	SpawnedActors.Add(TileActor);
}
