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
UCLASS()
class WFC_API UWFCAdjacencyConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void NotifyCellChanged(FWFCCellIndex CellIndex) override;
	virtual bool Next() override;

	/**
	 * Add an adjacency mapping that allows AllowedTileId to be placed next to TileId for
	 * an incoming direction (from AllowedTileId -> TileId).
	 */
	void AddAdjacentTileMapping(FWFCTileId TileId, FWFCGridDirection Direction, FWFCTileId AllowedTileId);

	void MarkCellForAdjacencyCheck(FWFCCellIndex Index);

protected:
	/** Reference to the grid being used. */
	UPROPERTY(Transient)
	const UWFCGrid* Grid;

	/** Adjacent tile mappings for each tile. */
	TMap<FWFCTileId, FWFCAdjacentTileMapping> TileAdjacencyMap;

	/** Current list of cells adjacency constraints to check */
	TArray<FWFCCellIndexAndDirection> AdjacentCellDirsToCheck;

	/** Return the array of all valid tiles that an be placed next to a tile in a direction */
	TArray<FWFCTileId> GetValidAdjacentTileIds(FWFCTileId TileId, FWFCGridDirection Direction) const;
};
