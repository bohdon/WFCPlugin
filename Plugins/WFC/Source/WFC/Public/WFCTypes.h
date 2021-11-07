// Copyright Bohdon Sayre. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WFCTypes.generated.h"


/**
 * Abstractly represents a single tile of any size or shape.
 */
USTRUCT(BlueprintType)
struct FWFCTile
{
	GENERATED_BODY()

	FWFCTile()
		: Id(0)
	{
	}

	/** The unique id of this tile */
	UPROPERTY(VisibleAnywhere)
	int32 Id;
};


/**
 * An abstract direction that informs how to locate one tile from another.
 */
UCLASS(Abstract, BlueprintType, DefaultToInstanced, EditInlineNew)
class WFC_API UWFCDirection : public UObject
{
	GENERATED_BODY()

public:
	/** Return the string representation of the direction */
	virtual FString ToString() const { return FString(); }
};

/**
 * A two dimensional direction
 */
UCLASS()
class WFC_API UWFCDirection2D : public UWFCDirection
{
	GENERATED_BODY()

public:
	/** The delta location implied by this direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Delta;
};


/**
 * A three dimensional direction
 */
UCLASS()
class WFC_API UWFCDirection3D : public UWFCDirection
{
	GENERATED_BODY()

public:
	/** The delta location implied by this direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Delta;
};
