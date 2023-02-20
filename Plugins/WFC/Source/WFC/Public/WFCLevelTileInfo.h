// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WFCTileAsset3D.h"
#include "GameFramework/Actor.h"
#include "WFCLevelTileInfo.generated.h"

class AWFCLevelTileEdge;
class UWFCTileAsset3D;
class UWFCTileSet;


/**
 * An actor used to auto generate 3d tiles from a level.
 */
UCLASS()
class WFC_API AWFCLevelTileInfo : public AActor
{
	GENERATED_BODY()

public:
	AWFCLevelTileInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWFCTileSet3DInfo* TileSetInfo;

	/** The tile data asset representing this level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWFCTileAsset3D* TileAsset;

	/** The actor class to spawn for this tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> TileActorClass;

	/** If true, automatically update the tile asset when this actor's level is saved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoSaveTileAsset;

	/** The dimensions of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	FIntVector Dimensions;

	/** The default edge type to use for sides of the tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"))
	FGameplayTag DefaultSideEdgeType;

	/** The default edge type to use for tops of the tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"))
	FGameplayTag DefaultTopEdgeType;

	/** The default edge type to use for bottoms of the tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"))
	FGameplayTag DefaultBottomEdgeType;

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

	UFUNCTION(BlueprintPure)
	FVector GetTileSize() const;

	/** Convert a world location to a tile location */
	UFUNCTION(BlueprintPure)
	FIntVector WorldToTileLocation(FVector WorldLocation) const;

	/** Find all edges in the level for this tile */
	UFUNCTION(BlueprintCallable)
	virtual void FindEdgeActors();

	/** Update the tile asset with settings from this actor. */
	UFUNCTION(BlueprintCallable)
	virtual bool UpdateTileAsset();

	/** Return the socket types for each edge of a tile */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	virtual TMap<EWFCTile3DEdge, FGameplayTag> GetAllEdgeTypesForTile(FIntVector TileLocation) const;

	/** Return the socket types for an edge of a tile */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	virtual FGameplayTag GetEdgeTypeForTile(FIntVector TileLocation, FIntVector Direction) const;

	/** Return true if an edge is interior to the larger tile. */
	UFUNCTION(BlueprintPure)
	bool IsExteriorEdge(FIntVector TileLocation, FIntVector Direction) const;

protected:
	UPROPERTY(Transient, VisibleAnywhere)
	TArray<TWeakObjectPtr<AWFCLevelTileEdge>> Edges;

#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
#endif
};
