// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCTypes.h"
#include "UObject/Interface.h"
#include "WFCTileActorInterface.generated.h"

class UWFCGeneratorComponent;
struct FWFCModelTile;


UINTERFACE(Meta = (CannotImplementInterfaceInBlueprint))
class UWFCTileActorInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface for an actor spawned for a WFC tile that can receive info about which was selected.
 */
class WFC_API IWFCTileActorInterface
{
	GENERATED_BODY()

public:
	/**
	 * Set the generator component used to select the tile.
	 * Currently also used to acquire the grid and place the tile in it's cell.
	 */
	virtual void SetGeneratorComp(UWFCGeneratorComponent* NewGeneratorComp) = 0;

	/** Set the tile being represented by this actor. */
	virtual void SetTileAndCell(const FWFCModelTile* NewTile, FWFCCellIndex NewCellIndex) = 0;
};
