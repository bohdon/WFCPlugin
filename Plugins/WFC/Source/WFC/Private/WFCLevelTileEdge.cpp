// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCLevelTileEdge.h"

#include "WFCLevelTileInfo.h"
#include "WFCStatics.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"


AWFCLevelTileEdge::AWFCLevelTileEdge()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

#if WITH_EDITORONLY_DATA
	bRunConstructionScriptOnDrag = false;

	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));

	if (!IsRunningCommandlet())
	{
		if (ArrowComponent)
		{
			ArrowComponent->ArrowColor = FColor::White;
			ArrowComponent->ArrowLength = 40.f;
			ArrowComponent->bTreatAsASprite = true;
			ArrowComponent->SetupAttachment(GetRootComponent());
			ArrowComponent->bIsScreenSizeScaled = true;
		}
	}
#endif // WITH_EDITORONLY_DATA
}

AWFCLevelTileInfo* AWFCLevelTileEdge::GetOwningLevelTileInfo() const
{
	// TODO: find on spawn or register
	return Cast<AWFCLevelTileInfo>(UGameplayStatics::GetActorOfClass(this, AWFCLevelTileInfo::StaticClass()));
}

void AWFCLevelTileEdge::SnapToGrid()
{
	if (const AWFCLevelTileInfo* TileInfo = GetOwningLevelTileInfo())
	{
		// snap to 90 degrees
		const FRotator Rotation = GetActorRotation();
		const FRotator NewRotation = FRotator(FMath::GridSnap(Rotation.Pitch, 90.f), FMath::GridSnap(Rotation.Yaw, 90.f), 0.f);
		SetActorRotation(NewRotation);

		const FVector TileSize = TileInfo->GetTileSize();

		// edges are positioned in the center of the YZ plane of any face, pointing towards +X
		const FVector CenterOffset = NewRotation.RotateVector(FVector(0.f, 1.f, 1.f)) * TileSize * 0.5f;
		const FVector NewLocation = UWFCStatics::SnapToNonUniformGrid(GetActorLocation() + CenterOffset, TileSize) - CenterOffset;
		SetActorLocation(NewLocation);
	}
}

void AWFCLevelTileEdge::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	SnapToGrid();
}

void AWFCLevelTileEdge::UpdateTileLocationAndDirection()
{
	const FVector Forward = GetActorForwardVector();
	EdgeDirection = FIntVector(FMath::RoundToInt(Forward.X), FMath::RoundToInt(Forward.Y), FMath::RoundToInt(Forward.Z));

	if (const AWFCLevelTileInfo* TileInfo = GetOwningLevelTileInfo())
	{
		const FVector TileCenterOffset = FVector(EdgeDirection) * TileInfo->GetTileSize() * 0.5f;
		const FVector TileCenter = GetActorLocation() - TileCenterOffset;
		TileLocation = TileInfo->WorldToTileLocation(TileCenter);
	}
}

#if WITH_EDITOR
void AWFCLevelTileEdge::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	UpdateTileLocationAndDirection();
}
#endif
