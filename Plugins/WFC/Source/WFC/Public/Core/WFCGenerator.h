// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCGenerator.generated.h"

class UWFCGrid;
class UWFCModel;


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

	/** The current state of the generator. */
	UPROPERTY(BlueprintReadOnly)
	EWFCGeneratorState State;

	/** Initialize the generator. */
	UFUNCTION(BlueprintCallable)
	void Initialize(const UWFCGrid* InGrid, const UWFCModel* InModel);

	// TODO: make async
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

	/** Return the selected tiles for every cell in the grid */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void GetSelectedTiles(TArray<FWFCTile>& OutTiles) const;

	DECLARE_MULTICAST_DELEGATE_OneParam(FCellSelectedDelegate, int32 /* CellIndex */);

	FCellSelectedDelegate OnCellSelected;

protected:
	/** The grid being used */
	UPROPERTY(Transient)
	const UWFCGrid* Grid;

	/** The model being used */
	UPROPERTY(Transient)
	const UWFCModel* Model;

	/** Array of all cells in the grid by cell index. */
	TArray<FWFCCell> Cells;

	/** The cached total number of cells in the grid */
	int32 NumCells;

	bool bIsInitialized;

	FORCEINLINE bool IsValidCellIndex(FWFCCellIndex Index) const { return Cells.IsValidIndex(Index); }

	/** Return cell data by index */
	FWFCCell& GetCell(FWFCCellIndex Index);

	/** Return cell data by index */
	const FWFCCell& GetCell(FWFCCellIndex Index) const;

	/** Called when the candidates for a cell have changed. */
	virtual void OnCellChanged(FWFCCellIndex Index);

	/**
	 * Propagate all cached changes by checking and applying constraints.
	 * @return True if any changes were propagated.
	 */
	virtual bool PropagateNext();

	/** Return the next cell that should have a tile selected */
	virtual FWFCCellIndex SelectNextCellIndex();

	/** Return the tile to select for a cell */
	virtual FWFCTileId SelectNextTileForCell(FWFCCellIndex Index);


	// Adjacency Constraint
	// --------------------
public:
	void AddAdjacentTileMapping(FWFCTileId TileId, FWFCGridDirection Direction, FWFCTileId AcceptedTileId);

protected:
	/** Map of tiles that can be placed next to other tiles by id and direction */
	TMap<FWFCTileId, TMap<FWFCGridDirection, TArray<FWFCTileId>>> TileAdjacencyMap;

	/** Current list of cells adjacency constraints to check */
	TArray<FWFCCellIndexAndDirection> AdjacentCellDirsToCheck;

	void MarkCellForAdjacencyCheck(FWFCCellIndex Index);

	/** @return True if a change was propagated, regardless of whether any cells changed. */
	bool PropagateNextAdjacencyConstraint();

	/** Return the array of all valid tiles that an be placed next to a tile in a direction */
	TArray<FWFCTileId> GetValidAdjacentTileIds(FWFCTileId TileId, FWFCGridDirection Direction) const;
};
