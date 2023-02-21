// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset3D.h"

#include "Core/Grids/WFCGrid3D.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "WFCEditor"


UWFCTileAsset3D::UWFCTileAsset3D()
	: Dimensions(FIntVector(1, 1, 1)),
	  bAllowRotation(true)
{
}

FWFCTileDef3D UWFCTileAsset3D::GetTileDefByLocation(FIntVector Location, int32& Index) const
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
	return FWFCTileDef3D();
}

FWFCTileDef3D UWFCTileAsset3D::GetTileDefByIndex(int32 Index) const
{
	return TileDefs.IsValidIndex(Index) ? TileDefs[Index] : FWFCTileDef3D();
}

void UWFCTileAsset3D::GetAllowedRotations(TArray<int32>& OutRotations) const
{
	if (bAllowRotation)
	{
		// only yaw rotations are supported
		OutRotations = {0, 1, 2, 3};
	}
	else
	{
		OutRotations = {0};
	}
}

FGameplayTag UWFCTileAsset3D::GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].EdgeTypes[static_cast<EWFCTile3DEdge>(Direction)];
}

TSubclassOf<AActor> UWFCTileAsset3D::GetTileDefActorClass(int32 TileDefIndex) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	return TileDefs[TileDefIndex].ActorClass;
}

bool UWFCTileAsset3D::IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	check(TileDefs.IsValidIndex(TileDefIndex));
	const FIntVector DirectionVector = UWFCGrid3D::GetDirectionVectorStatic(Direction);
	const FIntVector TileLocation = TileDefs[TileDefIndex].Location;
	const FIntVector DeltaLocation = TileLocation + DirectionVector;
	return DeltaLocation.X >= 0 && DeltaLocation.X < Dimensions.X &&
		DeltaLocation.Y >= 0 && DeltaLocation.Y < Dimensions.Y &&
		DeltaLocation.Z >= 0 && DeltaLocation.Z < Dimensions.Z;
}

#if WITH_EDITOR
EDataValidationResult UWFCTileAsset3D::IsDataValid(FDataValidationContext& Context)
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (TileDefs.Num() != Dimensions.X * Dimensions.Y * Dimensions.Z)
	{
		Context.AddError(LOCTEXT("IncorrectTileDefCount", "Tile def count does not match tile dimensions"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif


UWFCTileSet3DInfo::UWFCTileSet3DInfo()
	: TileSize(FVector(100.f, 100.f, 100.f))
{
}

#undef LOCTEXT_NAMESPACE
