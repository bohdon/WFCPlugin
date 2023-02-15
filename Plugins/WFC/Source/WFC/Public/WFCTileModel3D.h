// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCAssetModel.h"
#include "WFCTileAsset.h"
#include "WFCTileAsset3D.h"
#include "WFCTileModel3D.generated.h"

class UWFCAdjacencyConstraint;
class UWFCGrid;


/**
 * Generates a tile for each asset and possible 3D rotations.
 */
UCLASS()
class WFC_API UWFCTileModel3D : public UWFCAssetModel
{
	GENERATED_BODY()

public:
	UWFCTileModel3D();

	virtual void GenerateTiles() override;
	virtual bool CanTilesBeAdjacent(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB,
	                                FWFCGridDirection Direction, const UWFCGrid* Grid) const override;
	virtual bool CanTileBeAdjacentToGridBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction,
	                                             const UWFCGrid* Grid, const UWFCGenerator* Generator) const override;
};
