// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCModel.generated.h"


/**
 * A model which contains all possible tiles and tile set info.
 * The model is also responsible for generating tiles and tile ids from arbitrary input data.
 */
UCLASS(Abstract, BlueprintType)
class WFC_API UWFCModel : public UObject
{
	GENERATED_BODY()

public:
	/** Initialize this model with arbitrary tile data. */
	UFUNCTION(BlueprintCallable)
	void Initialize(const UObject* TileData);

	/** Generate all tiles and tile ids for this model. */
	UFUNCTION(BlueprintCallable)
	virtual void GenerateTiles();

	UFUNCTION(BlueprintPure)
	int32 GetNumTiles() const { return Tiles.Num(); }

	UFUNCTION(BlueprintPure)
	int32 GetMaxTileId() const { return Tiles.Num() - 1; }

	/** Add a tile and generate a new id for it */
	FWFCTileId AddTile(TSharedPtr<FWFCModelTile> Tile);

	/** Return a tile by id */
	FWFCModelTile* GetTile(FWFCTileId TileId) const;

	/** Return a tile by id cast to a type. */
	template <typename T>
	T* GetTile(FWFCTileId TileId) const
	{
		return static_cast<T*>(GetTile(TileId));
	}

	/** Return a tile reference by id cast to a type. */
	template <typename T>
	const T& GetTileRef(FWFCTileId TileId) const
	{
		T* TilePtr = GetTile<T>(TileId);
		check(TilePtr != nullptr);
		return *TilePtr;
	}

	/** Return all the tiles in this model. */
	const TArray<TSharedPtr<FWFCModelTile>>& GetTiles() const { return Tiles; }

	/** Return the tile data object cast to a type. */
	template <class T>
	const T* GetTileData() const
	{
		return Cast<T>(TileDataRef.Get());
	}

	/** Return the weight of a tile. */
	FORCEINLINE float GetTileWeightUnchecked(FWFCTileId TileId) const { return TileWeights[TileId]; }

	/** Return a debug string representing a tile id. */
	virtual FString GetTileDebugString(FWFCTileId TileId) const;

protected:
	/** Reference to the tile data that was used to generate tiles. Usually a WFCAsset. */
	UPROPERTY(Transient)
	TObjectPtr<const UObject> TileDataRef;

	/** All generated tiles. Array index is the same as the tile id. */
	TArray<TSharedPtr<FWFCModelTile>> Tiles;

	/** All tile weights, by tile id. */
	TArray<float> TileWeights;
};
