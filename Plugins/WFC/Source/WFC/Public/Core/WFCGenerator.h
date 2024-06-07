// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "WFCGenerator.generated.h"

class UWFCConstraintSnapshot;
class UWFCCellSelector;
class UWFCModel;
class UWFCGrid;
class UWFCGridConfig;
class UWFCConstraint;


/** Stores information about a generator that can be used to restore state. */
UCLASS(DefaultToInstanced, EditInlineNew)
class WFC_API UWFCGeneratorSnapshot : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	TArray<FWFCCell> Cells;

	/** Snapshots for each of the constraints, by class. */
	UPROPERTY(VisibleAnywhere)
	TMap<TSubclassOf<UWFCConstraint>, TObjectPtr<UWFCConstraintSnapshot>> ConstraintSnapshots;
};


/**
 * Required objects and settings for initializing a WFCGenerator.
 */
USTRUCT(BlueprintType)
struct FWFCGeneratorConfig
{
	GENERATED_BODY()

	FWFCGeneratorConfig()
	{
	}

	UPROPERTY()
	TWeakObjectPtr<UWFCModel> Model;

	UPROPERTY()
	TWeakObjectPtr<const UWFCGridConfig> GridConfig;

	UPROPERTY()
	TArray<TSubclassOf<UWFCConstraint>> ConstraintClasses;

	UPROPERTY()
	TArray<TSubclassOf<UWFCCellSelector>> CellSelectorClasses;
};


// TODO: make async

/**
 * Handles running the actual processes for selecting, banning, and propagating
 * changes for a WFC model, grid, and tile set.
 */
UCLASS(BlueprintType, Blueprintable)
class WFC_API UWFCGenerator : public UObject
{
	GENERATED_BODY()

public:
	UWFCGenerator();

	/** The configuration supplied to this generator on initialize. */
	UPROPERTY(BlueprintReadOnly)
	FWFCGeneratorConfig Config;

	/** The current state of the generator. */
	UPROPERTY(BlueprintReadOnly)
	EWFCGeneratorState State;

	void SetState(EWFCGeneratorState NewState);

	/** The granularity of work that should be performed when calling Next() */
	UPROPERTY(BlueprintReadWrite)
	EWFCGeneratorStepGranularity StepGranularity;

	/** Return the total number of cells */
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumCells() const { return NumCells; }

	/** Return the total number of unique tile types. */
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumTiles() const { return NumTiles; }

	/** Return the model object, containing all expanded tiles. */
	UFUNCTION(BlueprintPure)
	const UWFCModel* GetModel() const { return Config.Model.Get(); }

	/** Return the grid config of the source asset. */
	UFUNCTION(BlueprintPure)
	const UWFCGridConfig* GetGridConfig() const { return Config.GridConfig.Get(); }

	template <class T>
	const T* GetModel() const
	{
		return Cast<T>(GetModel());
	}

	FORCEINLINE const UWFCGrid* GetGrid() const { return Grid; }

	template <class T>
	const T* GetGrid() const
	{
		return Cast<T>(GetGrid());
	}

	/** Return a constraint by class. */
	UFUNCTION(BlueprintPure, Meta = (DeterminesOutputType="ConstraintClass"))
	UWFCConstraint* GetConstraint(TSubclassOf<UWFCConstraint> ConstraintClass) const;

	/** Return a cell selector by class. */
	UFUNCTION(BlueprintPure, Meta = (DeterminesOutputType="SelectorClass"))
	UWFCCellSelector* GetCellSelector(TSubclassOf<UWFCCellSelector> SelectorClass) const;

	/** Set the config. Should be called before Initialize. */
	void Configure(FWFCGeneratorConfig InConfig);

	/**
	 * Initialize the generator.
	 * @param bFull If true, perform a full initialization of the model and constraints, otherwise those must be done manually.
	 */
	UFUNCTION(BlueprintCallable)
	void Initialize(bool bFull = true);

	/** Initialize all constraints. */
	UFUNCTION(BlueprintCallable)
	void InitializeConstraints();

	UFUNCTION(BlueprintPure)
	bool IsInitialized() const { return bIsInitialized; }

	/** Reset all cells and constraints to their initialized state. */
	UFUNCTION(BlueprintCallable)
	void Reset();

	/** Run the generator until it is either finished, or an error occurs. */
	UFUNCTION(BlueprintCallable)
	void Run(int32 StepLimit = 100000);

	/** Run the deterministic constraints for startup only, abort before any tile selection. */
	UFUNCTION(BlueprintCallable)
	void RunStartup(int32 StepLimit = 100000);

	/** Continue the generator forward by selecting the next tile. */
	UFUNCTION(BlueprintCallable, Meta = (AdvancedDisplay = "0"))
	void Next(bool bNoSelection = false);

	/**
	 * Ban a tile from being a candidate for a cell.
	 * @return True if a contradiction was created.
	 */
	UFUNCTION(BlueprintCallable)
	bool Ban(int32 CellIndex, int32 TileId);

