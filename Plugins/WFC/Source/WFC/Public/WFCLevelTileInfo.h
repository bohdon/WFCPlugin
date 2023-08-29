// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WFCTileAsset3D.h"
#include "GameFramework/Actor.h"
#include "WFCLevelTileInfo.generated.h"

class UWFCPreviewSplineComponent;
class AWFCLevelTileEdge;
class UWFCTileAsset3D;
class UWFCTileSet;


/**
 * An actor used to auto generate 3d tiles from a level.
 */
UCLASS(HideCategories = (Spline, SelectedPoints, Navigation, Rendering, Physics, Networking, LOD, TextureStreaming, RayTracing))
class WFC_API AWFCLevelTileInfo : public AActor
{
	GENERATED_BODY()

private:
	/** Spline preview for representing the tile without having to load it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	UWFCPreviewSplineComponent* PreviewSpline;

public:
	AWFCLevelTileInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFCTileInfo")
	UWFCTileSet3DInfo* TileSetInfo;

	/** The tile data asset representing this level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFCTileInfo")
	UWFCTileAsset3D* TileAsset;

	/** The actor class to spawn for this tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFCTileInfo")
	TSubclassOf<AActor> TileActorClass;

	/** If true, automatically update the tile asset when this actor's level is saved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFCTileInfo")
	bool bAutoSaveTileAsset;

	/** Save the preview spline points in the tile asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFCTileInfo")
	bool bSaveSplinePreview;

	/** The dimensions of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"), Category = "WFCTileInfo")
	FIntVector Dimensions;

	/** The default edge type to use for sides of the tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"), Category = "WFCTileInfo")
	FGameplayTag DefaultSideEdgeType;

	/** The default edge type to use for tops of the tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"), Category = "WFCTileInfo")
	FGameplayTag DefaultTopEdgeType;

	/** The default edge type to use for bottoms of the tile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"), Category = "WFCTileInfo")
	FGameplayTag DefaultBottomEdgeType;

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

	/** Return the size of a single cell in world units. */
	UFUNCTION(BlueprintPure, Category="WFCTileInfo")
	FVector GetCellSize() const;

	/** Return the size of the full large tile in world units. */
	UFUNCTION(BlueprintPure, Category="WFCTileInfo")
	FVector GetLargeTileSize() const;

	/** Convert a world location to a tile location */
	UFUNCTION(BlueprintPure, Category="WFCTileInfo")
	FIntVector WorldToTileLocation(FVector WorldLocation) const;

	/** Find all edges in the level for this tile */
	UFUNCTION(BlueprintCallable, Category="WFCTileInfo")
	virtual void FindEdgeActors();

	/** Update the tile asset with settings from this actor. */
	UFUNCTION(BlueprintCallable, Category="WFCTileInfo")
	virtual bool UpdateTileAsset();

	/** Return the socket types for each edge of a tile */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category="WFCTileInfo")
	virtual TMap<EWFCTile3DEdge, FGameplayTag> GetAllEdgeTypesForTile(FIntVector TileLocation) const;

	/** Return the socket types for an edge of a tile */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category="WFCTileInfo")
	virtual FGameplayTag GetEdgeTypeForTile(FIntVector TileLocation, FIntVector Direction) const;

	/** Return true if an edge is interior to the larger tile. */
	UFUNCTION(BlueprintPure, Category="WFCTileInfo")
	bool IsExteriorEdge(FIntVector TileLocation, FIntVector Direction) const;

	/** Update the preview spline to match the current tile dimensions. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category="WFCTileInfo")
	void SetSplinePreviewToTileDimensions();

protected:
	UPROPERTY(Transient, VisibleAnywhere, Category="WFCTileInfo")
	TArray<TWeakObjectPtr<AWFCLevelTileEdge>> Edges;

#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
#endif
};
