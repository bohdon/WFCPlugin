// Copyright Bohdon Sayre. All Rights Reserved.


#include "LevelInstance/DynamicLevelInstance.h"

#include "Engine/LevelStreamingDynamic.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "LevelInstance/LevelInstanceComponent.h"
#include "LevelInstance/LevelInstanceLevelStreaming.h"


ADynamicLevelInstance::ADynamicLevelInstance()
	: LevelInstanceActorImpl(this),
	  bAutoLoad(true)
{
	RootComponent = CreateDefaultSubobject<ULevelInstanceComponent>(TEXT("Root"));
	RootComponent->Mobility = EComponentMobility::Static;
}

void ADynamicLevelInstance::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoLoad && !LevelAsset.IsNull())
	{
		LoadLevel();
	}
}

void ADynamicLevelInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		UnloadLevel();
	}
}

void ADynamicLevelInstance::LoadLevel()
{
	if (LevelAsset.IsNull())
	{
		return;
	}

	if (!LevelInstanceGuid.IsValid())
	{
		// ensure a valid guid exists
		LevelInstanceGuid = FGuid::NewGuid();
	}

	LevelInstanceActorImpl.RegisterLevelInstance();
}

void ADynamicLevelInstance::UnloadLevel()
{
	LevelInstanceActorImpl.UnregisterLevelInstance();
}

void ADynamicLevelInstance::SetLevelAsset(TSoftObjectPtr<UWorld> NewLevel)
{
	if (LevelAsset == NewLevel)
	{
		return;
	}

	if (IsLoaded())
	{
		UnloadLevel();
	}

	LevelAsset = NewLevel;

	// reset guid so a new one will be used on level load
	LevelInstanceGuid.Invalidate();

	if (bAutoLoad)
	{
		LoadLevel();
	}
}

const FLevelInstanceID& ADynamicLevelInstance::GetLevelInstanceID() const
{
	return LevelInstanceActorImpl.GetLevelInstanceID();
}

bool ADynamicLevelInstance::HasValidLevelInstanceID() const
{
	return LevelInstanceActorImpl.HasValidLevelInstanceID();
}

const FGuid& ADynamicLevelInstance::GetLevelInstanceGuid() const
{
	return LevelInstanceGuid;
}

const TSoftObjectPtr<UWorld>& ADynamicLevelInstance::GetWorldAsset() const
{
	return LevelAsset;
}

void ADynamicLevelInstance::OnLevelInstanceLoaded()
{
	LevelInstanceActorImpl.OnLevelInstanceLoaded();

	if (ULevelStreamingLevelInstance* LevelStreaming = GetLevelStreaming())
	{
		LevelStreaming->OnLevelShown.AddUniqueDynamic(this, &ADynamicLevelInstance::OnLevelShown);
		LevelStreaming->OnLevelHidden.AddUniqueDynamic(this, &ADynamicLevelInstance::OnLevelHidden);
		LevelStreaming->OnLevelUnloaded.AddUniqueDynamic(this, &ADynamicLevelInstance::OnLevelUnloaded);

		if (LevelStreaming->IsLevelVisible())
		{
			OnLevelShown();
		}
		else
		{
		}
	}
}

bool ADynamicLevelInstance::IsLoadingEnabled() const
{
	return LevelInstanceActorImpl.IsLoadingEnabled();
}

#if WITH_EDITOR
bool ADynamicLevelInstance::SetWorldAsset(TSoftObjectPtr<UWorld> InWorldAsset)
{
	// no editor functionality for dynamic level instances
	return false;
}

ULevelInstanceComponent* ADynamicLevelInstance::GetLevelInstanceComponent() const
{
	return Cast<ULevelInstanceComponent>(RootComponent);
}

ELevelInstanceRuntimeBehavior ADynamicLevelInstance::GetDesiredRuntimeBehavior() const
{
	return ELevelInstanceRuntimeBehavior::None;
}

ELevelInstanceRuntimeBehavior ADynamicLevelInstance::GetDefaultRuntimeBehavior() const
{
	// dynamic level instance actors should never be saved, and don't support world partition
	return ELevelInstanceRuntimeBehavior::None;
}
#endif

void ADynamicLevelInstance::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();

	UnloadLevel();
}

void ADynamicLevelInstance::OnLevelShown()
{
}

void ADynamicLevelInstance::OnLevelHidden()
{
}

void ADynamicLevelInstance::OnLevelUnloaded()
{
}
