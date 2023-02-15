// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Core/WFCConstraint.h"
#include "WFCBoundaryConstraint.generated.h"

class UWFCGrid;


/**
 * Require tiles placed next to the grid boundary to follow certain rules.
 */
UCLASS(DisplayName = "Boundary Constraint Config")
class WFC_API UWFCBoundaryConstraintConfig : public UWFCConstraintConfig
{
	GENERATED_BODY()

public:
	UWFCBoundaryConstraintConfig();

	/** An optional tag query that tile edges must match to be allowed to be adjacent to the boundary. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagQuery EdgeTypeQuery;

	virtual TSubclassOf<UWFCConstraint> GetConstraintClass() const override;

	virtual void Configure(UWFCConstraint* Constraint) const override;
};


/**
 * Require tiles placed next to the grid boundary to follow certain rules.
 * This is useful for enforcing that large tiles remain wholly inside the grid.
 */
UCLASS()
class WFC_API UWFCBoundaryConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	/** An optional tag query that tile edges must match to be allowed to be adjacent to the boundary. */
	UPROPERTY()
	FGameplayTagQuery EdgeTypeQuery;

	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void Reset() override;
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

	/** Cached map of tiles to ban for each cell. Calculated after the first time this constraint is run incase it needs to re-run */
	TMap<FWFCCellIndex, TArray<FWFCTileId>> CachedTileBans;

	/** Return true if a tile is allowed to be adjacent to boundaries in the given outgoing directions. */
	bool CanTileBeAdjacentToBoundaries(FWFCTileId TileId, const TArray<FWFCGridDirection>& BoundaryDirections) const;
};
