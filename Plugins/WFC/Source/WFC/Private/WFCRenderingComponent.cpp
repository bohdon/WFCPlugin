// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCRenderingComponent.h"

#include "WFCAsset.h"
#include "WFCGeneratorComponent.h"
#include "Core/WFCGenerator.h"
#include "Core/CellSelectors/WFCEntropyCellSelector.h"
#include "Core/Constraints/WFCEdgeConstraint.h"
#include "Core/Grids/WFCGrid2D.h"
#include "Core/Grids/WFCGrid3D.h"


FWFCDebugSceneProxy::FWFCDebugSceneProxy(const UPrimitiveComponent* InComponent)
	: FDebugRenderSceneProxy(InComponent)
{
	DrawType = EDrawType::WireMesh;
	ViewFlagIndex = FEngineShowFlags::FindIndexByName(TEXT("WFC"));
	ViewFlagName = FString(TEXT("WFC"));
	TextWithoutShadowDistance = 0.f;
}

SIZE_T FWFCDebugSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

FPrimitiveViewRelevance FWFCDebugSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = View->Family->EngineShowFlags.GetSingleFlag(ViewFlagIndex) && IsShown(View);
	Result.bDynamicRelevance = true;
	Result.bEditorNoDepthTestPrimitiveRelevance = true;
	return Result;
}


UWFCRenderingComponent::UWFCRenderingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWFCRenderingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// TODO: determine correct way to keep renderer up to date
	MarkRenderStateDirty();
}

