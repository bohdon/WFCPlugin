// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCConstraint.generated.h"

class UWFCGrid;
class UWFCModel;
class UWFCGenerator;


/**
 * A set of rules that can be applied to how tiles are allowed to be placed.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class WFC_API UWFCConstraint : public UObject
{
	GENERATED_BODY()

public:
	/** Return the generator that owns this constraint */
	UWFCGenerator* GetGenerator() const { return Generator; }

	/** Initialize the constraint for a generator */
	virtual void Initialize(UWFCGenerator* InGenerator);

	/** Reset the constraint to its initialized state */
	virtual void Reset();

	/** Called when a cell's tile candidates have changed */
	virtual void NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection);

	/** Called when a tile candidate has been banned from a cell. */
	virtual void NotifyCellBan(FWFCCellIndex CellIndex, FWFCTileId BannedTileId);

	/**
	 * Update the constraint
	 * @return True if the constraint made any changes, false otherwise.
	 */
	virtual bool Next();

	/** Log debug info about this constraint. */
	virtual void LogDebugInfo() const;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UWFCGenerator> Generator;

	/** Reference to the grid being used. */
	UPROPERTY(Transient)
	TObjectPtr<const UWFCGrid> Grid;

	/** Reference to the model being used. */
	UPROPERTY(Transient)
	TObjectPtr<const UWFCModel> Model;
};
