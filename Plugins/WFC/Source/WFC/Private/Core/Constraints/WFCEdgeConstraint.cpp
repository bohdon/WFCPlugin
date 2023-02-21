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
	if (!AssetModel || !AssetModel->GetAssetTileSet())
	{
		UE_LOG(LogWFC, Error, TEXT("WFCEdgeConstraint requires a UWFCAssetModel and tile set."));
		return;
	}

	bDidApplyInitialConstraint = false;
	AdjacentCellDirsToCheck.Reset();

	SCOPE_LOG_TIME_FUNC();
	SET_DWORD_STAT(STAT_WFCEdgeConstraintMappingChecks, 0);

	InitializeFromTiles();

	LogDebugInfo();
}

bool UWFCEdgeConstraint::AreEdgesCompatible(const FGameplayTag& EdgeA, const FGameplayTag& EdgeB) const
{
	return EdgeA.IsValid() && EdgeB.IsValid() && EdgeA == EdgeB;
}

bool UWFCEdgeConstraint::AreTilesCompatible(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB,
                                            FWFCGridDirection Direction) const
{
	check(TileA.TileAsset.IsValid());
	check(TileB.TileAsset.IsValid());

	// the given direction represents 'incoming' direction into TileA
	const FWFCGridDirection OutDirection = Grid->GetOppositeDirection(Direction);
	const FWFCGridDirection LocalOutDirectionA = Grid->InverseRotateDirection(OutDirection, TileA.Rotation);

	if (TileA.TileAsset->IsInteriorEdge(TileA.TileDefIndex, LocalOutDirectionA))
	{
		// interior edge, only matching tile would be the exact neighbor of the same rotation
		return TileA.Rotation == TileB.Rotation &&
			TileA.TileAsset == TileB.TileAsset &&
			TileA.TileAsset->GetTileDefInDirection(TileA.TileDefIndex, LocalOutDirectionA) == TileB.TileDefIndex;
	}

	const FGameplayTag EdgeTypeA = TileA.TileAsset->GetTileDefEdgeType(TileA.TileDefIndex, LocalOutDirectionA);

	const FWFCGridDirection LocalOutDirectionB = Grid->InverseRotateDirection(Direction, TileB.Rotation);
	const FGameplayTag EdgeTypeB = TileB.TileAsset->GetTileDefEdgeType(TileB.TileDefIndex, LocalOutDirectionB);

	return AreEdgesCompatible(EdgeTypeA, EdgeTypeB);
}

void UWFCEdgeConstraint::InitializeFromTiles()
{
	const int32 NumDirections = Grid->GetNumDirections();

	// iterate over all distinct pairs of tiles, including reflectivity, comparing socket types for compatibility
	for (FWFCTileId TileIdA = 0; TileIdA <= Model->GetMaxTileId(); ++TileIdA)
	{
		const FWFCModelAssetTile& TileA = Model->GetTileRef<FWFCModelAssetTile>(TileIdA);

		// compare A <-> A for each direction
		for (FWFCGridDirection WorldInDirection = 0; WorldInDirection < NumDirections; ++WorldInDirection)
		{
			INC_DWORD_STAT(STAT_WFCEdgeConstraintMappingChecks);

			if (AreTilesCompatible(TileA, TileA, WorldInDirection))
			{
				AddAllowedTileForDirection(TileIdA, WorldInDirection, TileIdA);
			}
		}

		for (FWFCTileId TileIdB = TileIdA + 1; TileIdB <= Model->GetMaxTileId(); ++TileIdB)
		{
			const FWFCModelAssetTile& TileB = Model->GetTileRef<FWFCModelAssetTile>(TileIdB);

			// compare A <-> B for each direction
			for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
			{
				INC_DWORD_STAT(STAT_WFCEdgeConstraintMappingChecks);

				if (AreTilesCompatible(TileA, TileB, Direction))
				{
					AddAllowedTileForDirection(TileIdA, Direction, TileIdB);

					// add opposite directly as well
					const FWFCGridDirection OppositeDirection = Grid->GetOppositeDirection(Direction);
					AddAllowedTileForDirection(TileIdB, OppositeDirection, TileIdA);
				}
			}
		}
	}
}

