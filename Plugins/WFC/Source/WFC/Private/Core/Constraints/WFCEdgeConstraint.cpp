// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCEdgeConstraint.h"

#include "WFCAssetModel.h"
#include "WFCModule.h"
#include "WFCTileSet.h"
#include "Core/WFCGenerator.h"
#include "Core/WFCGrid.h"


DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Edge Constraint - Time (ms)"), STAT_WFCEdgeConstraintTime, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Edge Constraint - Checks"), STAT_WFCEdgeConstraintNumChecks, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Edge Constraint - Bans"), STAT_WFCEdgeConstraintNumBans, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Edge Constraint - Mapping Checks"), STAT_WFCEdgeConstraintMappingChecks, STATGROUP_WFC);


UWFCEdgeConstraint::UWFCEdgeConstraint()
{
}

void UWFCEdgeConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	AssetModel = Cast<UWFCAssetModel>(Model);
	if (!AssetModel)
	{
		UE_LOG(LogWFC, Error, TEXT("WFCEdgeConstraint requires a UWFCAssetModel and tile set."));
		return;
	}

	bDidApplyInitialConstraint = false;
	AdjacentCellDirsToCheck.Reset();

	SCOPE_LOG_TIME_FUNC();
	SET_DWORD_STAT(STAT_WFCEdgeConstraintMappingChecks, 0);

	const FWFCGridDirection NumDirections = Grid->GetNumDirections();

	const UWFCTileSet* TileSet = AssetModel->GetAssetTileSet();
	for (int32 AssetIdxA = 0; AssetIdxA < TileSet->TileAssets.Num(); ++AssetIdxA)
	{
		const UWFCTileAsset* TileAssetA = TileSet->TileAssets[AssetIdxA];
		if (!TileAssetA)
		{
			continue;
		}

		// for each def in tile asset a...
		const int32 NumDefsA = TileAssetA->GetNumTileDefs();
		for (int32 DefIdxA = 0; DefIdxA < NumDefsA; ++DefIdxA)
		{
			TArray<FWFCTileId> TileAIds = AssetModel->GetTileIdsForAssetAndDef(TileAssetA, DefIdxA);

			// for each direction out of tile a...
			for (FWFCGridDirection LocalOutDirectionA = 0; LocalOutDirectionA < NumDirections; ++LocalOutDirectionA)
			{
				if (TileAssetA->IsInteriorEdge(DefIdxA, LocalOutDirectionA))
				{
					// interior edge of a large tile, add the exact neighbors
					AddInteriorAllowedTiles(TileAssetA, DefIdxA, LocalOutDirectionA, TileAIds);
					continue;
				}

				const FWFCGridDirection LocalInDirectionA = Grid->GetOppositeDirection(LocalOutDirectionA);
				FGameplayTag EdgeTypeA = TileAssetA->GetTileDefEdgeType(DefIdxA, LocalOutDirectionA);

				// TODO (bsayre): put this logic in the grid somewhere?
				// don't attempt to try to match up XY and Z directions, since only yaw rotation is supported
				const bool bIsLateralDirection = LocalOutDirectionA < 4;
				// either use 0..3 for lateral directions, or 4..5 for vertical
				const FWFCGridDirection MinBDir = bIsLateralDirection ? 0 : 4;
				const FWFCGridDirection MaxBDir = bIsLateralDirection ? 3 : 5;

				// for each other asset (without re-checking same pairs, but including reflectivity)...
				for (int32 AssetIdxB = AssetIdxA; AssetIdxB < TileSet->TileAssets.Num(); ++AssetIdxB)
				{
					const UWFCTileAsset* TileAssetB = TileSet->TileAssets[AssetIdxB];
					if (!TileAssetB)
					{
						continue;
					}

					// for each def in tile asset b...
					const int32 NumDefsB = TileAssetB->GetNumTileDefs();
					for (int32 DefIdxB = 0; DefIdxB < NumDefsB; ++DefIdxB)
					{
						TArray<FWFCTileId> TileBIds = AssetModel->GetTileIdsForAssetAndDef(TileAssetB, DefIdxB);

						// for each (compatible) direction out of tile b...
						for (FWFCGridDirection LocalOutDirectionB = MinBDir; LocalOutDirectionB <= MaxBDir; ++LocalOutDirectionB)
						{
							// TODO: early out if the tile can never be rotated such that this direction would line up with A

							if (TileAssetB->IsInteriorEdge(DefIdxB, LocalOutDirectionB))
							{
								continue;
							}

							FGameplayTag EdgeTypeB = TileAssetB->GetTileDefEdgeType(DefIdxB, LocalOutDirectionB);
							if (!AreEdgesCompatible(EdgeTypeA, EdgeTypeB))
							{
								continue;
							}

							// edges match, add A<-B and B<-A for all rotation variations with the same local directions
							AddMatchingEdgeAllowedTiles(TileAIds, LocalInDirectionA, TileBIds, LocalOutDirectionB);
						}
					}
				}
			}
		}
	}

	LogDebugInfo();
}

