// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCPreviewSplineComponent.h"

#include "WFCStatics.h"
#include "WFCTilePreviewData.h"


UWFCPreviewSplineComponent::UWFCPreviewSplineComponent()
	: bUseRandomColorFromTile(false)
{
	SetClosedLoop(true, false);
	SetSplinePoints({
		                FVector(0.f, 0.f, 0.f),
		                FVector(1.f, 0.f, 0.f),
		                FVector(1.f, 1.f, 0.f),
		                FVector(0.f, 1.f, 0.f),
	                }, ESplineCoordinateSpace::Local);
	SetAllSplinePointsType(ESplinePointType::Linear);
}

TArray<ESplinePointType::Type> UWFCPreviewSplineComponent::GetEnabledSplinePointTypes() const
{
	return {
		ESplinePointType::Linear
	};
}

void UWFCPreviewSplineComponent::SetAllSplinePointsType(ESplinePointType::Type Type, bool bUpdateSpline)
{
	for (FInterpCurvePoint<FVector>& Point : SplineCurves.Position.Points)
	{
		Point.InterpMode = ConvertSplinePointTypeToInterpCurveMode(Type);
	}
	if (bUpdateSpline)
	{
		UpdateSpline();
	}
}

void UWFCPreviewSplineComponent::SetSplinePointsFromTile(FWFCModelAssetTile AssetTile)
{
	if (!AssetTile.TileAsset.IsValid())
	{
		return;
	}

	const UWFCTilePreviewData* PreviewData = AssetTile.TileAsset->GetTileDefPreviewData(AssetTile.TileDefIndex);
	if (!PreviewData)
	{
		return;
	}

	SetSplinePoints(PreviewData->SplinePoints, ESplineCoordinateSpace::Local);

	// ensure all points are linear
	SetAllSplinePointsType(ESplinePointType::Linear);

#if WITH_EDITORONLY_DATA
	if (bUseRandomColorFromTile)
	{
		EditorUnselectedSplineSegmentColor = UWFCStatics::GetRandomDebugColor(AssetTile.Id);
	}
#endif
}

TArray<FVector> UWFCPreviewSplineComponent::GetSplinePoints() const
{
	TArray<FVector> Result;
	for (const FInterpCurvePoint<FVector>& Point : GetSplinePointsPosition().Points)
	{
		Result.Add(Point.OutVal);
	}
	return Result;
}
