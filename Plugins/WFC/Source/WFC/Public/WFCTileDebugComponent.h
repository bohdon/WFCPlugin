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
		: Location(FVector::ZeroVector),
		  TileId(INDEX_NONE)
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Setter, Meta = (ClampMin="0"))
	int32 TileId;

	UFUNCTION(BlueprintSetter)
	void SetTileId(int32 NewTileId);

	/** Automatically spawn tile actors when the tile id changes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoSpawn;

	/** Delay before auto spawning to prevent hitches when rapidly changing id. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoSpawnDelay;

	/** The spacing to place between tiles, as a factor of cell size. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Spacing;

	UFUNCTION(BlueprintPure)
	int32 GetMaxTileId() const;

	UFUNCTION(BlueprintPure)
	FVector GetCellSize() const;

	virtual void PostInitProperties() override;

	UWFCGeneratorComponent* GetGeneratorComp() const;

	UWFCGenerator* GetGenerator() const;

	const UWFCAssetModel* GetAssetModel() const;

	/** Return the debug string for the currently selected tile. */
	UFUNCTION(BlueprintPure)
	FString GetTileDebugString() const;

	UFUNCTION(BlueprintCallable)
	virtual void GetDebugTileInstances(TArray<FWFCTileDebugInstance>& OutTileInstances) const;

	/** Spawn the tile actors for the currently previewed tiles. */
	UFUNCTION(BlueprintCallable, CallInEditor)
	void SpawnTileActors();

	/** Clear all spawned tile actors. */
	UFUNCTION(BlueprintCallable, CallInEditor)
	void ClearTileActors();

	UFUNCTION(BlueprintNativeEvent)
	void OnTileIdChanged();

#if UE_ENABLE_DEBUG_DRAWING
	virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
	void AddTileSceneProxy(FDebugRenderSceneProxy* DebugProxy, const FWFCModelAssetTile* AssetTile, FVector Location);
#endif // UE_ENABLE_DEBUG_DRAWING

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

protected:
	// cached value of current cell size
	FVector CachedCellSize;

	FTimerHandle AutoSpawnTimer;

	UPROPERTY(Transient)
	TArray<AActor*> SpawnedActors;

	virtual void SpawnTileActor(const FWFCModelAssetTile* AssetTile, const FVector& Location);
};
