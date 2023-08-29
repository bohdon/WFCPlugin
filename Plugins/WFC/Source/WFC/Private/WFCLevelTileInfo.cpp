// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCLevelTileInfo.h"

#include "EngineUtils.h"
#include "WFCLevelTileEdge.h"
#include "WFCPreviewSplineComponent.h"
#include "WFCTileAsset3D.h"
#include "WFCTilePreviewData.h"
#include "Core/Grids/WFCGrid3D.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ObjectSaveContext.h"


AWFCLevelTileInfo::AWFCLevelTileInfo()
	: bAutoSaveTileAsset(true),
	  bSaveSplinePreview(true),
	  Dimensions(FIntVector(1, 1, 1))
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	PreviewSpline = CreateOptionalDefaultSubobject<UWFCPreviewSplineComponent>(TEXT("PreviewSpline"));
	if (PreviewSpline)
	{
		PreviewSpline->SetupAttachment(RootComponent);
	}
}

void AWFCLevelTileInfo::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

#if WITH_EDITOR
	FindEdgeActors();

	// update data asset
	if (bAutoSaveTileAsset)
	{
		UpdateTileAsset();
	}
#endif
}

FVector AWFCLevelTileInfo::GetCellSize() const
{
	return TileSetInfo ? TileSetInfo->TileSize : FVector(100.f, 100.f, 100.f);
}

FVector AWFCLevelTileInfo::GetLargeTileSize() const
{
	return GetCellSize() * FVector(Dimensions);
}

FIntVector AWFCLevelTileInfo::WorldToTileLocation(FVector WorldLocation) const
{
	const FVector RelativeLocation = GetActorTransform().InverseTransformPosition(WorldLocation);
	const FVector TileLocation = RelativeLocation / GetCellSize();
	return FIntVector(TileLocation);
}

void AWFCLevelTileInfo::FindEdgeActors()
{
	Edges.Reset();
	for (TActorIterator<AWFCLevelTileEdge> It(GetWorld()); It; ++It)
	{
		if (AWFCLevelTileEdge* Edge = *It)
		{
			if (Edge->GetLevel() == GetLevel())
			{
				Edges.Add(Edge);
			}
		}
	}
}

bool AWFCLevelTileInfo::UpdateTileAsset()
{
	if (!TileAsset)
	{
		return false;
	}

	// ensure edges are up to date
	for (const TWeakObjectPtr<AWFCLevelTileEdge>& Edge : Edges)
	{
		Edge->UpdateTileLocationAndDirection();
	}

	const TSoftObjectPtr<UWorld> TileLevel = GetLevel()->GetWorld();

	// update asset dimensions
	if (TileAsset->Dimensions != Dimensions)
	{
		TileAsset->Dimensions = Dimensions;
		TileAsset->Modify();
	}

	// update tile defs
	bool bAreTileDefsDirty = false;
	const int32 NewNumTileDefs = Dimensions.X * Dimensions.Y * Dimensions.Z;
	if (TileAsset->TileDefs.Num() != NewNumTileDefs)
	{
		TileAsset->TileDefs.SetNum(NewNumTileDefs);
		bAreTileDefsDirty = true;
	}

	int32 Idx = 0;
	for (int32 Z = 0; Z < Dimensions.Z; ++Z)
	{
		for (int32 Y = 0; Y < Dimensions.Y; ++Y)
		{
			for (int32 X = 0; X < Dimensions.X; ++X)
			{
				FWFCTileDef3D& TileDef = TileAsset->TileDefs[Idx];

				// set location
				const FIntVector Location = FIntVector(X, Y, Z);
				if (TileDef.Location != Location)
				{
					TileDef.Location = Location;
					bAreTileDefsDirty = true;
				}

				// set actor class and level
				TSubclassOf<AActor> NewActorClass = Idx == 0 ? TileActorClass : nullptr;
				TSoftObjectPtr<UWorld> NewLevel = Idx == 0 ? TileLevel : nullptr;
				if (TileDef.ActorClass != NewActorClass || TileDef.Level != NewLevel)
				{
					TileDef.ActorClass = NewActorClass;
					TileDef.Level = NewLevel;
					bAreTileDefsDirty = true;
				}

				// update edge types
				TMap<EWFCTile3DEdge, FGameplayTag> NewEdgeTypes = GetAllEdgeTypesForTile(TileDef.Location);
				if (!TileDef.EdgeTypes.OrderIndependentCompareEqual(NewEdgeTypes))
				{
					TileDef.EdgeTypes = NewEdgeTypes;
					bAreTileDefsDirty = true;
				}

				// update preview data
				if (Idx == 0 && bSaveSplinePreview && PreviewSpline)
				{
					if (!TileDef.PreviewData)
					{
						TileDef.PreviewData = NewObject<UWFCTilePreviewData>(TileAsset, NAME_None, RF_Transactional);
						bAreTileDefsDirty = true;
					}

					const TArray<FVector> SplinePoints = PreviewSpline->GetSplinePoints();
					if (TileDef.PreviewData->SplinePoints != SplinePoints)
					{
						TileDef.PreviewData->SplinePoints = SplinePoints;
						bAreTileDefsDirty = true;
					}
				}

				++Idx;
			}
		}
	}

	if (bAreTileDefsDirty)
	{
		TileAsset->Modify();
	}

	return true;
}

TMap<EWFCTile3DEdge, FGameplayTag> AWFCLevelTileInfo::GetAllEdgeTypesForTile(FIntVector TileLocation) const
{
	TMap<EWFCTile3DEdge, FGameplayTag> Result;

	for (uint8 Direction = 0; Direction < static_cast<uint8>(EWFCTile3DEdge::MAX); ++Direction)
	{
		const FIntVector DirectionVector = UWFCGrid3D::GetDirectionVectorStatic(Direction);
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

void AWFCLevelTileInfo::SetSplinePreviewToTileDimensions()
{
	if (!PreviewSpline)
	{
		return;
	}

	const FVector LargeTileSize = GetLargeTileSize();
	const TArray<FVector> NewPoints = {
		FVector(0.f, 0.f, 0.f) * LargeTileSize,
		FVector(1.f, 0.f, 0.f) * LargeTileSize,
		FVector(1.f, 1.f, 0.f) * LargeTileSize,
		FVector(0.f, 1.f, 0.f) * LargeTileSize,
	};
	PreviewSpline->SetSplinePoints(NewPoints, ESplineCoordinateSpace::Local);
	PreviewSpline->SetAllSplinePointsType(ESplinePointType::Linear);
}

#if WITH_EDITOR
void AWFCLevelTileInfo::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	FindEdgeActors();

	// ensure edges are up to date
	for (const TWeakObjectPtr<AWFCLevelTileEdge>& Edge : Edges)
	{
		Edge->UpdateTileLocationAndDirection();
	}
}
#endif
