// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCConstraint.h"
#include "WFCArcConsistencyConstraint.generated.h"


/**
 * Base class for a constraint that uses Generalized Arc Consistency to ensure remaining tile candidates for a cell are valid.
 * The most common use case for this is the adjacency constraint, which removes tile candidates that are not allowed to be adjacent
 * to selected tiles for a cell.
 *
 * This constraint uses the AC4 algorithm, which keeps a count of how many "supports" (or valid remaining options)
 * a cell has left for each tile candidate, decrements the support count when an option is removed,
 * and removes a tile candidate when it's support count reaches 0.
 * See https://www.boristhebrave.com/2021/08/30/arc-consistency-explained/ for more info.
 */
UCLASS(Abstract)
class WFC_API UWFCArcConsistencyConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	UWFCArcConsistencyConstraint();

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
	virtual void NotifyCellBan(FWFCCellIndex CellIndex, FWFCTileId BannedTileId) override;
	virtual bool Next() override;
	virtual void LogDebugInfo() const override;

	/**
	 * Add an entry to the table that allows a tile to be placed next to another tile
	 * for an incoming direction (from AllowedTileId -> TileId).
	 * @param TileId The tile that defines a constraint for other tiles adjacent to it.
	 * @param Direction The direction from Tile -> AllowedTileId.
	 * @param AllowedTileId The tile that is allowed to exist adjacent to the tile.
	 */
	void AddAllowedTileForDirection(FWFCTileId TileId, FWFCGridDirection Direction, FWFCTileId AllowedTileId);

	/** Return the array of all valid tiles that can be placed next to a tile in a direction. */
	const TArray<FWFCTileId>& GetAllowedTileIds(FWFCTileId TileId, FWFCGridDirection Direction) const;

	const TArray<FWFCCellIndexAndTileId>& GetBansToPropagate() const { return BansToPropagate; }

	const TArray<FWFCCellIndexAndDirection>& GetVisitedDuringPropagation() const { return VisitedDuringPropagation; }

protected:
	/** Contains the allowed list of tiles for each [TileId][Direction]. */
	TArray<TArray<TArray<FWFCTileId>>> AllowedTiles;

	/** Contains the number of supports for each [CellIndex][TileId][Direction]. */
	TArray<TArray<TArray<int16>>> SupportCounts;

	/** Cached copy of the support counts after initialization for faster resetting. */
	TArray<TArray<TArray<int16>>> DefaultSupportCounts;

	/** List of banned tiles per cell that need to be propagated in the next update. */
	TArray<FWFCCellIndexAndTileId> BansToPropagate;

	/** Unique cell directions that were visited during the last propagation. */
	TArray<FWFCCellIndexAndDirection> VisitedDuringPropagation;

	bool bDidApplyInitialConsistency;

	/** Initialize support counts and check for contradictions. */
	void ApplyInitialConsistency();

	/** Propagate changes due to banned tiles and ensure consistency. */
	bool PropagateChanges();
};
