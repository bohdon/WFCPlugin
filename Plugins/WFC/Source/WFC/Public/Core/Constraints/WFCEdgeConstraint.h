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
 * TODO: rename to something with 'Tag' in it for clarity.
 */
UCLASS(DisplayName = "Adjacency Constraint")
class WFC_API UWFCEdgeConstraint : public UWFCArcConsistencyConstraint
{
	GENERATED_BODY()

public:
	UWFCEdgeConstraint();

	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void ApplySnapshot(const UWFCConstraintSnapshot* Snapshot) override;

	/** Return true if two edges are allowed to be next to each other. */
	virtual bool AreEdgesCompatible(const FGameplayTag& EdgeA, const FGameplayTag& EdgeB) const;

	/** Return true if TileB can be placed next to TileA in a direction going from A -> B. */
	virtual bool AreTilesCompatible(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB, FWFCGridDirection Direction) const;

protected:
	bool bIsInitializedFromTiles;
	
	/** Reference to the asset model required for this constraint. */
	UPROPERTY(Transient)
	TObjectPtr<const UWFCAssetModel> AssetModel;

	/**
	 * Initialize the allowed tiles by iterating all tiles in the model,
	 * looking up their edge types, and adding mappings.
	 */
	void InitializeFromTiles();

	/**
	 * Initialize the allowed tiles by iterating all assets, checking their edge types against
	 * other assets, then adding mappings for the matching tiles and rotation variants.
	 * Experimental, performs less checks but inefficiently attempts to add redundant entries
	 * and has issues if there are duplicate tile assets in the tile set.
	 */
	void InitializeFromAssets();

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
	 * @param OutDirectionA The direction local to A, going from A->B. 
	 * @param TileBIds The list of tile ids for tile def B.
	 * @param OutDirectionB The direction local to B, going from B->A.
	 */
	void AddMatchingEdgeAllowedTiles(const TArray<FWFCTileId>& TileAIds, FWFCGridDirection OutDirectionA,
	                                 const TArray<FWFCTileId>& TileBIds, FWFCGridDirection OutDirectionB);
};
