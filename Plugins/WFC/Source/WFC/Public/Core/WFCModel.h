﻿// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.h"
#include "UObject/Object.h"
#include "WFCModel.generated.h"

class UWFCGenerator;


/**
 * Contains all relevant data about a tile needed to construct
 * a final output from the model once tile ids have been selected.
 * Designed to be subclassed to add any additional or relevant info about each tile.
 */
USTRUCT(BlueprintType)
struct FWFCModelTile
{
	GENERATED_BODY()

	FWFCModelTile()
		: Id(INDEX_NONE)
	{
	}

	virtual ~FWFCModelTile()
	{
	}

	/** The id of this tile */
	UPROPERTY(BlueprintReadOnly)
	int32 Id;

	virtual FString ToString() const { return FString::Printf(TEXT("[%d]"), Id); }
};


// TODO: rename UWFCTileGenerator

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

	/** Configure a generator for use with this model's tile data */
	UFUNCTION(BlueprintCallable)
	virtual void ConfigureGenerator(UWFCGenerator* Generator);

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
		return (T*)GetTile(TileId);
	}

	/** Return the tile data object cast to a type. */
	template <class T>
	const T* GetTileData() const
	{
		return Cast<T>(TileDataRef.Get());
	}

protected:
	/** Weak reference to the tile data that was used to generate tiles. */
	UPROPERTY(Transient)
	TWeakObjectPtr<const UObject> TileDataRef;

	/** All generated tiles. Array index is the same as the tile id. */
	TArray<TSharedPtr<FWFCModelTile>> Tiles;

	/** Return a tile reference by id cast to a type. */
	template <typename T>
	const T& GetTileRef(FWFCTileId TileId) const
	{
		T* TilePtr = GetTile<T>(TileId);
		check(TilePtr != nullptr);
		return *TilePtr;
	}
};
