// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCConstraint.h"
#include "Core/WFCTypes.h"
#include "WFCAdjacencyConstraint.generated.h"

class UWFCGrid;


/**
 * Mapping of directions to tiles that are allowed to be placed next to another tile.
 */
struct FWFCAdjacentTileMapping
{
	/** Map of tiles that are allowed to be placed next to this tile for an incoming direction */
	TMap<FWFCGridDirection, TArray<FWFCTileId>> AllowedTiles;
};


/**
 * Constrains tiles such that only explicitly allowed tiles can be
 * placed next to other tiles in any given direction.
 */
UCLASS(DisplayName = "Adjacency Constraint")
class WFC_API UWFCAdjacencyConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	UWFCAdjacencyConstraint();

	/**
	 * When true, treat cells with no candidates as empty spaces and don't
	 * use them to ban candidates from neighboring cells.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIgnoreContradictionCells;

	/** If true, stop after every tile check for debugging purposes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugNext;

	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void Reset() override;
	virtual void NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection) override;
	virtual bool Next() override;

	/**
	 * Add an adjacency mapping that allows AllowedTileId to be placed next to TileId for
	 * an incoming direction (from AllowedTileId -> TileId).
	 */
	void AddAdjacentTileMapping(FWFCTileId TileId, FWFCGridDirection Direction, FWFCTileId AllowedTileId);

	void MarkCellForAdjacencyCheck(FWFCCellIndex CellIndex);

	const TArray<FWFCCellIndexAndDirection>& GetAdjacentCellDirsToCheck() const { return AdjacentCellDirsToCheck; }

	const TMap<FWFCCellIndex, FWFCCellIndex>& GetAdjacenciesEnforcedThisUpdate() const { return AdjacenciesEnforcedThisUpdate; }

protected:
	/** Reference to the grid being used. */
	UPROPERTY(Transient)
	const UWFCGrid* Grid;

	/** Adjacent tile mappings for each tile. */
	TMap<FWFCTileId, FWFCAdjacentTileMapping> TileAdjacencyMap;

	/** Current list of cells adjacency constraints to check */
	TArray<FWFCCellIndexAndDirection> AdjacentCellDirsToCheck;

	/** Map of cells that were affected by this constraint in the last update. */
	TMap<FWFCCellIndex, FWFCCellIndex> AdjacenciesEnforcedThisUpdate;

	/** Return the array of all valid tiles that an be placed next to a tile in a direction */
	TArray<FWFCTileId> GetValidAdjacentTileIds(FWFCTileId TileId, FWFCGridDirection Direction) const;
};
