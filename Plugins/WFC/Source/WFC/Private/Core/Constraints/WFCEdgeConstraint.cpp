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

	bDidApplyInitialConstraint = false;
	AdjacentCellDirsToCheck.Reset();

	SCOPE_LOG_TIME_FUNC();
	SET_DWORD_STAT(STAT_WFCEdgeConstraintMappingChecks, 0);

	const UWFCAssetModel* AssetModel = Cast<UWFCAssetModel>(Model);
	if (!AssetModel)
	{
		UE_LOG(LogWFC, Error, TEXT("WFCEdgeConstraint requires a UWFCAssetModel and tile set."));
		return;
	}

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
			// for each direction out of tile a...
			for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
			{
				if (TileAssetA->IsInteriorEdge(DefIdxA, Direction))
				{
					// it's an interior edge, don't check all other tiles, just map it to the other tile in this asset
					// TODO: find the other tile and add to allowed tiles
					continue;
				}

				FGameplayTag EdgeTypeA = TileAssetA->GetTileDefEdgeType(DefIdxA, Direction);
				const FWFCGridDirection InvDirection = Grid->GetOppositeDirection(Direction);

				// for each other asset, without re-checking same pairs, but including reflectivity...
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
						if (TileAssetB->IsInteriorEdge(DefIdxB, InvDirection))
						{
							continue;
						}
						FGameplayTag EdgeTypeB = TileAssetB->GetTileDefEdgeType(DefIdxB, InvDirection);
						if (!AreEdgesCompatible(EdgeTypeA, EdgeTypeB))
						{
							continue;
						}

						// add allowed tile for this pairing, for each rotation of tile def a
						TArray<FWFCTileId> TileAIds = AssetModel->GetTileIdsForAssetAndDef(TileAssetA, DefIdxA);
						TArray<FWFCTileId> TileBIds = AssetModel->GetTileIdsForAssetAndDef(TileAssetB, DefIdxB);
						for (const FWFCTileId TileAId : TileAIds)
						{
							const FWFCModelAssetTile& AssetTileA = Model->GetTileRef<FWFCModelAssetTile>(TileAId);
							const FWFCGridDirection RotatedDirection = Grid->RotateDirection(Direction, AssetTileA.Rotation);
							const FWFCGridDirection RotatedInvDirection = Grid->RotateDirection(InvDirection, AssetTileA.Rotation);

							for (const FWFCTileId TileBId : TileBIds)
							{
								const FWFCModelAssetTile& AssetTileB = Model->GetTileRef<FWFCModelAssetTile>(TileBId);
								if (AssetTileA.Rotation != AssetTileB.Rotation)
								{
									continue;
								}

								// using 'incoming' direction, and add both A<-B and B<-A
								AddAllowedTileForDirection(TileAId, RotatedInvDirection, TileBId);
								AddAllowedTileForDirection(TileBId, RotatedDirection, TileAId);
							}
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
