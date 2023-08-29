// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTileAsset.h"
#include "Components/SplineComponent.h"
#include "WFCPreviewSplineComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WFC_API UWFCPreviewSplineComponent : public USplineComponent
{
	GENERATED_BODY()

public:
	UWFCPreviewSplineComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bUseRandomColorFromTile;

	virtual TArray<ESplinePointType::Type> GetEnabledSplinePointTypes() const override;

	/** Set the spline point type for all points. */
	UFUNCTION(BlueprintCallable)
	void SetAllSplinePointsType(ESplinePointType::Type Type, bool bUpdateSpline = true);

	UFUNCTION(BlueprintCallable)
	void SetSplinePointsFromTile(FWFCModelAssetTile AssetTile);

	/** Return all the splint point locations in local space. */
	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSplinePoints() const;
};
