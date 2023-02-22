// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/WFCTypes.h"
#include "WFCGeneratorComponent.generated.h"

class UWFCAsset;
class UWFCGenerator;
class UWFCGrid;
class UWFCModel;


USTRUCT(BlueprintType)
struct FWFCGeneratorDebugSettings
{
	GENERATED_BODY()

	FWFCGeneratorDebugSettings()
		: bShowCellCoordinates(false),
		  bShowCandidates(true),
		  bShowEntropy(false),
		  DebugEntropyThreshold(2.f),
		  bShowSelectedTileIds(false),
		  bHighlightUpdatedCells(true),
		  MaxTileIdCount(10),
		  DebugCellScale(FVector(0.6f))
	{
	}

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowCellCoordinates;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowCandidates;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowEntropy;

	/** Only show entropy when it is below this threshold. */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bShowEntropy"), Category = "Debug")
	float DebugEntropyThreshold;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowSelectedTileIds;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bHighlightUpdatedCells;

	/** The maximum number of tile ids to show explicitly before truncating to just a count. */
	UPROPERTY(EditAnywhere, Category = "Debug")
	int32 MaxTileIdCount;

	/** Scale applied to cell boxes in addition to dynamic scaling. */
	UPROPERTY(EditAnywhere, Category = "Debug")
	FVector DebugCellScale;
};


/**
 * A component that handles running a Wave Function Collapse generator,
 * and then populating the scene with tile actors as desired.
 */
UCLASS(ClassGroup=(Procedural), meta=(BlueprintSpawnableComponent))
class WFC_API UWFCGeneratorComponent : public USceneComponent
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

	/** Color of the grid in the editor */
	UPROPERTY(EditAnywhere, Category = "Debug")
	FLinearColor EditorGridColor;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FWFCGeneratorDebugSettings DebugSettings;

	virtual void BeginPlay() override;

	/** Initialize the WFC model and generator */
	UFUNCTION(BlueprintCallable)
	bool Initialize();

	/** Return true if the generator is currently initialized. */
	UFUNCTION(BlueprintPure)
	bool IsInitialized() const;

	/** Reset the current model and generator. */
	UFUNCTION(BlueprintCallable)
	void ResetGenerator();

	/** Run the generator and spawn all actors */
	UFUNCTION(BlueprintCallable)
	void Run();

	/** Iterate the generator one step. */
	UFUNCTION(BlueprintCallable)
	void Next(bool bBreakAfterConstraints = false);

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

	DECLARE_MULTICAST_DELEGATE_OneParam(FCellSelectedDelegate, int32 /*CellIndex*/);

	FCellSelectedDelegate OnCellSelectedEvent;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCellSelectedDynDelegate, int32, CellIndex);

	UPROPERTY(BlueprintAssignable)
	FCellSelectedDynDelegate OnCellSelectedEvent_BP;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStateChangedDynDelegate, EWFCGeneratorState, State);

	/** Called when the generator state has changed */
	UPROPERTY(BlueprintAssignable)
	FStateChangedDynDelegate OnStateChangedEvent_BP;

	DECLARE_MULTICAST_DELEGATE_OneParam(FFinishedDelegate, EWFCGeneratorState /*State*/)

	FFinishedDelegate OnFinishedEvent;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFinishedDynDelegate, bool, bSuccess);

	/** Called when the generated has finished running */
	UPROPERTY(BlueprintAssignable)
	FFinishedDynDelegate OnFinishedEvent_BP;

	UWFCGenerator* GetGenerator() const { return Generator; }

protected:
	/** The model instance. */
	UPROPERTY(Transient, BlueprintReadOnly)
	UWFCModel* Model;

	/** The generator instance */
	UPROPERTY(Transient, BlueprintReadOnly)
	UWFCGenerator* Generator;

	void OnCellSelected(int32 CellIndex);
	void OnStateChanged(EWFCGeneratorState State);
};
