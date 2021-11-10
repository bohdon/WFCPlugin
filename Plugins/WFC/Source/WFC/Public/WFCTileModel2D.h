// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCAssetModel.h"
#include "WFCTileAsset.h"
#include "WFCTileModel2D.generated.h"

class UWFCAdjacencyConstraint;
class UWFCGrid;


/**
 * Generates a tile for each asset and possible rotation on a 2D grid.
 * TODO: also generate reflections
 */
UCLASS()
class WFC_API UWFCTileModel2D : public UWFCAssetModel
{
	GENERATED_BODY()

public:
	UWFCTileModel2D();

	virtual void GenerateTiles() override;
	virtual void ConfigureGenerator(UWFCGenerator* Generator) override;
	virtual bool CanTilesBeAdjacent(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB,
	                                FWFCGridDirection Direction, const UWFCGrid* Grid) const override;
};
