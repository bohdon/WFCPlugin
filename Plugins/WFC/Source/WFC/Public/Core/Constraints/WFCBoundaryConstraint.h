// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCConstraint.h"
#include "WFCBoundaryConstraint.generated.h"

class UWFCGrid;


/**
 * Constrains prohibited tiles from being adjacent to grid boundaries.
 * This is useful for enforcing that large tiles remain wholly inside the grid.
 */
UCLASS()
class WFC_API UWFCBoundaryConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual bool Next() override;

	/**
	 * Add a mapping that prohibits a tile from being placed next to a grid boundary for an outgoing direction.
	 */
	void AddProhibitedAdjacentBoundaryMapping(FWFCTileId TileId, FWFCGridDirection Direction);

protected:
	/** Reference to the grid being used. */
	UPROPERTY(Transient)
	const UWFCGrid* Grid;

	/** Map of tiles and the outgoing directions for which they are prohibited from being adjacent to the grid boundary. */
	TMap<FWFCTileId, TArray<FWFCGridDirection>> TileBoundaryProhibitionMap;

	bool bDidApplyInitialConstraint;

	/** Return true if a tile is allowed to be adjacent to boundaries in the given outgoing directions. */
	bool CanTileBeAdjacentToBoundaries(FWFCTileId TileId, const TArray<FWFCGridDirection>& BoundaryDirections) const;
};
