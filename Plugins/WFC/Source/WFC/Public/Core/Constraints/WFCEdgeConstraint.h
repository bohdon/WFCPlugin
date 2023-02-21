// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WFCArcConsistencyConstraint.h"
#include "WFCEdgeConstraint.generated.h"


/**
 * Constrains tiles such that only those with matching edge tags can be placed next to each other.
 */
UCLASS(DisplayName = "Adjacency Constraint")
class WFC_API UWFCEdgeConstraint : public UWFCArcConsistencyConstraint
{
	GENERATED_BODY()

public:
	UWFCEdgeConstraint();

	virtual void Initialize(UWFCGenerator* InGenerator) override;

	/** Return true if two edges are allowed to be next to each other. */
	virtual bool AreEdgesCompatible(const FGameplayTag& EdgeA, const FGameplayTag& EdgeB) const;
};
