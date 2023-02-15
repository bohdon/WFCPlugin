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
	/** Return the generator that owns this constraint */
	FORCEINLINE UWFCGenerator* GetGenerator() const { return Generator; }

	/** Initialize the constraint for a generator */
	virtual void Initialize(UWFCGenerator* InGenerator);

	/** Reset the constraint to its initialized state */
	virtual void Reset();

	/** Called when a cell's tile candidates have changed */
	virtual void NotifyCellChanged(FWFCCellIndex CellIndex, bool bHasSelection);

	/**
	 * Update the constraint
	 * @return True if the constraint made any changes, false otherwise.
	 */
	virtual bool Next();

protected:
	UPROPERTY(Transient)
	UWFCGenerator* Generator;
};


/**
 * Defines the settings necessary for setting up and using a constraint.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class WFC_API UWFCConstraintConfig : public UObject
{
	GENERATED_BODY()

public:
	/** Return the constraint class to use for this config */
	virtual TSubclassOf<UWFCConstraint> GetConstraintClass() const;

	/** Configure a constraint */
	virtual void Configure(UWFCConstraint* Constraint) const;
};
