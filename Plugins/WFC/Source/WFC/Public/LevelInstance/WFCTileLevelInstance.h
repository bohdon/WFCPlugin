// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DynamicLevelInstance.h"
#include "WFCTileActorInterface.h"
#include "WFCTileAsset.h"
#include "Core/WFCTypes.h"
#include "WFCTileLevelInstance.generated.h"

class UWFCGeneratorComponent;
class UWFCPreviewSplineComponent;
class UWFCTileAsset3D;


UCLASS()
class WFC_API AWFCTileLevelInstance : public ADynamicLevelInstance,
                                      public IWFCTileActorInterface
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	UWFCPreviewSplineComponent* PreviewSpline;

public:
	AWFCTileLevelInstance();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bHidePreviewWhenLoaded;

	/** IWFCTileActorInterface */
	virtual void SetGeneratorComp(UWFCGeneratorComponent* NewGeneratorComp) override;
	virtual void SetTileAndCell(const FWFCModelTile* NewTile, FWFCCellIndex NewCellIndex) override;

	const FWFCModelAssetTile& GetModelTile() const { return ModelTile; }

	/** Refresh the preview for this tile. */
	UFUNCTION(BlueprintNativeEvent)
	void UpdatePreview();

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<UWFCGeneratorComponent> GeneratorComp;

	/** The model tile that was selected. */
	UPROPERTY(Transient, BlueprintReadOnly)
	FWFCModelAssetTile ModelTile;

	/** The cell where this tile is placed. */
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 CellIndex;

	virtual void OnLevelShown() override;
	virtual void OnLevelHidden() override;
};
