// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCCountConstraint.h"

#include "WFCAssetModel.h"
#include "WFCModule.h"
#include "Core/WFCGenerator.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Count Constraint - Mappings"), STAT_WFCCountConstraintMappings, STATGROUP_WFC);
DECLARE_FLOAT_ACCUMULATOR_STAT(TEXT("Count Constraint - Time (ms)"), STAT_WFCCountConstraintTime, STATGROUP_WFC);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Count Constraint - Bans"), STAT_WFCCountConstraintNumBans, STATGROUP_WFC);


TSubclassOf<UWFCConstraint> UWFCCountConstraintConfig::GetConstraintClass() const
{
	return UWFCCountConstraint::StaticClass();
}


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

	TileGroupCounts.Reset(TileGroups.Num());
	TileGroupCounts.SetNum(TileGroups.Num());
	TileGroupsToBan.Reset();

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

	const int32 GroupId = TileGroups.Num();

	// add tile group and max count
	TileGroups.Add(FWFCCountConstraintTileGroup(TileIds, MaxCount));
	TileGroupCounts.SetNum(TileGroups.Num());

	// cache tile id -> group id mappings
	for (const FWFCTileId& TileId : TileIds)
	{
		if (TileIdGroups.Num() < TileId + 1)
		{
			TileIdGroups.SetNum(TileId + 1);
		}
		TileIdGroups[TileId] = GroupId;
	}

	SET_DWORD_STAT(STAT_WFCCountConstraintMappings, TileGroups.Num());
}

void UWFCCountConstraint::NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection)
{
	if (bHasSelection)
	{
		const FWFCCell& Cell = Generator->GetCell(CellIndex);
		const FWFCTileId TileId = Cell.GetSelectedTileId();
		const int32 TileGroupIndex = TileIdGroups.IsValidIndex(TileId) ? TileIdGroups[TileId] : INDEX_NONE;
		if (TileGroupIndex != INDEX_NONE)
		{
			const FWFCCountConstraintTileGroup& TileGroup = TileGroups[TileGroupIndex];
			TileGroupCounts[TileGroupIndex] += 1;
			if (TileGroupCounts[TileGroupIndex] >= TileGroup.MaxCount)
			{
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
			TileIdsToBan.Append(TileGroups[TileGroupIndex].TileIds);
		}

		UE_LOG(LogWFC, Verbose, TEXT("Banning %d tile(s) after reaching max count"), TileIdsToBan.Num());

		// remove from all unselected cells
		for (FWFCCellIndex CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
		{
			FWFCCell& Cell = Generator->GetCell(CellIndex);
			if (!Cell.HasSelection())
			{
				Generator->BanMultiple(CellIndex, TileIdsToBan);
				INC_DWORD_STAT_BY(STAT_WFCCountConstraintNumBans, TileIdsToBan.Num());
				bDidMakeChanges = true;
			}
		}

		TileGroupsToBan.Reset();
	}


	INC_FLOAT_STAT_BY(STAT_WFCCountConstraintTime, (FPlatformTime::Seconds() - StartTime) * 1000);
	return bDidMakeChanges;
}


// Tile Asset Count Constraint
// ---------------------------

void UWFCTileAssetCountConstraintConfig::Configure(UWFCConstraint* Constraint) const
{
	Super::Configure(Constraint);

	UWFCCountConstraint* CountConstraint = Cast<UWFCCountConstraint>(Constraint);

	const UWFCAssetModel* Model = Constraint->GetGenerator()->GetModel<UWFCAssetModel>();
	const UWFCTileSet* TileSet = Model ? Model->GetAssetTileSet() : nullptr;
	if (!Model || !TileSet)
	{
		UE_LOG(LogWFC, Error, TEXT("UWFCTileAssetCountConstraintConfig requires a UWFCAssetModel and UWFCTileSet to be used: %s"),
		       *GetNameSafe(GetOuter()));
		return;
	}

	for (const FWFCTileSetEntry& TileSetEntry : TileSet->Tiles)
	{
		const UWFCTileAsset* TileAsset = TileSetEntry.TileAsset;
		if (TileSetEntry.MaxCount > 0)
		{
			const FWFCTileIdArray IdArray = Model->GetTileIdsForAsset(TileAsset);
			if (IdArray.TileIds.Num() > 0)
			{
				CountConstraint->AddTileGroupMaxCountMapping(IdArray.TileIds, TileSetEntry.MaxCount);
			}
		}
	}
}
