// Copyright Bohdon Sayre. All Rights Reserved.


#include "LevelInstance/WFCTileLevelInstance.h"

#include "WFCGeneratorComponent.h"
#include "WFCPreviewSplineComponent.h"
#include "WFCTileAsset3D.h"


AWFCTileLevelInstance::AWFCTileLevelInstance()
	: bHidePreviewWhenLoaded(false),
	  CellIndex(INDEX_NONE)
{
	PreviewSpline = CreateOptionalDefaultSubobject<UWFCPreviewSplineComponent>(TEXT("PreviewSpline"));
	if (PreviewSpline)
	{
		PreviewSpline->SetupAttachment(RootComponent);
		PreviewSpline->bUseRandomColorFromTile = true;
	}
}

void AWFCTileLevelInstance::SetGeneratorComp(UWFCGeneratorComponent* NewGeneratorComp)
{
	GeneratorComp = NewGeneratorComp;
}

void AWFCTileLevelInstance::SetTileAndCell(const FWFCModelTile* NewTile, FWFCCellIndex NewCellIndex)
{
	CellIndex = NewCellIndex;

	if (const FWFCModelAssetTile* AssetTilePtr = static_cast<const FWFCModelAssetTile*>(NewTile))
	{
		ModelTile = *AssetTilePtr;

		UpdatePreview();

		// retrieve tile level
		const UWFCTileAsset3D* TileAsset3D = Cast<UWFCTileAsset3D>(ModelTile.TileAsset.Get());
		if (TileAsset3D)
		{
			const FWFCTileDef3D TileDef = TileAsset3D->GetTileDefByIndex(ModelTile.TileDefIndex);

			SetLevelAsset(TileDef.Level);
		}
	}
}

void AWFCTileLevelInstance::UpdatePreview_Implementation()
{
	if (PreviewSpline)
	{
		PreviewSpline->SetSplinePointsFromTile(ModelTile);
	}
}

void AWFCTileLevelInstance::OnLevelShown()
{
	Super::OnLevelShown();

	if (bHidePreviewWhenLoaded && PreviewSpline)
	{
		// hide preview
		PreviewSpline->SetVisibility(false);
	}
}

void AWFCTileLevelInstance::OnLevelHidden()
{
	Super::OnLevelHidden();

	if (bHidePreviewWhenLoaded && PreviewSpline)
	{
		// show preview again
		PreviewSpline->SetVisibility(true);
	}
}
