﻿// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCCountConstraint.h"

#include "WFCAssetModel.h"
#include "WFCModule.h"
#include "WFCTileAsset.h"
#include "Core/WFCGenerator.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Count Constraint - Mappings"), STAT_WFCCountConstraintMappings, STATGROUP_WFC);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Count Constraint - Time (ms)"), STAT_WFCCountConstraintTime, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Count Constraint - Bans"), STAT_WFCCountConstraintNumBans, STATGROUP_WFC);


// UWFCCountConstraint
// -------------------

void UWFCCountConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	SET_DWORD_STAT(STAT_WFCCountConstraintMappings, 0);
	SET_FLOAT_STAT(STAT_WFCCountConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCCountConstraintNumBans, 0);
}

void UWFCCountConstraint::Reset()
{
	Super::Reset();

	TileGroupCurrentCounts.Reset(TileGroupMaxCounts.Num());
	TileGroupCurrentCounts.SetNum(TileGroupMaxCounts.Num());
	TileGroupsToBan.Reset();
	BannedGroups.Reset();

	SET_FLOAT_STAT(STAT_WFCCountConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCCountConstraintNumBans, 0);
}

void UWFCCountConstraint::AddTileGroupMaxCountMapping(const TArray<FWFCTileId>& TileIds, int32 MaxCount)
{
	if (MaxCount <= 0)
	{
		UE_LOG(LogWFC, Warning, TEXT("MaxCount must be > 0 for a count constraint: %s"),
		       *GetNameSafe(GetOuter()));
		return;
	}

	// add tile group and max count
	const int32 GroupId = TileGroupMaxCounts.Add(FWFCCountConstraintTileGroup(TileIds, MaxCount));
	TileGroupCurrentCounts.SetNum(TileGroupMaxCounts.Num());

	// cache tile id -> group id mappings
	for (const FWFCTileId& TileId : TileIds)
	{
		TileIdsToGroups.Add(TileId, GroupId);
	}

	UE_LOG(LogWFC, VeryVerbose, TEXT("UWFCCountConstraint: Setting Max Count of %d for %d tile(s)"),
	       MaxCount, TileIds.Num());

	SET_DWORD_STAT(STAT_WFCCountConstraintMappings, TileGroupMaxCounts.Num());
}

void UWFCCountConstraint::NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection)
{
	if (bHasSelection)
	{
		const FWFCCell& Cell = Generator->GetCell(CellIndex);
		const FWFCTileId TileId = Cell.GetSelectedTileId();
		const int32 TileGroupIndex = TileIdsToGroups.Contains(TileId) ? TileIdsToGroups[TileId] : INDEX_NONE;
		if (TileGroupIndex != INDEX_NONE && !BannedGroups.Contains(TileGroupIndex))
		{
			const FWFCCountConstraintTileGroup& TileGroup = TileGroupMaxCounts[TileGroupIndex];
			TileGroupCurrentCounts[TileGroupIndex] += 1;
			if (TileGroupCurrentCounts[TileGroupIndex] >= TileGroup.MaxCount)
			{
				UE_LOG(LogWFC, VeryVerbose, TEXT("Tile %d selection reached Max Count of %d for group %d, banning next update."),
				       TileId, TileGroup.MaxCount, TileGroupIndex);
				TileGroupsToBan.AddUnique(TileGroupIndex);
			}
		}
	}
}

bool UWFCCountConstraint::Next()
{
	STAT(const double StartTime = FPlatformTime::Seconds());
	SET_FLOAT_STAT(STAT_WFCCountConstraintTime, 0);
	SET_DWORD_STAT(STAT_WFCCountConstraintNumBans, 0);

	bool bDidMakeChanges = false;

	if (TileGroupsToBan.Num() > 0)
	{
		// accumulate all tile ids from all groups to ban
		TArray<FWFCTileId> TileIdsToBan;
		for (const int32 TileGroupIndex : TileGroupsToBan)
		{
			TileIdsToBan.Append(TileGroupMaxCounts[TileGroupIndex].TileIds);
			BannedGroups.AddUnique(TileGroupIndex);
		}

		UE_LOG(LogWFC, Verbose, TEXT("Banning %d tile ids(s) after reaching max count"), TileIdsToBan.Num());

		// remove from all unselected cells
		for (FWFCCellIndex CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
		{
			FWFCCell& Cell = Generator->GetCell(CellIndex);
			if (!Cell.HasSelection())
			{
				INC_DWORD_STAT_BY(STAT_WFCCountConstraintNumBans, TileIdsToBan.Num());
				if (Generator->BanMultiple(CellIndex, TileIdsToBan))
				{
					// contradiction
					return true;
				}

				bDidMakeChanges = true;
			}
		}

		TileGroupsToBan.Reset();
	}

	INC_FLOAT_STAT_BY(STAT_WFCCountConstraintTime, (FPlatformTime::Seconds() - StartTime) * 1000);
	return bDidMakeChanges;
}


// UWFCTagCountConstraint
// ----------------------

int32 UWFCTagCountConstraint::GetTileMaxCount(const UWFCTileAsset* TileAsset) const
{
	if (!TileAsset)
	{
		return 0;
	}
	const FWFCTileTagMaxCount* MaxCountRule = MaxCounts.FindByPredicate([TileAsset](const FWFCTileTagMaxCount& MaxCountRule)
	{
		return TileAsset->OwnedTags.HasTag(MaxCountRule.Tag);
	});

	if (MaxCountRule)
	{
		UE_LOG(LogWFC, VeryVerbose, TEXT("Tile '%s' matches tag '%s', using Max Count: '%d'"),
		       *TileAsset->GetName(), *MaxCountRule->Tag.ToString(), MaxCountRule->MaxCount);

		return MaxCountRule->MaxCount;
	}
	return 0;
}

void UWFCTagCountConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	const UWFCAssetModel* AssetModel = Cast<UWFCAssetModel>(Model);
	if (!AssetModel)
	{
		UE_LOG(LogWFC, Error, TEXT("%s requires a UWFCAssetModel: %s"), *GetClass()->GetName(), *GetNameSafe(GetOuter()));
		return;
	}

	TArray<UWFCTileAsset*> TileAssets;
	AssetModel->GetAllTileAssets(TileAssets);

	for (const UWFCTileAsset* TileAsset : TileAssets)
	{
		const int32 TileMaxCount = GetTileMaxCount(TileAsset);
		if (TileMaxCount > 0)
		{
			const TArray<FWFCTileId> IdArray = AssetModel->GetTileIdsForAsset(TileAsset);

			// only apply the max count limitation to the tile at 0,0,0 within the asset,
			// so that large tiles don't count against it multiple times.
			TArray<FWFCTileId> OriginTileIds = IdArray.FilterByPredicate([AssetModel](const int32& TileId)
			{
				const FWFCModelAssetTile* AssetTile = AssetModel->GetTile<FWFCModelAssetTile>(TileId);
				check(AssetTile != nullptr);
				return AssetTile->TileDefIndex == 0;
			});

			if (OriginTileIds.Num() > 0)
			{
				AddTileGroupMaxCountMapping(OriginTileIds, TileMaxCount);
			}
		}
	}

	UE_LOG(LogWFC, Verbose, TEXT("UWFCTileAssetCountConstraintConfig configured %d max count mappings"), TileGroupMaxCounts.Num());
}
