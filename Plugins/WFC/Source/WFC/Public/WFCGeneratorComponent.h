// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/WFCTypes.h"
#include "WFCGeneratorComponent.generated.h"

class UWFCAsset;
class UWFCGenerator;
class UWFCGrid;
class UWFCModel;


/**
 * A component that handles running a Wave Function Collapse generator,
 * and then populating the scene with tile actors as desired.
 */
UCLASS(ClassGroup=(Procedural), meta=(BlueprintSpawnableComponent))
class WFC_API UWFCGeneratorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWFCGeneratorComponent();

	/** The asset which contains all settings needed to create and run the generator. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UWFCAsset> WFCAsset;

	/** The maximum number of steps to allow during generation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StepLimit;

	/** If true, automatically run the generator on begin play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoRun;

	virtual void BeginPlay() override;

	/** Initialize the WFC generator */
	UFUNCTION(BlueprintCallable)
	bool InitializeGenerator();

	/** Run the generator and spawn all actors */
	UFUNCTION(BlueprintCallable)
	void Run();

	/** Return the grid being used by the generator. */
	UFUNCTION(BlueprintPure)
	const UWFCGrid* GetGrid() const;

	/** Return the current state of the generator */
	UFUNCTION(BlueprintPure)
	EWFCGeneratorState GetState() const;

	/**
	 * Return a selected tile id by cell index.
	 * @see Model for retrieving tile objects.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void GetSelectedTileId(int32 CellIndex, bool& bSuccess, int32& TileId) const;

	/**
	 * Return the selected tile ids for every cell in the grid.
	 * @see Model for retrieving tile objects.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void GetSelectedTileIds(TArray<int32>& TileIds) const;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCellSelectedDynDelegate, int32, CellIndex);

	UPROPERTY(BlueprintAssignable)
	FCellSelectedDynDelegate OnCellSelectedEvent_BP;

protected:
	/** The model instance. */
	UPROPERTY(Transient, BlueprintReadOnly)
	UWFCModel* Model;

	/** The generator instance */
	UPROPERTY(Transient, BlueprintReadOnly)
	UWFCGenerator* Generator;

	void OnCellSelected(int32 CellIndex);
};