bool UWFCEdgeConstraint::AreEdgesCompatible(const FGameplayTag& EdgeA, const FGameplayTag& EdgeB) const
{
	INC_DWORD_STAT(STAT_WFCEdgeConstraintMappingChecks);
	return EdgeA.IsValid() && EdgeB.IsValid() && EdgeA == EdgeB;
}

void UWFCEdgeConstraint::AddInteriorAllowedTiles(const UWFCTileAsset* TileAsset, int32 TileDefIndex,
                                                 FWFCGridDirection Direction, const TArray<FWFCTileId>& TileDefIds)
{
	check(TileAsset != nullptr);
	check(AssetModel != nullptr);

	const FWFCGridDirection InDirection = Grid->GetOppositeDirection(Direction);
	// it's an interior edge, don't check all other tiles, just map it to the other tile in this asset
	const int32 NeighborDefIdxA = TileAsset->GetTileDefInDirection(TileDefIndex, Direction);
	// if the direction was interior, neighbor must be valid
	check(NeighborDefIdxA != INDEX_NONE);

	TArray<FWFCTileId> NeighborTileIds = AssetModel->GetTileIdsForAssetAndDef(TileAsset, NeighborDefIdxA);

	for (const FWFCTileId& TileIdA : TileDefIds)
	{
		const FWFCModelAssetTile& AssetTileA = Model->GetTileRef<FWFCModelAssetTile>(TileIdA);
		const FWFCGridDirection WorldInDirectionA = Grid->RotateDirection(InDirection, AssetTileA.Rotation);
		const FWFCGridDirection WorldInDirectionB = Grid->GetOppositeDirection(WorldInDirectionA);

		// for each rotation of neighbor tile def a...
		for (const FWFCTileId& TileIdB : NeighborTileIds)
		{
			const FWFCModelAssetTile& AssetTileB = Model->GetTileRef<FWFCModelAssetTile>(TileIdB);
			if (AssetTileA.Rotation == AssetTileB.Rotation)
			{
				// add both A<-B and B<-A
				AddAllowedTileForDirection(TileIdA, WorldInDirectionA, TileIdB);
				AddAllowedTileForDirection(TileIdB, WorldInDirectionB, TileIdA);
			}
		}
	}
}

void UWFCEdgeConstraint::AddMatchingEdgeAllowedTiles(const TArray<FWFCTileId>& TileAIds, FWFCGridDirection InDirectionA,
                                                     const TArray<FWFCTileId>& TileBIds, FWFCGridDirection OutDirectionB)
{
	for (const FWFCTileId TileIdA : TileAIds)
	{
		const FWFCModelAssetTile& AssetTileA = Model->GetTileRef<FWFCModelAssetTile>(TileIdA);
		const FWFCGridDirection WorldInDirectionA = Grid->RotateDirection(InDirectionA, AssetTileA.Rotation);
		const FWFCGridDirection WorldInDirectionB = Grid->GetOppositeDirection(WorldInDirectionA);

		for (const FWFCTileId TileIdB : TileBIds)
		{
			// ensure the local direction being used for tile b is the same (edge still matches)
			// meaning a matching rotation with tile a, or a direction where rotation doesn't matter (+/-Z)
			const FWFCModelAssetTile& AssetTileB = Model->GetTileRef<FWFCModelAssetTile>(TileIdB);
			if (Grid->InverseRotateDirection(WorldInDirectionA, AssetTileB.Rotation) != OutDirectionB)
			{
				continue;
			}

			// add both A<-B and B<-A
			AddAllowedTileForDirection(TileIdA, WorldInDirectionA, TileIdB);
			AddAllowedTileForDirection(TileIdB, WorldInDirectionB, TileIdA);
		}
	}
}
