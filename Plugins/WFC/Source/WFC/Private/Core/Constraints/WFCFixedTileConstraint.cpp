// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Constraints/WFCFixedTileConstraint.h"

#include "WFCModule.h"
#include "WFCAssetModel.h"
#include "Core/WFCGenerator.h"
#include "Core/Grids/WFCGrid3D.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Fixed Tile Constraint - Mappings"), STAT_WFCFixedTileConstraintMappings, STATGROUP_WFC);


TSubclassOf<UWFCConstraint> UWFCFixedTileConstraintConfig::GetConstraintClass() const
{
	return UWFCFixedTileConstraint::StaticClass();
}

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

void UWFCFixedTile3DConstraintConfig::Configure(UWFCConstraint* Constraint) const
{
	Super::Configure(Constraint);

	SCOPE_LOG_TIME(TEXT("UWFCFixedTile3DConstraintConfig::Configure"), nullptr);

	UWFCFixedTileConstraint* FixedConstraint = Cast<UWFCFixedTileConstraint>(Constraint);
	check(FixedConstraint != nullptr);

	const UWFCGrid3D* Grid = Constraint->GetGenerator()->GetGrid<UWFCGrid3D>();
	if (!Grid)
	{
		UE_LOG(LogWFC, Error, TEXT("Cannot configure UWFCFixedTile3DConstraintConfig, requires a UWFCGrid3D to be used: %s"),
		       *GetNameSafe(GetOuter()));
		return;
	}

	const UWFCAssetModel* Model = Constraint->GetGenerator()->GetModel<UWFCAssetModel>();
	if (!Model)
	{
		UE_LOG(LogWFC, Error, TEXT("Cannot configure UWFCFixedTile3DConstraintConfig, requires a UWFCAssetModel to be used: %s"),
		       *GetNameSafe(GetOuter()));
		return;
	}

	for (const FWFCFixedTileConstraint3DEntry& FixedTile : FixedTiles)
	{
		if (!FixedTile.TileAsset)
		{
			UE_LOG(LogWFC, Warning, TEXT("Found invalid fixed tile constraint in %s"),
			       *GetNameSafe(GetOuter()));
			continue;
		}

		// get the cell index
		const FWFCCellIndex CellIndex = Grid->GetCellIndexForLocation(FixedTile.CellLocation);
		if (!Constraint->GetGenerator()->IsValidCellIndex(CellIndex))
		{
			UE_LOG(LogWFC, Warning, TEXT("Found invalid fixed tile constraint in %s, location not valid: %s"),
			       *GetNameSafe(GetOuter()), *FixedTile.CellLocation.ToString());
			continue;
		}

		// get the tile id
		int32 TileDefIndex;
		FixedTile.TileAsset->GetTileDefByLocation(FIntVector(0, 0, 0), TileDefIndex);
		const FWFCTileId TileId = Model->GetTileIdForAssetTileDef(FixedTile.TileAsset, TileDefIndex, FixedTile.TileRotation);
		if (!Constraint->GetGenerator()->IsValidTileId(TileId))
		{
			UE_LOG(LogWFC, Warning, TEXT("Found invalid fixed tile constraint in %s, tile asset and rotation not found: %s, %d"),
			       *GetNameSafe(GetOuter()), *FixedTile.TileAsset->GetName(), FixedTile.TileRotation);
			continue;
		}

		// add the constraint
		FixedConstraint->AddFixedTileMapping(CellIndex, TileId);
	}
}
