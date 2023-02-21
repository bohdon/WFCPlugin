// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugDrawComponent.h"
#include "WFCTileDebugComponent.generated.h"

class UWFCAssetModel;
class UWFCGeneratorComponent;
struct FWFCModelAssetTile;
class UWFCGenerator;


USTRUCT(BlueprintType)
struct FWFCTileDebugInstance
{
	GENERATED_BODY()

	FWFCTileDebugInstance()
		: TileId(INDEX_NONE)
	{
	}

	FWFCTileDebugInstance(const FVector& InLocation, const int32& InTileId)
		: Location(InLocation),
		  TileId(InTileId)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TileId;
};


/**
 * Allows debugging individual tiles and their relationship to constraints in the generator.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WFC_API UWFCTileDebugComponent : public UDebugDrawComponent
{
	GENERATED_BODY()

public:
	UWFCTileDebugComponent();

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	AActor* GeneratorActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin="0"))
	int32 TileId;

	UFUNCTION(BlueprintPure)
	int32 GetMaxTileId() const;

	UFUNCTION(BlueprintPure)
	FVector GetCellSize() const;

	virtual void PostInitProperties() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UWFCGeneratorComponent* GetGeneratorComp() const;

	UWFCGenerator* GetGenerator() const;

	const UWFCAssetModel* GetAssetModel() const;

	UFUNCTION(BlueprintCallable)
	virtual void GetDebugTileInstances(TArray<FWFCTileDebugInstance>& OutTileInstances);

	/** Spawn the tile actors for the currently previewed tiles. */
	UFUNCTION(BlueprintCallable, CallInEditor)
	void SpawnTileActors();

	/** Clear all spawned tile actors. */
	UFUNCTION(BlueprintCallable, CallInEditor)
	void ClearTileActors();

#if UE_ENABLE_DEBUG_DRAWING
	virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
	void AddTileSceneProxy(FDebugRenderSceneProxy* DebugProxy, const FWFCModelAssetTile* AssetTile, FVector Location);
#endif // UE_ENABLE_DEBUG_DRAWING

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

protected:
	// cached value of current cell size
	FVector CachedCellSize;

	UPROPERTY(Transient)
	TArray<AActor*> SpawnedActors;

	virtual void SpawnTileActor(const FWFCModelAssetTile* AssetTile, const FVector& Location);
};
