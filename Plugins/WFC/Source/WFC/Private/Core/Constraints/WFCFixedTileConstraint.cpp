// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCFixedTileConstraint.h"

#include "WFCModule.h"
#include "WFCAssetModel.h"
#include "Core/WFCGenerator.h"
#include "Core/Grids/WFCGrid3D.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Fixed Tile Constraint - Mappings"), STAT_WFCFixedTileConstraintMappings, STATGROUP_WFC);


void UWFCFixedTileConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);
	SET_DWORD_STAT(STAT_WFCFixedTileConstraintMappings, 0);
}

void UWFCFixedTileConstraint::Reset()
{
	Super::Reset();

	bDidApplyInitialConstraint = false;
}

void UWFCFixedTileConstraint::AddFixedTileMapping(FWFCCellIndex CellIndex, FWFCTileId TileId)
{
	INC_DWORD_STAT(STAT_WFCFixedTileConstraintMappings);
	FixedTileMappings.Add(FWFCFixedTileConstraintEntry(CellIndex, TileId));
}

bool UWFCFixedTileConstraint::Next()
{
	if (bDidApplyInitialConstraint)
	{
		// already applied the constraint
		return false;
	}

	bool bDidMakeChanges = false;

	// select the fixed tiles
	for (const FWFCFixedTileConstraintEntry& TileMapping : FixedTileMappings)
	{
		Generator->Select(TileMapping.CellIndex, TileMapping.TileId);
		bDidMakeChanges = true;
	}

	bDidApplyInitialConstraint = true;
	return bDidMakeChanges;
}


// 3D Fixed Tile Constraints
// -------------------------


void UWFCFixedTile3DConstraint::Initialize(UWFCGenerator* InGenerator)
{
	Super::Initialize(InGenerator);

	SCOPE_LOG_TIME(TEXT("UWFCFiledTile3DConstraint::Initialize"), nullptr);

	const UWFCGrid3D* Grid = GetGenerator()->GetGrid<UWFCGrid3D>();
	if (!Grid)
	{
		UE_LOG(LogWFC, Error, TEXT("UWFCFiledTile3DConstraint requires a UWFCGrid3D to be used: %s"),
		       *GetNameSafe(GetOuter()));
		return;
	}

	const UWFCAssetModel* Model = GetGenerator()->GetModel<UWFCAssetModel>();
	if (!Model)
	{
		UE_LOG(LogWFC, Error, TEXT("UWFCFiledTile3DConstraint requires a UWFCAssetModel to be used: %s"),
		       *GetNameSafe(GetOuter()));
		return;
	}

	for (const FWFCFixedTileConstraint3DEntry& FixedTile : FixedTiles)
	{
		if (!FixedTile.TileAsset)
		{
			// TODO: move to asset validation
			UE_LOG(LogWFC, Warning, TEXT("Found invalid fixed tile constraint in %s"), *GetNameSafe(GetOuter()));
			continue;
		}

		// get the cell index
		const FWFCCellIndex CellIndex = Grid->GetCellIndexForLocation(FixedTile.CellLocation);
		if (!GetGenerator()->IsValidCellIndex(CellIndex))
		{
			UE_LOG(LogWFC, Warning, TEXT("Found invalid fixed tile constraint in %s, location not valid: %s"),
			       *GetNameSafe(GetOuter()), *FixedTile.CellLocation.ToString());
			continue;
		}

		// get the tile id
		int32 TileDefIndex;
		FixedTile.TileAsset->GetTileDefByLocation(FIntVector(0, 0, 0), TileDefIndex);
		const FWFCTileId TileId = Model->GetTileIdForAssetTileDef(FixedTile.TileAsset, TileDefIndex, FixedTile.TileRotation);
		if (!GetGenerator()->IsValidTileId(TileId))
		{
			UE_LOG(LogWFC, Warning, TEXT("Found invalid fixed tile constraint in %s, tile asset and rotation not found: %s, %d"),
			       *GetNameSafe(GetOuter()), *FixedTile.TileAsset->GetName(), FixedTile.TileRotation);
			continue;
		}

		// add the constraint
		AddFixedTileMapping(CellIndex, TileId);
	}
}