void UWFCEdgeConstraint::InitializeFromAssets()
{
	struct FTileAssetDefEdge
	{
		FTileAssetDefEdge(const UWFCTileAsset* InTileAsset, int32 InDefIdx, FWFCGridDirection InDirection, const TArray<FWFCTileId>& InIds)
			: Asset(InTileAsset), Index(InDefIdx), Direction(InDirection), Ids(InIds)
		{
			bIsInterior = Asset->IsInteriorEdge(Index, Direction);
			EdgeType = Asset->GetTileDefEdgeType(Index, Direction);
		}

		const UWFCTileAsset* Asset;
		int32 Index;
		FWFCGridDirection Direction;
		// cached array of tile ids for this asset def
		TArray<FWFCTileId> Ids;
		bool bIsInterior;
		FGameplayTag EdgeType;
	};

	const FWFCGridDirection NumDirections = Grid->GetNumDirections();
	const UWFCTileSet* TileSet = AssetModel->GetAssetTileSet();
	check(TileSet != nullptr);

	// build flat list of tile asset defs so they can be iterated without checking duplicate pairs
	TArray<FTileAssetDefEdge> TileEdges;
	// will usually be larger than this but just starting with initial reserve
	TileEdges.Reserve(TileSet->TileAssets.Num() * NumDirections);
	for (const UWFCTileAsset* TileAsset : TileSet->TileAssets)
	{
		if (!TileAsset)
		{
			continue;
		}
		for (int32 DefIdx = 0; DefIdx < TileAsset->GetNumTileDefs(); ++DefIdx)
		{
			TArray<int32> TileIds = AssetModel->GetTileIdsForAssetAndDef(TileAsset, DefIdx);

			for (FWFCGridDirection Direction = 0; Direction < NumDirections; ++Direction)
			{
				TileEdges.Emplace(TileAsset, DefIdx, Direction, TileIds);
			}
		}
	}

	// for each tile def edge...
	for (int32 IdxA = 0; IdxA < TileEdges.Num(); ++IdxA)
	{
		const FTileAssetDefEdge& TileEdgeA = TileEdges[IdxA];

		if (TileEdgeA.bIsInterior)
		{
			// interior edge of a large tile, add the exact neighbors
			AddInteriorAllowedTiles(TileEdgeA.Asset, TileEdgeA.Index, TileEdgeA.Direction, TileEdgeA.Ids);
			continue;
		}

		// TODO (bsayre): put this logic in the grid somewhere?
		// don't attempt to try to match up XY and Z directions, since only yaw rotation is supported
		const bool bIsLateralDirection = TileEdgeA.Direction < 4;

		// for each other tile def edge...
		for (int32 IdxB = IdxA; IdxB < TileEdges.Num(); ++IdxB)
		{
			const FTileAssetDefEdge& TileEdgeB = TileEdges[IdxB];

			// don't compare vertical and lateral directions, since only yaw rotation is supported
			if (bIsLateralDirection && TileEdgeB.Direction >= 4 ||
				!bIsLateralDirection && TileEdgeB.Direction < 4)
			{
				// either use 0..3 for lateral directions, or 4..5 for vertical
				continue;
			}

			// TODO: early out if the tile can never be rotated such that this direction would line up with A

			if (TileEdgeB.bIsInterior)
			{
				continue;
			}

			INC_DWORD_STAT(STAT_WFCEdgeConstraintMappingChecks);
			if (!AreEdgesCompatible(TileEdgeA.EdgeType, TileEdgeB.EdgeType))
			{
				continue;
			}

			// edges match, add A<-B and B<-A for all rotation variations with the same local directions
			AddMatchingEdgeAllowedTiles(TileEdgeA.Ids, TileEdgeA.Direction, TileEdgeB.Ids, TileEdgeB.Direction);
		}
	}
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

void UWFCEdgeConstraint::AddMatchingEdgeAllowedTiles(const TArray<FWFCTileId>& TileAIds, FWFCGridDirection OutDirectionA,
                                                     const TArray<FWFCTileId>& TileBIds, FWFCGridDirection OutDirectionB)
{
	for (const FWFCTileId TileIdA : TileAIds)
	{
		const FWFCModelAssetTile& AssetTileA = Model->GetTileRef<FWFCModelAssetTile>(TileIdA);
		const FWFCGridDirection WorldOutDirectionA = Grid->RotateDirection(OutDirectionA, AssetTileA.Rotation);
		const FWFCGridDirection WorldInDirectionA = Grid->GetOppositeDirection(WorldOutDirectionA);

		for (const FWFCTileId TileIdB : TileBIds)
		{
			const FWFCModelAssetTile& AssetTileB = Model->GetTileRef<FWFCModelAssetTile>(TileIdB);
			const FWFCGridDirection WorldOutDirectionB = Grid->RotateDirection(OutDirectionB, AssetTileB.Rotation);
			if (WorldInDirectionA != WorldOutDirectionB)
			{
				// mismatching world directions
				continue;
			}

			// add both A<-B and B<-A
			AddAllowedTileForDirection(TileIdA, WorldOutDirectionB, TileIdB);
			AddAllowedTileForDirection(TileIdB, WorldOutDirectionA, TileIdA);
		}
	}
}
