// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCTileAsset.h"


FString FWFCModelAssetTile::ToString() const
{
	return FString::Printf(TEXT("%s:%s-r%d[%d]"), *Super::ToString(), *GetNameSafe(TileAsset.Get()), Rotation, TileDefIndex);
}

UWFCTileAsset::UWFCTileAsset()
{
}

int32 UWFCTileAsset::GetNumRotations() const
{
	TArray<int32> Rotations;
	GetAllowedRotations(Rotations);
	return Rotations.Num();
}

void UWFCTileAsset::GetAllowedRotations(TArray<int32>& OutRotations) const
{
	OutRotations = {0};
}

int32 UWFCTileAsset::GetNumTileDefs() const
{
	return 0;
}

FGameplayTag UWFCTileAsset::GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	return FGameplayTag::EmptyTag;
}

TSubclassOf<AActor> UWFCTileAsset::GetTileDefActorClass(int32 TileDefIndex) const
{
	return nullptr;
}

bool UWFCTileAsset::IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const
{
	return false;
}
