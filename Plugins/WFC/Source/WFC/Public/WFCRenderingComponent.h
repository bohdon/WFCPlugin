// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
	
	UPROPERTY(EditDefaultsOnly)
	bool bShowCellCoordinates;
	
	UPROPERTY(EditDefaultsOnly)
	bool bShowNumCandidates;
	
	UPROPERTY(EditDefaultsOnly)
	bool bShowEntropy;

	/** Only show entropy when it is below this threshold. */
	UPROPERTY(EditDefaultsOnly, Meta = (EditCondition = "bShowEntropy"))
	float EntropyThreshold;

	UPROPERTY(EditDefaultsOnly)
	bool bShowSelectedTileIds;

	UPROPERTY(EditDefaultsOnly)
	bool bHighlightUpdatedCells;

	/** Scale applied to cell boxes in addition to dynamic scaling. */
	UPROPERTY(EditDefaultsOnly)
	FVector CellScale;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if UE_ENABLE_DEBUG_DRAWING
	virtual FDebugRenderSceneProxy* CreateDebugSceneProxy() override;
#endif // UE_ENABLE_DEBUG_DRAWING

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	UWFCGeneratorComponent* GetGeneratorComponent() const;
	void GetGridDimensionsAndSize(FIntVector& OutDimensions, FVector& OutCellSize) const;
};
