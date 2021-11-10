﻿// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/WFCTypes.h"
#include "WFCGeneratorComponent.generated.h"

class UWFCAsset;
class UWFCGenerator;
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

	/** The asset defining which model and tile set to use */
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
	void InitializeGenerator();

	/** Run the generator and spawn all actors */
	UFUNCTION(BlueprintCallable)
	void Run();

	/** Return the current state of the generator */
	UFUNCTION(BlueprintPure)
	EWFCGeneratorState GetState() const;

	/** Return the selected tiles for every cell in the grid */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void GetSelectedTiles(TArray<FWFCTile>& OutTiles) const;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCellSelectedDynDelegate, int32, CellIndex);

	UPROPERTY(BlueprintAssignable)
	FCellSelectedDynDelegate OnCellSelectedEvent_BP;

protected:
	/** The generator instance */
	UPROPERTY(Transient, BlueprintReadOnly)
	UWFCGenerator* Generator;

	/** The model instance */
	UPROPERTY(Transient, BlueprintReadOnly)
	UWFCModel* Model;

	void AddAdjacencyMappings();

	void OnCellSelected(int32 CellIndex);
};
