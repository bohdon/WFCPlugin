// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WFCTestingActor.generated.h"

class UWFCGeneratorComponent;
class UWFCRenderingComponent;


/**
 * Actor for running a WFC generator and providing debug drawing and input.
 */
UCLASS()
class WFC_API AWFCTestingActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWFCGeneratorComponent> WFCGenerator;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWFCRenderingComponent> WFCRendering;
#endif

public:
	AWFCTestingActor(const FObjectInitializer& ObjectInitializer);
};
