// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelInstance/LevelInstanceActorImpl.h"
#include "LevelInstance/LevelInstanceInterface.h"
#include "DynamicLevelInstance.generated.h"


/**
 * A Level Instance actor designed to be spawned and given a dynamic world at runtime.
 * Loading and unloading the level can also be controlled independent of the actor's lifecycle.
 */
UCLASS(Transient, NotPlaceable)
class WFC_API ADynamicLevelInstance : public AActor,
                                      public ILevelInstanceInterface
{
	GENERATED_BODY()

private:
	FLevelInstanceActorImpl LevelInstanceActorImpl;

public:
	ADynamicLevelInstance();

	/** Automatically load the level when set. This will also auto load the level on BeginPlay if set during spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn))
	bool bAutoLoad;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	void LoadLevel();

	UFUNCTION(BlueprintCallable)
	void UnloadLevel();

	/** Return the Level this instance is using. */
	UFUNCTION(BlueprintPure)
	TSoftObjectPtr<UWorld> GetLevelAsset() const { return LevelAsset; }

	/** Set the level to load, and optionally start loading it. */
	UFUNCTION(BlueprintCallable)
	void SetLevelAsset(TSoftObjectPtr<UWorld> NewLevel);

	UFUNCTION(BlueprintPure)
	bool IsLevelAssetValid() const { return LevelAsset.IsValid(); }

	/** ILevelInstanceInterface */
	virtual const FLevelInstanceID& GetLevelInstanceID() const override;
	virtual bool HasValidLevelInstanceID() const override;
	virtual const FGuid& GetLevelInstanceGuid() const override;
	virtual const TSoftObjectPtr<UWorld>& GetWorldAsset() const override;
	virtual void OnLevelInstanceLoaded() override;
	virtual bool IsLoadingEnabled() const override;
#if WITH_EDITOR
	virtual bool SetWorldAsset(TSoftObjectPtr<UWorld> InWorldAsset) override;
	virtual ULevelInstanceComponent* GetLevelInstanceComponent() const override;
	virtual ELevelInstanceRuntimeBehavior GetDesiredRuntimeBehavior() const override;
	virtual ELevelInstanceRuntimeBehavior GetDefaultRuntimeBehavior() const override;
#endif

	virtual void PostUnregisterAllComponents() override;

protected:
	/** The guid to use for this level instance. */
	FGuid LevelInstanceGuid;

	/** The level to load. */
	UPROPERTY(Transient, BlueprintReadOnly, Meta = (ExposeOnSpawn))
	TSoftObjectPtr<UWorld> LevelAsset;

	UFUNCTION()
	virtual void OnLevelShown();

	UFUNCTION()
	virtual void OnLevelHidden();

	UFUNCTION()
	virtual void OnLevelUnloaded();
};
