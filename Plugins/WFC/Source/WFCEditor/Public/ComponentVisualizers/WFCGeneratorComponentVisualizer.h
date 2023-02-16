// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

class UWFCGridConfig;


/** Base class for clickable WFC generator visualization. */
struct HWFCGeneratorVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HWFCGeneratorVisProxy(const UActorComponent* InComponent)
		: HComponentVisProxy(InComponent, HPP_Wireframe)
	{
	}
};

/** Proxy for WFC grid cell */
struct HWFCCellProxy : public HWFCGeneratorVisProxy
{
	DECLARE_HIT_PROXY();

	HWFCCellProxy(const UActorComponent* InComponent, int32 CellIndex)
		: HWFCGeneratorVisProxy(InComponent)
		  , CellIndex(CellIndex)
	{
	}

	int32 CellIndex;
};


/**
 * Visualizes a WFC Generator component.
 */
class WFCEDITOR_API FWFCGeneratorComponentVisualizer : public FComponentVisualizer
{
public:
	FWFCGeneratorComponentVisualizer();

	~FWFCGeneratorComponentVisualizer();

	/** FComponentVisualizer */
	virtual void OnRegister() override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual bool ShowWhenSelected() override { return false; }
};