#if UE_ENABLE_DEBUG_DRAWING
FDebugRenderSceneProxy* UWFCRenderingComponent::CreateDebugSceneProxy()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	FWFCDebugSceneProxy* DebugProxy = new FWFCDebugSceneProxy(this);

	const UWFCGeneratorComponent* GeneratorComp = GetGeneratorComponent();
	if (!GeneratorComp)
	{
		DebugProxy->Texts.Emplace(FString(TEXT("Missing WFCGeneratorComponent")), GetComponentLocation(), FLinearColor::Yellow);
		return DebugProxy;
	}

	const FWFCGeneratorDebugSettings& Settings = GeneratorComp->DebugSettings;

	const UWFCAsset* WFCAsset = GeneratorComp->WFCAsset;
	if (!WFCAsset)
	{
		DebugProxy->Texts.Emplace(FString(TEXT("Select a WFCAsset")), GetComponentLocation(), FLinearColor::White);
		return DebugProxy;
	}

	const FTransform GridTransform = GeneratorComp->GetComponentTransform();

	// draw grid
	FIntVector GridDimensions;
	FVector GridCellSize;
	GetGridDimensionsAndSize(GridDimensions, GridCellSize);

	const FVector GridMin = GridTransform.GetTranslation();
	const FVector GridMax = GridTransform.TransformPosition(FVector(GridDimensions) * GridCellSize);
	DebugProxy->Boxes.Emplace(FBox(GridMin, GridMax), GeneratorComp->EditorGridColor.ToFColor(true));

	if (GeneratorComp->IsInitialized())
	{
		const UWFCGenerator* Generator = GeneratorComp->GetGenerator();
		const UWFCGrid* Grid = Generator->GetGrid();
		const UWFCEntropyCellSelector* EntropySelector = Generator->GetCellSelector<UWFCEntropyCellSelector>();
		const UWFCEdgeConstraint* AdjacencyConstraint = Generator->GetConstraint<UWFCEdgeConstraint>();

		const FLinearColor OpenColor = FLinearColor(0.5f, 0.1f, 1.f);
		const FLinearColor CollapsedColor = FLinearColor(0.7f, 1.f, 0.7f);

		if (AdjacencyConstraint)
		{
			TMultiMap<FWFCCellIndex, FWFCTileId> BansToPropagateByCell;
			// draw all pending arc bans to propagate
			for (const FWFCCellIndexAndTileId& BansToPropagate : AdjacencyConstraint->GetBansToPropagate())
			{
				BansToPropagateByCell.Add(BansToPropagate.CellIndex, BansToPropagate.TileId);
			}

			TArray<FWFCCellIndex> BanCellsToPropagate;
			BansToPropagateByCell.GetKeys(BanCellsToPropagate);
			for (FWFCCellIndex CellIndex : BanCellsToPropagate)
			{
				// draw black sphere around the cell
				const FVector CellCenterA = GridTransform.TransformPosition(Grid->GetCellWorldLocation(CellIndex, true));
				DebugProxy->Spheres.Emplace(GridCellSize.GetMax(), CellCenterA, FColor::Black);

				// draw banned tile ids still to propagate above the cell
				TArray<FWFCTileId> TileIds;
				BansToPropagateByCell.MultiFind(CellIndex, TileIds);
				FString TileIdsStr = GetTileIdsDebugString(TileIds, Settings.MaxTileIdCount);
				const FVector CellTop = CellCenterA + FVector::UpVector * GridCellSize.GetMax();
				DebugProxy->Texts.Emplace(TileIdsStr, CellTop, FLinearColor::Black);
			}

			// draw arc cells visited during last update
			for (const FWFCCellIndexAndDirection& CellVisited : AdjacencyConstraint->GetVisitedDuringPropagation())
			{
				FWFCCellIndex CellIndexB = Grid->GetCellIndexInDirection(CellVisited.CellIndex, CellVisited.Direction);
				if (!Grid->IsValidCellIndex(CellIndexB))
				{
					continue;
				}

				const FVector CellCenterA = GridTransform.TransformPosition(Grid->GetCellWorldLocation(CellVisited.CellIndex, true));
				const FVector CellCenterB = GridTransform.TransformPosition(Grid->GetCellWorldLocation(CellIndexB, true));
				DebugProxy->ArrowLines.Emplace(CellCenterA, CellCenterB, FColor::Red);
			}
		}

		// draw cells
		for (int32 CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
		{
			const FWFCCell& Cell = Generator->GetCell(CellIndex);
			const bool bWasAffectedThisUpdate = Generator->GetCellsAffectedThisUpdate().Contains(CellIndex);

			TArray<FString> CellTextLines;
			if (Settings.bShowCellCoordinates)
			{
				CellTextLines.Add(Grid->GetCellName(CellIndex));
			}
			const FVector CellCenter = GridTransform.TransformPosition(Grid->GetCellWorldLocation(CellIndex, true));
			FVector CellHalfSize = Settings.DebugCellScale * GridCellSize * 0.5f;

			FLinearColor Color;
			FLinearColor TextColor;
			if (Cell.HasSelection())
			{
				// was the collapse from selection or from constraints?
				const bool bWasFromSelection = Cell.CollapsePhase == EWFCGeneratorStepPhase::Selection;
				CellHalfSize *= 0.1f;
				Color = bWasFromSelection ? FLinearColor::Green : FLinearColor::Blue;
				TextColor = bWasFromSelection ? FLinearColor::Green : FLinearColor::Blue;
				if (Settings.bShowSelectedTileIds || (Settings.bHighlightUpdatedCells && bWasAffectedThisUpdate))
				{
					CellTextLines.Add(Generator->GetTileDebugString(Cell.GetSelectedTileId()));
				}
			}
			else if (Cell.HasNoCandidates())
			{
				CellHalfSize *= 0.01f;
				Color = FLinearColor::Red;
				TextColor = FLinearColor::Red;
				if (Settings.bShowCandidates)
				{
					CellTextLines.Add(GetTileIdsDebugString(TArray<FWFCTileId>(), Settings.MaxTileIdCount));
				}
			}
			else
			{
				const int32 NumCandidates = Cell.TileCandidates.Num();
				const float Openness = FMath::Pow(static_cast<float>(NumCandidates) / Generator->GetNumTiles(), 1.f);
				CellHalfSize *= Openness * 0.8f + 0.2f;
				Color = FLinearColor::LerpUsingHSV(CollapsedColor, OpenColor, Openness);
				TextColor = Color * 1.5f;
				if (Settings.bShowCandidates)
				{
					CellTextLines.Add(GetTileIdsDebugString(Cell.TileCandidates, Settings.MaxTileIdCount));
				}
				if (Settings.bShowEntropy && EntropySelector)
				{
					const float Entropy = EntropySelector->GetCellEntropy(CellIndex);
					if (Entropy <= Settings.DebugEntropyThreshold)
					{
						CellTextLines.Add(FString::Printf(TEXT("e%0.2f"), Entropy));
					}
				}
			}

			if (Settings.bHighlightUpdatedCells && bWasAffectedThisUpdate)
			{
				DebugProxy->Spheres.Emplace(GridCellSize.X * 0.15f, CellCenter, FColor::Green);
				TextColor = FLinearColor(FColor::Green);
			}

			const FVector CellMin = CellCenter - CellHalfSize;
			const FVector CellMax = CellCenter + CellHalfSize;
			DebugProxy->Boxes.Emplace(FBox(CellMin, CellMax), Color.ToFColor(false));
			if (!CellTextLines.IsEmpty())
			{
				DebugProxy->Texts.Emplace(FString::Join(CellTextLines, TEXT("\n")), CellCenter, TextColor);
			}
		}
	}

	return DebugProxy;
}
#endif // UE_ENABLE_DEBUG_DRAWING

