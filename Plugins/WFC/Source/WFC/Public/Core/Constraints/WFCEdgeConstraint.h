// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WFCArcConsistencyConstraint.h"
#include "WFCTileAsset.h"
#include "WFCEdgeConstraint.generated.h"

class UWFCAssetModel;


/**
 * Constrains tiles such that only those with matching edge tags can be placed next to each other.
 */
UCLASS(DisplayName = "Adjacency Constraint")
class WFC_API UWFCEdgeConstraint : public UWFCArcConsistencyConstraint
{
	GENERATED_BODY()

public:
	UWFCEdgeConstraint();

	virtual void Initialize(UWFCGenerator* InGenerator) override;

	/** Return true if two edges are allowed to be next to each other. */
	virtual bool AreEdgesCompatible(const FGameplayTag& EdgeA, const FGameplayTag& EdgeB) const;

protected:
	/** Reference to the asset model required for this constraint. */
	UPROPERTY(Transient)
	TObjectPtr<const UWFCAssetModel> AssetModel;

	/**
	 * Add allowed tiles for a large tile interior edge, including all rotation variants.
	 * @param TileAsset The tile asset containing the tile def.
	 * @param TileDefIndex The tile def to add neighbor allowed tile entries.
	 * @param Direction The local direction in the tile asset pointing to the neighbor.
	 * @param TileDefIds The list of tile ids matching the tile def index, precalculated for faster lookup.
	 */
	void AddInteriorAllowedTiles(const UWFCTileAsset* TileAsset, int32 TileDefIndex, FWFCGridDirection Direction,
	                             const TArray<FWFCTileId>& TileDefIds);

	/**
	 * Add allowed tiles for a tile def whose edge is compatible with another tile def.
	 * Adds mappings for rotation variants in which the local direction pairing is the same.
	 * @param TileAIds The list of tile ids for tile def A.
	 * @param InDirectionA The local incoming direction into tile def A. 
	 * @param TileBIds The list of tile ids for tile def B.
	 * @param OutDirectionB The local outgoing direction of tile def B that matches the incoming direction of A.
	 */
	void AddMatchingEdgeAllowedTiles(const TArray<FWFCTileId>& TileAIds, FWFCGridDirection InDirectionA,
	                                 const TArray<FWFCTileId>& TileBIds, FWFCGridDirection OutDirectionB);
};
