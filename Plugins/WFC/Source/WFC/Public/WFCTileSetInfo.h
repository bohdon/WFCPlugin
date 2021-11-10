// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WFCTileSetInfo.generated.h"


/**
 * Base class for any information that is associated with a set
 * of tiles and is relevant for how they will be selected.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class WFC_API UWFCTileSetInfo : public UObject
{
	GENERATED_BODY()
};
