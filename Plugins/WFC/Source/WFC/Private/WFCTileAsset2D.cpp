// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset2D.h"

#include "Core/Grids/WFCGrid2D.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "WFCEditor"


UWFCTileAsset2D::UWFCTileAsset2D()
	: Dimensions(FIntPoint(1, 1)),
	  bAllowRotation(true)
{
}

FWFCTileDef2D UWFCTileAsset2D::GetTileDefByLocation(FIntPoint Location, int32& Index) const
{
	Index = INDEX_NONE;
	for (int32 Idx = 0; Idx < TileDefs.Num(); ++Idx)
	{
		if (TileDefs[Idx].Location == Location)
		{
			Index = Idx;
			return TileDefs[Idx];
		}
	}
	return FWFCTileDef2D();
}

FWFCTileDef2D UWFCTileAsset2D::GetTileDefByIndex(int32 Index) const
{
	return TileDefs.IsValidIndex(Index) ? TileDefs[Index] : FWFCTileDef2D();
}

void UWFCTileAsset2D::GetAllowedRotations(TArray<int32>& OutRotations) const
{
	if (bAllowRotation)
	{
		OutRotations = {0, 1, 2, 3};
	}
	else
	{
		OutRotations = {0};
	}
}

FGameplayTag UWFCTileAsset2D::GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].EdgeTypes[static_cast<EWFCTile2DEdge>(Direction)];
}

TSubclassOf<AActor> UWFCTileAsset2D::GetTileDefActorClass(int32 TileDefIndex) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].ActorClass;
}

bool UWFCTileAsset2D::IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	const FIntPoint DirectionVector = UWFCGrid2D::GetDirectionVectorStatic(Direction);
	const FIntPoint TileLocation = TileDefs[TileDefIndex].Location;
	const FIntPoint DeltaLocation = TileLocation + DirectionVector;
	return DeltaLocation.X >= 0 && DeltaLocation.X < Dimensions.X &&
		DeltaLocation.Y >= 0 && DeltaLocation.Y < Dimensions.Y;
}


#if WITH_EDITOR
EDataValidationResult UWFCTileAsset2D::IsDataValid(FDataValidationContext& Context)
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (TileDefs.Num() != Dimensions.X * Dimensions.Y)
	{
		Context.AddError(LOCTEXT("IncorrectTileDefCount", "Tile def count does not match tile dimensions"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
