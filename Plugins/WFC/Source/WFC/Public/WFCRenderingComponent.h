// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCTypes.h"
#include "Debug/DebugDrawComponent.h"
#include "WFCRenderingComponent.generated.h"


class UWFCGeneratorComponent;

class FWFCDebugSceneProxy : public FDebugRenderSceneProxy
{
public:
	virtual SIZE_T GetTypeHash() const override;

	FWFCDebugSceneProxy(const UPrimitiveComponent* InComponent);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
};


/**
 * Displays debug drawing for a WFC generator.
 */
UCLASS(ClassGroup = Debug)
class WFC_API UWFCRenderingComponent : public UDebugDrawComponent
{
	GENERATED_BODY()

protected:
	UWFCRenderingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if UE_ENABLE_DEBUG_DRAWING
	virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
#endif // UE_ENABLE_DEBUG_DRAWING

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	UWFCGeneratorComponent* GetGeneratorComponent() const;
	void GetGridDimensionsAndSize(FIntVector& OutDimensions, FVector& OutCellSize) const;

	FString GetTileIdsDebugString(const TArray<FWFCTileId>& TileIds, int32 MaxCount = 10) const;
};
