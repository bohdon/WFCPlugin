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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<const UWFCModel> Model;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<const UWFCGridConfig> GridConfig;

	/** The constraints to create and use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UWFCConstraint>> ConstraintClasses;
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

	/** Return the total number of cells */
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumCells() const { return NumCells; }

	/** Return the total number of unique tile types. */
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetNumTiles() const { return NumTiles; }

	FORCEINLINE const UWFCGrid* GetGrid() const { return Grid; }

	template <class T>
	const T* GetGrid() const
	{
		return Cast<T>(GetGrid());
	}

	/** Initialize the generator. */
	UFUNCTION(BlueprintCallable)
	void Initialize(FWFCGeneratorConfig InConfig);

	/** Run the generator until it is either finished, or an error occurs. */
	UFUNCTION(BlueprintCallable)
	void Run(int32 StepLimit = 100000);

	/** Continue the generator forward by selecting the next tile. */
	UFUNCTION(BlueprintCallable)
	void Next();

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

	/** Return cell data by index */
	FWFCCell& GetCell(FWFCCellIndex CellIndex);

	/** Return cell data by index */
	const FWFCCell& GetCell(FWFCCellIndex CellIndex) const;

	/** Return the total number of candidates for a cell */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	int32 GetNumCellCandidates(int32 CellIndex) const;

	FORCEINLINE TArray<UWFCConstraint*>& GetConstraints() { return Constraints; }

	FORCEINLINE const TArray<UWFCConstraint*>& GetConstraints() const { return Constraints; }

	/** Return the selected tiles for every cell in the grid */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void GetSelectedTileIds(TArray<int32>& OutTileIds) const;

	DECLARE_MULTICAST_DELEGATE_OneParam(FCellSelectedDelegate, int32 /* CellIndex */);

	FCellSelectedDelegate OnCellSelected;

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

	/** Create and initialize the grid. */
	virtual void InitializeGrid(const UWFCGridConfig* GridConfig);

	/** Create and initialize constraint objects. */
	virtual void InitializeConstraints(TArray<TSubclassOf<UWFCConstraint>> ConstraintClasses);

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
};