	/**
	 * Ban multiple tiles from being candidates for a cell.
	 * @return True if a contradiction was created.
	 */
	UFUNCTION(BlueprintCallable)
	bool BanMultiple(int32 CellIndex, TArray<int32> TileIds);

	/**
	 * Select a tile to use for a cell.
	 * This is equivalent to banning all other tile candidates.
	 */
	UFUNCTION(BlueprintCallable)
	void Select(int32 CellIndex, int32 TileId);

	FORCEINLINE bool IsValidCellIndex(FWFCCellIndex Index) const { return Cells.IsValidIndex(Index); }

	FORCEINLINE bool IsValidTileId(FWFCTileId TileId) const { return TileId >= 0 && TileId < NumTiles; }

	/** Return cell data by index */
	FWFCCell& GetCell(FWFCCellIndex CellIndex);

	/** Return cell data by index */
	const FWFCCell& GetCell(FWFCCellIndex CellIndex) const;

	/** Return the total number of candidates for a cell */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	int32 GetNumCellCandidates(int32 CellIndex) const;

	FORCEINLINE TArray<TObjectPtr<UWFCConstraint>>& GetConstraints() { return Constraints; }

	FORCEINLINE const TArray<TObjectPtr<UWFCConstraint>>& GetConstraints() const { return Constraints; }

	FString GetTileDebugString(int32 TileId) const;

	/** Return the selected tiles for every cell in the grid */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void GetSelectedTileIds(TArray<int32>& OutTileIds) const;

	/** Create an return a snapshot of this generator. */
	UWFCGeneratorSnapshot* CreateSnapshot(UObject* Outer) const;

	/** Update the state of this generator to a previous snapshot. */
	void ApplySnapshot(const UWFCGeneratorSnapshot* Snapshot);

	const TArray<FWFCCellIndex>& GetCellsAffectedThisUpdate() const { return CellsAffectedThisUpdate; }

	DECLARE_MULTICAST_DELEGATE_OneParam(FCellSelectedDelegate, int32 /* CellIndex */);

	/** Called when a cell has been fully collapsed to a single selected tile id. */
	FCellSelectedDelegate OnCellSelected;

	DECLARE_MULTICAST_DELEGATE_OneParam(FStateChangedDelegate, EWFCGeneratorState /* State */);

	/** Called when the state has changed */
	FStateChangedDelegate OnStateChanged;

protected:
	/** The grid being used */
	UPROPERTY(Transient)
	TObjectPtr<const UWFCGrid> Grid;

	/** The constraint instances to apply, in order of priority. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UWFCConstraint>> Constraints;

	/** The cell selector instances to use, in order of priority. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UWFCCellSelector>> CellSelectors;

	/** The cached total number of available tiles. */
	int32 NumTiles;

	/** Array of all cells in the grid by cell index. */
	TArray<FWFCCell> Cells;

	/** The cached total number of cells in the grid */
	int32 NumCells;

	bool bIsInitialized;

	bool bDidSelectCellThisStep;

	/** Tracks how many bans occur during an update. */
	int32 NumBansThisUpdate;

	EWFCGeneratorStepPhase CurrentStepPhase;

	/** Array of cells that were modified during the last update. */
	TArray<FWFCCellIndex> CellsAffectedThisUpdate;

	/** Create and initialize the grid. */
	virtual void InitializeGrid(const UWFCGridConfig* GridConfig);

	virtual void CreateConstraints();

	virtual void CreateCellSelectors();

	/** Create and initialize cell selector objects. */
	virtual void InitializeCellSelectors();

	/** Populate the cells array with default values for every cell in the grid */
	virtual void InitializeCells();

	bool AreAllCellsSelected() const;

	/** Called when a tile candidate has been banned from a cell. */
	virtual void OnCellCandidateBanned(FWFCCellIndex CellIndex, FWFCTileId BannedTileId);

	/** Called when tile candidates have been banned from a cell. */
	virtual void OnCellCandidatesBanned(FWFCCellIndex CellIndex, const TArray<FWFCTileId>& BannedTileIds);

	/** Called when the candidates for a cell have changed. */
	virtual void OnCellChanged(FWFCCellIndex CellIndex);

	/** Return the next cell that should be fully collapsed. */
	virtual FWFCCellIndex SelectNextCellIndex();

	/** Return the tile to select for a cell being collapsed. */
	virtual FWFCTileId SelectNextTileForCell(FWFCCellIndex CellIndex);

public:
	template <class T>
	T* GetConstraint()
	{
		T* Result = nullptr;
		if (Constraints.FindItemByClass(&Result))
		{
			return Result;
		}
		return nullptr;
	}

	template <class T>
	const T* GetConstraint() const
	{
		const T* Result = nullptr;
		if (Constraints.FindItemByClass(&Result))
		{
			return Result;
		}
		return nullptr;
	}

	template <class T>
	const T* GetCellSelector() const
	{
		const T* Result = nullptr;
		if (CellSelectors.FindItemByClass(&Result))
		{
			return Result;
		}
		return nullptr;
	}
};
