// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/CellSelectors/WFCEntropyCellSelector.h"

#include "Core/WFCGenerator.h"
#include "Core/WFCModel.h"


UWFCEntropyCellSelector::UWFCEntropyCellSelector()
	: RandomDeviation(0.001f)
{
}

float UWFCEntropyCellSelector::GetCellEntropy(int32 CellIndex) const
{
	if (!Generator)
	{
		return 0;
	}
	const FWFCCell& Cell = Generator->GetCell(CellIndex);
	if (Cell.HasSelectionOrNoCandidates())
	{
		return 0.f;
	}
	return CalculateShannonEntropy(Cell);
}

FWFCCellIndex UWFCEntropyCellSelector::SelectNextCell()
{
	check(Generator != nullptr);

	float MinEntropy = MAX_FLT;
	FWFCCellIndex BestCellIndex = INDEX_NONE;

	// select the cell with the lowest entropy, adding in a bit of randomness
	for (FWFCCellIndex CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
	{
		const FWFCCell& Cell = Generator->GetCell(CellIndex);

		if (Cell.HasSelectionOrNoCandidates())
		{
			// nothing to collapse
			continue;
		}

		const float Entropy = CalculateShannonEntropy(Cell) + FMath::FRand() * RandomDeviation;
		if (Entropy < MinEntropy)
		{
			MinEntropy = Entropy;
			BestCellIndex = CellIndex;
		}
	}

	return BestCellIndex;
}

float UWFCEntropyCellSelector::CalculateShannonEntropy(const FWFCCell& Cell) const
{
	check(Generator != nullptr);

	float SumOfWeights = 0.f;
	float SumOfLogWeights = 0.f;

	for (const FWFCTileId& TileId : Cell.TileCandidates)
	{
		const float Weight = Generator->GetModel()->GetTileWeightUnchecked(TileId);
		if (Weight <= 0.f)
		{
			continue;
		}

		SumOfWeights += Weight;
		SumOfLogWeights += Weight * FMath::Loge(Weight);
	}

	return FMath::Loge(SumOfWeights) - (SumOfLogWeights / SumOfWeights);
}
