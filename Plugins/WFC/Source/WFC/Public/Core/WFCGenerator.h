// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCGenerator.generated.h"

class UWFCModel;
class UWFCGrid;
class UWFCGridConfig;
class UWFCConstraint;
class UWFCConstraintConfig;


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
	TWeakObjectPtr<const UWFCModel> Model;

	UPROPERTY()
	TWeakObjectPtr<const UWFCGridConfig> GridConfig;

	/** The constraints to create and use. */
	UPROPERTY()
	TArray<TWeakObjectPtr<UWFCConstraintConfig>> ConstraintConfigs;
};


// TODO: make async
// TODO: rename UWFCTileSelector

/**
 * Handles running the actual processes for selecting, banning, and propagating
 * changes for a WFC model, grid, and tile set.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
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

	/** Return the total number of cells */
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumCells() const { return NumCells; }

	/** Return the total number of unique tile types. */
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumTiles() const { return NumTiles; }

	FORCEINLINE const UWFCModel* GetModel() const { return Config.Model.Get(); }

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

	/** Initialize the generator. */
	UFUNCTION(BlueprintCallable)
	void Initialize(FWFCGeneratorConfig InConfig);

	UFUNCTION(BlueprintPure)
	bool IsInitialized() const { return bIsInitialized; }

	/** Reset all cells and constraints to their initialized state. */
	UFUNCTION(BlueprintCallable)
	void Reset();

	/** Run the generator until it is either finished, or an error occurs. */
	UFUNCTION(BlueprintCallable)
	void Run(int32 StepLimit = 100000);

	/** Continue the generator forward by selecting the next tile. */
	UFUNCTION(BlueprintCallable)
	void Next(bool bBreakAfterConstraints = false);

	/** Ban a tile from being a candidate for a cell. */
	UFUNCTION(BlueprintCallable)
	void Ban(int32 CellIndex, int32 TileId);

	/** Ban multiple tiles from being candidates for a cell. */
	UFUNCTION(BlueprintCallable)
	void BanMultiple(int32 CellIndex, TArray<int32> TileIds);

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

	FORCEINLINE TArray<UWFCConstraint*>& GetConstraints() { return Constraints; }

	FORCEINLINE const TArray<UWFCConstraint*>& GetConstraints() const { return Constraints; }

	FString GetTileDebugString(int32 TileId) const;

	/** Return the selected tiles for every cell in the grid */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void GetSelectedTileIds(TArray<int32>& OutTileIds) const;

	DECLARE_MULTICAST_DELEGATE_OneParam(FCellSelectedDelegate, int32 /* CellIndex */);

	FCellSelectedDelegate OnCellSelected;

	DECLARE_MULTICAST_DELEGATE_OneParam(FStateChangedDelegate, EWFCGeneratorState /* State */);

	/** Called when the state has changed */
	FStateChangedDelegate OnStateChanged;

protected:
	/** The grid being used */
	UPROPERTY(Transient)
	const UWFCGrid* Grid;

	/** The constraints to apply, in order of priority, during generation. */
	UPROPERTY(Transient)
	TArray<UWFCConstraint*> Constraints;

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

	/** Create and initialize the grid. */
	virtual void InitializeGrid(const UWFCGridConfig* GridConfig);

	/** Create and initialize constraint objects. */
	virtual void InitializeConstraints();

	/** Populate the cells array with default values for every cell in the grid */
	virtual void InitializeCells();

	/** Called when the candidates for a cell have changed. */
	virtual void OnCellChanged(FWFCCellIndex CellIndex);

	/** Return the next cell that should have a tile selected */
	virtual FWFCCellIndex SelectNextCellIndex();

	/** Return the tile to select for a cell */
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
};
