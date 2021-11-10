// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileAsset.h"
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
	UFUNCTION(BlueprintPure)
	FWFCModelAssetTile GetTileById(int32 TileId) const;

	// Adjacency Constraints
	// ---------------------

	// TODO: this is shared functionality, but pick a better place, how does adjacency constraint involve itself with the model?
	virtual void ConfigureAdjacencyConstraint(const UWFCGenerator* Generator, class UWFCAdjacencyConstraint* AdjacencyConstraint) const;

	// TODO: this is also coupling the Core/ classes with asset-related classes at the moment
	/**
	 * Return true if TileA and TileB can be adjacent to each other for a direction.
	 * @param Direction The incoming direction from B -> A
	 */
	virtual bool CanTilesBeAdjacent(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB,
	                                FWFCGridDirection Direction, const class UWFCGrid* Grid) const;
};
