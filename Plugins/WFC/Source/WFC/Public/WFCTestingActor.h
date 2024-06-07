// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileAsset.h"
#include "GameFramework/Actor.h"
#include "WFCTestingActor.generated.h"

class UWFCGeneratorComponent;
class UWFCRenderingComponent;


/**
 * Actor for running a WFC generator and handling the spawning of tile actors.
 */
UCLASS()
class WFC_API AWFCTestingActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWFCGeneratorComponent> WFCGenerator;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWFCRenderingComponent> WFCRendering;
#endif

public:
	AWFCTestingActor(const FObjectInitializer& ObjectInitializer);

	/** Load tile actor levels when the generator finishes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	bool bLoadTileLevelsOnFinish;

	/** Spawn tiles even when the generator errors out. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	bool bSpawnOnError;

	/** Spawn tile actors as soon as they are selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	bool bSpawnOnCellSelection;

	/** Load tile actor levels as soon as they are spawned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	bool bLoadTileLevelsOnSelection;

	/** Return the tile that was selected for a collapsed cell. */
	const FWFCModelAssetTile* GetAssetTileForCell(int32 CellIndex) const;

	/**
	 * Return the transform to use for spawning a tile in a cell,
	 * incorporating the rotation of a tile as well.
	 */
	UFUNCTION(BlueprintPure, Category = "WFC")
	FTransform GetCellTransform(int32 CellIndex, int32 Rotation = 0) const;

	/** Spawn a tile instance actor for a cell, if it has been selected and has a valid level. */
	UFUNCTION(BlueprintCallable, Category = "WFC")
	virtual AActor* SpawnActorForCell(int32 CellIndex, bool bAutoLoad);

	/** Return a spawned tile actor for a cell. */
	UFUNCTION(BlueprintPure, Category = "WFC")
	virtual AActor* GetActorForCell(int32 CellIndex) const;

	/** Spawn tile instance actors for all selected cells in the generator. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WFC")
	virtual void SpawnActorsForAllCells();

	/** Load all level instances for each spawned tile actor. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WFC")
	virtual void LoadAllTileActorLevels();

	/** Destroy all spawned tile instance actors. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WFC")
	virtual void DestroyAllSpawnedActors();

	virtual void PostInitializeComponents() override;

protected:
	/** Actors that have been spawned for each cell. */
	UPROPERTY(Transient)
	TMap<int32, TWeakObjectPtr<AActor>> SpawnedTileActors;

	/** Called when the generator has selected a cell. */
	void OnCellSelected(int32 CellIndex);

	/** Called when the generator has finished. */
	void OnGeneratorFinished(EWFCGeneratorState State);
};