FBoxSphereBounds UWFCRenderingComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FIntVector Dimensions;
	FVector CellSize;
	GetGridDimensionsAndSize(Dimensions, CellSize);

	FBoxSphereBounds NewBounds;
	NewBounds.Origin = LocalToWorld.GetLocation();
	NewBounds.BoxExtent = FVector(Dimensions) * CellSize;
	NewBounds.SphereRadius = NewBounds.BoxExtent.Size();
	return NewBounds;
}

UWFCGeneratorComponent* UWFCRenderingComponent::GetGeneratorComponent() const
{
	if (GetOwner())
	{
		return GetOwner()->FindComponentByClass<UWFCGeneratorComponent>();
	}
	return nullptr;
}

void UWFCRenderingComponent::GetGridDimensionsAndSize(FIntVector& OutDimensions, FVector& OutCellSize) const
{
	OutDimensions = FIntVector::ZeroValue;
	OutCellSize = FVector::ZeroVector;

	const UWFCGeneratorComponent* GeneratorComponent = GetGeneratorComponent();
	if (!GeneratorComponent)
	{
		return;
	}

	if (GeneratorComponent->IsInitialized())
	{
		const UWFCGenerator* Generator = GeneratorComponent->GetGenerator();
		if (const UWFCGrid3D* Grid3D = Generator->GetGrid<UWFCGrid3D>())
		{
			OutDimensions = Grid3D->Dimensions;
			OutCellSize = Grid3D->CellSize;
		}
		else if (const UWFCGrid2D* Grid2D = Generator->GetGrid<UWFCGrid2D>())
		{
			OutDimensions = FIntVector(Grid2D->Dimensions.X, Grid2D->Dimensions.Y, 1);
			OutCellSize = FVector(Grid2D->CellSize.X, Grid2D->CellSize.Y, 1);
		}
	}
	else if (GeneratorComponent->WFCAsset)
	{
		if (const UWFCGrid3DConfig* Grid3DConfig = Cast<UWFCGrid3DConfig>(GeneratorComponent->WFCAsset->GridConfig))
		{
			OutDimensions = Grid3DConfig->Dimensions;
			OutCellSize = Grid3DConfig->CellSize;
		}
		else if (const UWFCGrid2DConfig* Grid2DConfig = Cast<UWFCGrid2DConfig>(GeneratorComponent->WFCAsset->GridConfig))
		{
			OutDimensions = FIntVector(Grid2DConfig->Dimensions.X, Grid2DConfig->Dimensions.Y, 1);
			OutCellSize = FVector(Grid2DConfig->CellSize.X, Grid2DConfig->CellSize.Y, 1);
		}
	}
}

FString UWFCRenderingComponent::GetTileIdsDebugString(const TArray<FWFCTileId>& TileIds, int32 MaxCount) const
{
	if (TileIds.IsEmpty())
	{
		return FString(TEXT("(none)"));
	}

	if (TileIds.Num() > MaxCount)
	{
		return FString::Printf(TEXT("%d tile(s)"), TileIds.Num());
	}

	// list every tile id
	TArray<FString> TileIdStrs;
	for (const FWFCTileId& TileId : TileIds)
	{
		TileIdStrs.Add(FString::FromInt(TileId));
	}
	return FString::Join(TileIdStrs, TEXT(", "));
}
