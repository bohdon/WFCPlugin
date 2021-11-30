// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCLevelTileInfo.h"

#include "EngineUtils.h"
#include "WFCLevelTileEdge.h"
#include "WFCTileAsset3D.h"
#include "Core/Grids/WFCGrid3D.h"
#include "Kismet/GameplayStatics.h"


AWFCLevelTileInfo::AWFCLevelTileInfo()
	: bAutoSaveTileAsset(true),
	  Dimensions(FIntVector(1, 1, 1))
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AWFCLevelTileInfo::PreSave(const ITargetPlatform* TargetPlatform)
{
	Super::PreSave(TargetPlatform);

#if WITH_EDITOR
	FindEdgeActors();

	// update data asset
	if (bAutoSaveTileAsset)
	{
		UpdateTileAsset();
	}
#endif
}

FVector AWFCLevelTileInfo::GetTileSize() const
{
	return TileSetInfo ? TileSetInfo->TileSize : FVector(100.f, 100.f, 100.f);
}

FIntVector AWFCLevelTileInfo::WorldToTileLocation(FVector WorldLocation) const
{
	const FVector RelativeLocation = GetActorTransform().InverseTransformPosition(WorldLocation);
	const FVector TileLocation = RelativeLocation / GetTileSize();
	return FIntVector(TileLocation);
}

void AWFCLevelTileInfo::FindEdgeActors()
{
	Edges.Reset();
	for (TActorIterator<AWFCLevelTileEdge> It(GetWorld()); It; ++It)
	{
		if (AWFCLevelTileEdge* Edge = *It)
		{
			Edges.Add(Edge);
		}
	}
}

bool AWFCLevelTileInfo::UpdateTileAsset()
{
	if (!TileAsset)
	{
		return false;
	}

	TSoftObjectPtr<UWorld> TileLevel = GetLevel()->GetWorld();

	// update asset dimensions
	if (TileAsset->Dimensions != Dimensions)
	{
		TileAsset->Dimensions = Dimensions;
		TileAsset->Modify();
	}

	// update tile defs
	TArray<FWFCTileDef3D> NewTileDefs;
	for (int32 Z = 0; Z < Dimensions.Z; ++Z)
	{
		for (int32 Y = 0; Y < Dimensions.Y; ++Y)
		{
			for (int32 X = 0; X < Dimensions.X; ++X)
			{
				FWFCTileDef3D TileDef;
				TileDef.Location = FIntVector(X, Y, Z);
				TileDef.Level = TileDef.Location == FIntVector::ZeroValue ? TileLevel : nullptr;
				TileDef.EdgeTypes = GetAllEdgeTypesForTile(TileDef.Location);

				NewTileDefs.Add(TileDef);
			}
		}
	}

	if (TileAsset->TileDefs != NewTileDefs)
	{
		TileAsset->TileDefs = NewTileDefs;
		TileAsset->Modify();
	}

	return true;
}

TMap<EWFCTile3DEdge, FGameplayTag> AWFCLevelTileInfo::GetAllEdgeTypesForTile(FIntVector TileLocation) const
{
	TMap<EWFCTile3DEdge, FGameplayTag> Result;

	for (uint8 Direction = 0; Direction < static_cast<uint8>(EWFCTile3DEdge::MAX); ++Direction)
	{
		const FIntVector DirectionVector = UWFCGrid3D::GetDirectionVector(Direction);
		const FGameplayTag EdgeType = GetEdgeTypeForTile(TileLocation, DirectionVector);
		EWFCTile3DEdge Edge = static_cast<EWFCTile3DEdge>(Direction);

		Result.Add(Edge, EdgeType);
	}
	return Result;
}

FGameplayTag AWFCLevelTileInfo::GetEdgeTypeForTile(FIntVector TileLocation, FIntVector Direction) const
{
	for (const TWeakObjectPtr<AWFCLevelTileEdge>& Edge : Edges)
	{
		if (Edge.IsValid() && Edge->TileLocation == TileLocation && Edge->EdgeDirection == Direction)
		{
			return Edge->EdgeType;
		}
	}

	// return default edge types for exterior edges
	if (IsExteriorEdge(TileLocation, Direction))
	{
		if (Direction.Z > 0)
		{
			return DefaultTopEdgeType;
		}
		if (Direction.Z < 0)
		{
			return DefaultBottomEdgeType;
		}
		return DefaultSideEdgeType;
	}

	return FGameplayTag::EmptyTag;
}

bool AWFCLevelTileInfo::IsExteriorEdge(FIntVector TileLocation, FIntVector Direction) const
{
	// get tile in the direction of the edge
	const FIntVector ForwardTileCenter = TileLocation + Direction;
	return ForwardTileCenter.X < 0 || ForwardTileCenter.X >= Dimensions.X ||
		ForwardTileCenter.Y < 0 || ForwardTileCenter.Y >= Dimensions.Y ||
		ForwardTileCenter.Z < 0 || ForwardTileCenter.Z >= Dimensions.Z;
}
