// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WFCTilePreviewData.generated.h"


/**
 * Base class for preview data for displaying WFC tile assets without needing to fully load them. 
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew)
class WFC_API UWFCTilePreviewData : public UObject
{
	GENERATED_BODY()

public:
	/** Splines points for drawing a wireframe representation of the tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> SplinePoints;
};
