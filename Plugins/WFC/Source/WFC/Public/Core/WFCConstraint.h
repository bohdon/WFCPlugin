// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCConstraint.generated.h"

class UWFCGenerator;


/**
 * A set of rules that can be applied to how tiles are allowed to be placed.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class WFC_API UWFCConstraint : public UObject
{
	GENERATED_BODY()

public:
	/** Initialize the constraint for a generator */
	virtual void Initialize(UWFCGenerator* InGenerator);

	/** Called when a cell's tile candidates have changed */
	virtual void NotifyCellChanged(FWFCCellIndex CellIndex);

	/**
	 * Update the constraint
	 * @return True if the constraint applied a change that resulted in cell selection, false otherwise.
	 */
	virtual bool Next();

protected:
	UPROPERTY(Transient)
	UWFCGenerator* Generator;
};
