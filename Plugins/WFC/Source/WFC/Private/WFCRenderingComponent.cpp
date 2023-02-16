// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCRenderingComponent.h"

#include "WFCAsset.h"
#include "WFCGeneratorComponent.h"
#include "Core/WFCGenerator.h"
#include "Core/CellSelectors/WFCEntropyCellSelector.h"
#include "Core/Constraints/WFCAdjacencyConstraint.h"
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
	: bShowCellCoordinates(false),
	  bShowNumCandidates(false),
	  bShowEntropy(false),
	  EntropyThreshold(2.f),
	  bShowSelectedTileIds(false),
	  bHighlightUpdatedCells(true),
	  CellScale(FVector(0.6f))
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWFCRenderingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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

	const UWFCGeneratorComponent* GeneratorComponent = GetGeneratorComponent();
	if (!GeneratorComponent)
	{
		DebugProxy->Texts.Emplace(FString(TEXT("Missing WFCGeneratorComponent")), GetComponentLocation(), FLinearColor::Yellow);
		return DebugProxy;
	}

	const UWFCAsset* WFCAsset = GeneratorComponent->WFCAsset;
	if (!WFCAsset)
	{
		DebugProxy->Texts.Emplace(FString(TEXT("Select a WFCAsset")), GetComponentLocation(), FLinearColor::White);
		return DebugProxy;
	}

	const FTransform GridTransform = GeneratorComponent->GetComponentTransform();

	// draw grid
	FIntVector GridDimensions;
	FVector GridCellSize;
	GetGridDimensionsAndSize(GridDimensions, GridCellSize);

	const FVector GridMin = GridTransform.GetTranslation();
	const FVector GridMax = GridTransform.TransformPosition(FVector(GridDimensions) * GridCellSize);
	DebugProxy->Boxes.Emplace(FBox(GridMin, GridMax), GeneratorComponent->EditorGridColor.ToFColor(true));

	if (GeneratorComponent->IsInitialized())
	{
		const UWFCGenerator* Generator = GeneratorComponent->GetGenerator();
		const UWFCGrid* Grid = Generator->GetGrid();
		const UWFCEntropyCellSelector* EntropySelector = Generator->GetCellSelector<UWFCEntropyCellSelector>();
		const UWFCAdjacencyConstraint* AdjacencyConstraint = Generator->GetConstraint<UWFCAdjacencyConstraint>();

		const FLinearColor OpenColor = FLinearColor(0.5f, 0.1f, 1.f);
		const FLinearColor CollapsedColor = FLinearColor(0.7f, 1.f, 0.7f);

		// draw all pending adjacency checks
		if (AdjacencyConstraint)
		{
			for (const FWFCCellIndexAndDirection& AdjacencyCellDir : AdjacencyConstraint->GetAdjacentCellDirsToCheck())
			{
				FWFCCellIndex CellIndexA = AdjacencyCellDir.CellIndex;
				FWFCCellIndex CellIndexB = Grid->GetCellIndexInDirection(AdjacencyCellDir.CellIndex, AdjacencyCellDir.Direction);
				if (!Grid->IsValidCellIndex(CellIndexB))
				{
					continue;
				}

				const FVector CellCenterA = GridTransform.TransformPosition(Grid->GetCellWorldLocation(CellIndexA, true));
				const FVector CellCenterB = GridTransform.TransformPosition(Grid->GetCellWorldLocation(CellIndexB, true));
				DebugProxy->ArrowLines.Emplace(CellCenterA, CellCenterB, FColor::White);
			}
		}

		// draw cells
		for (int32 CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
		{
			const FWFCCell& Cell = Generator->GetCell(CellIndex);
			const bool bWasAffectedThisUpdate = Generator->GetCellsAffectedThisUpdate().Contains(CellIndex);

			TArray<FString> CellTextLines;
			if (bShowCellCoordinates)
			{
				CellTextLines.Add(Grid->GetCellName(CellIndex));
			}
			const FVector CellCenter = GridTransform.TransformPosition(Grid->GetCellWorldLocation(CellIndex, true));
			FVector CellHalfSize = CellScale * GridCellSize * 0.5f;

			FLinearColor Color;
			FLinearColor TextColor;
			if (Cell.HasSelection())
			{
				CellHalfSize *= 0.01f;
				Color = FLinearColor::Blue;
				TextColor = FLinearColor::Blue;
				if (bShowSelectedTileIds || (bHighlightUpdatedCells && bWasAffectedThisUpdate))
				{
					CellTextLines.Add(Generator->GetTileDebugString(Cell.GetSelectedTileId()));
				}
			}
			else if (Cell.HasNoCandidates())
			{
				CellHalfSize *= 0.01f;
				Color = FLinearColor::Red;
				TextColor = FLinearColor::Red;
			}
			else
			{
				const int32 NumCandidates = Cell.TileCandidates.Num();
				const float Openness = FMath::Pow(static_cast<float>(NumCandidates) / Generator->GetNumTiles(), 1.f);
				CellHalfSize *= Openness * 0.8f + 0.2f;
				Color = FLinearColor::LerpUsingHSV(CollapsedColor, OpenColor, Openness);
				TextColor = Color * 1.5f;
				if (bShowNumCandidates)
				{
					CellTextLines.Add(FString::FromInt(NumCandidates));
				}
				if (bShowEntropy && EntropySelector)
				{
					const float Entropy = EntropySelector->GetCellEntropy(CellIndex);
					if (Entropy <= EntropyThreshold)
					{
						CellTextLines.Add(FString::Printf(TEXT("e%0.2f"), Entropy));
					}
				}
			}

			if (bHighlightUpdatedCells && bWasAffectedThisUpdate)
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

			// draw adjacency constraint updates
			if (AdjacencyConstraint && AdjacencyConstraint->GetAdjacenciesEnforcedThisUpdate().Contains(CellIndex))
			{
				// this cell's change caused a ban in another cell during the last update, draw the connection
				FWFCCellIndex AffectedCellIndex = AdjacencyConstraint->GetAdjacenciesEnforcedThisUpdate().FindRef(CellIndex);
				const FVector AffectedCellCenter = GridTransform.TransformPosition(Grid->GetCellWorldLocation(AffectedCellIndex, true));
				DebugProxy->ArrowLines.Emplace(CellCenter, AffectedCellCenter, FColor::Red);
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
