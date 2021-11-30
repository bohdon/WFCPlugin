// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "WFCLevelTileEdge.generated.h"

class AWFCLevelTileInfo;


/**
 * Represents a single edge of a tile in a tile level.
 */
UCLASS()
class WFC_API AWFCLevelTileEdge : public AActor
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	/** Arrow component to indicate forward direction of the edge */
	UPROPERTY()
	TObjectPtr<class UArrowComponent> ArrowComponent;
#endif

public:
	AWFCLevelTileEdge();

	/** The tile location of this edge. */
	UPROPERTY(VisibleAnywhere)
	FIntVector TileLocation;

	/** The direction of this edge within the tile. */
	UPROPERTY(VisibleAnywhere)
	FIntVector EdgeDirection;

	/** The type of this tile edge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"))
	FGameplayTag EdgeType;

	/** Return the level tile info for this edge's owning tile. */
	UFUNCTION(BlueprintPure)
	AWFCLevelTileInfo* GetOwningLevelTileInfo() const;

	/** Snap this edge to the grid */
	UFUNCTION(BlueprintCallable)
	virtual void SnapToGrid();

	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
#endif
};
