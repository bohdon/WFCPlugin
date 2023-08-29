// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WFCTileAsset.h"
#include "WFCTileAsset3D.generated.h"


class UWFCTilePreviewData;
/** The edge types for a 3D tile */
UENUM(BlueprintType)
enum class EWFCTile3DEdge : uint8
{
	XPos UMETA(DisplayName = "+X"),
	YPos UMETA(DisplayName = "+Y"),
	XNeg UMETA(DisplayName = "-X"),
	YNeg UMETA(DisplayName = "-Y"),
	ZPos UMETA(DisplayName = "+Z"),
	ZNeg UMETA(DisplayName = "-Z"),
	MAX UMETA(Hidden),
};


/**
 * Definition of a 3D tile unit.
 */
USTRUCT(BlueprintType)
struct FWFCTileDef3D
{
	GENERATED_BODY()

	FWFCTileDef3D()
		: Location(FIntVector::ZeroValue)
	{
		EdgeTypes = {
			{EWFCTile3DEdge::XPos, FGameplayTag::EmptyTag},
			{EWFCTile3DEdge::YPos, FGameplayTag::EmptyTag},
			{EWFCTile3DEdge::XNeg, FGameplayTag::EmptyTag},
			{EWFCTile3DEdge::YNeg, FGameplayTag::EmptyTag},
			{EWFCTile3DEdge::ZPos, FGameplayTag::EmptyTag},
			{EWFCTile3DEdge::ZNeg, FGameplayTag::EmptyTag},
		};
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Location;

	/** The actor to spawn for this tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass;

	/** The level to spawn for this tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> Level;

	/** The types for all edges of the tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"))
	TMap<EWFCTile3DEdge, FGameplayTag> EdgeTypes;

	/** Preview data for representing the tile without having to load it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
	TObjectPtr<UWFCTilePreviewData> PreviewData;

	bool operator==(const FWFCTileDef3D& Other) const
	{
		return Other.Location == Location && Other.ActorClass == ActorClass &&
			Other.Level == Level && Other.EdgeTypes.OrderIndependentCompareEqual(EdgeTypes) &&
			PreviewData == Other.PreviewData;
	}

	bool operator!=(const FWFCTileDef3D& Other) const
	{
		return !(operator==(Other));
	}
};


/**
 * A 3D tile with a socket type on each side for use with adjacency constraints.
 * References an actor to be spawned for each tile.
 */
UCLASS()
class WFC_API UWFCTileAsset3D : public UWFCTileAsset
{
	GENERATED_BODY()

public:
	UWFCTileAsset3D();

	/**
	 * The dimensions of this tile. Tile assets can contain multiple actual tiles
	 * which will have fixed adjacency rules setup for them.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Dimensions;

	/** If true allow this tile to generate rotated variations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowRotation;

	/** The individual tiles that make up this group of tiles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "Location"))
	TArray<FWFCTileDef3D> TileDefs;

	/** Return a tile def by location, as well as its index */
	UFUNCTION(BlueprintPure)
	FWFCTileDef3D GetTileDefByLocation(FIntVector Location, int32& Index) const;

	UFUNCTION(BlueprintPure)
	FWFCTileDef3D GetTileDefByIndex(int32 Index) const;

	virtual void GetAllowedRotations(TArray<int32>& OutRotations) const override;
	virtual int32 GetNumTileDefs() const override { return TileDefs.Num(); }
	virtual FGameplayTag GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const override;
	virtual int32 GetTileDefInDirection(int32 TileDefIndex, FWFCGridDirection Direction) const override;
	virtual TSubclassOf<AActor> GetTileDefActorClass(int32 TileDefIndex) const override;
	virtual bool IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const override;
	virtual const UWFCTilePreviewData* GetTileDefPreviewData(int32 TileDefIndex) const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) override;
#endif
};


/**
 * Shared tile set info for use with 3d tile assets.
 */
UCLASS(BlueprintType, Blueprintable)
class WFC_API UWFCTileSet3DInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UWFCTileSet3DInfo();

	/** The dimensions of a single tile within this tile set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TileSize;
};
