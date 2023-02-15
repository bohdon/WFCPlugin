// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileAsset.h"
#include "WFCTileSet.h"
#include "Core/WFCModel.h"
#include "WFCAssetModel.generated.h"


/**
 * A model for use with WFC tile sets and assets.
 */
UCLASS()
class WFC_API UWFCAssetModel : public UWFCModel
{
	GENERATED_BODY()

public:
	/** Add a tile to this model that was generated from an asset */
	void AddTileFromAsset(const UWFCTileAsset* TileAsset, TSharedPtr<FWFCModelTile> Tile);

	/** Return the tile set as a UWFCTileSet. */
	UFUNCTION(BlueprintPure)
	const UWFCTileSet* GetAssetTileSet() const;

	/** Return a FWFCModelAssetTile by id */
	UFUNCTION(BlueprintPure)
	FWFCModelAssetTile GetAssetTile(int32 TileId) const;

	/** Return all tile ids that were generated from a tile asset */
	UFUNCTION(BlueprintCallable)
	FWFCTileIdArray GetTileIdsForAsset(const UWFCTileAsset* TileAsset) const;

	/** Return the tile id that matches a tile asset, tile def index, and rotation */
	UFUNCTION(BlueprintCallable)
	int32 GetTileIdForAssetTileDef(const UWFCTileAsset* TileAsset, int32 TileDefIndex, int32 Rotation) const;

	virtual void ConfigureGenerator(UWFCGenerator* Generator) override;

	virtual FString GetTileDebugString(FWFCTileId TileId) const override;

protected:
	/** Map of all tile ids indexed by tile asset */
	UPROPERTY(Transient)
	TMap<const UWFCTileAsset*, FWFCTileIdArray> TileAssetIds;


public:
	// Adjacency Constraints
	// ---------------------

	// TODO: this is shared functionality, but pick a better place, how do constraints involve themself with the model?
	virtual void ConfigureAdjacencyConstraint(const UWFCGenerator* Generator, class UWFCAdjacencyConstraint* AdjacencyConstraint) const;

	// TODO: this is also coupling the Core/ classes with asset-related classes at the moment
	/**
	 * Return true if TileA and TileB can be adjacent to each other for a direction.
	 * @param Direction The incoming direction from B -> A
	 */
	virtual bool CanTilesBeAdjacent(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB,
	                                FWFCGridDirection Direction, const class UWFCGrid* Grid) const;

	// Boundary Constraints
	// --------------------

	// TODO: this is shared functionality, but pick a better place, how do constraints involve themself with the model?
	virtual void ConfigureBoundaryConstraint(const UWFCGenerator* Generator, class UWFCBoundaryConstraint* BoundaryConstraint) const;

	/**
	 * Return true if TileA can be adjacent to a grid boundary for an outgoing direction.
	 * @param Direction The outgoing direction from A -> Boundary
	 */
	virtual bool CanTileBeAdjacentToGridBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction,
	                                             const class UWFCGrid* Grid, const UWFCGenerator* Generator) const;
};
