// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Generators/WFCEntropyGenerator.h"

#include "Core/WFCModel.h"


float UWFCEntropyGenerator::GetCellEntropy(int32 CellIndex) const
{
	const FWFCCell& Cell = GetCell(CellIndex);
	if (Cell.HasSelectionOrNoCandidates())
	{
		return 0.f;
	}
	return CalculateShannonEntropy(Cell);
}

FWFCCellIndex UWFCEntropyGenerator::SelectNextCellIndex()
{
	float MinEntropy = MAX_FLT;
	FWFCCellIndex BestCellIndex = INDEX_NONE;

	// select the cell with the lowest entropy, adding in a bit of randomness
	for (FWFCCellIndex CellIndex = 0; CellIndex < NumCells; ++CellIndex)
	{
		const FWFCCell& Cell = GetCell(CellIndex);

		if (Cell.HasSelectionOrNoCandidates())
		{
			// nothing to collapse
			continue;
		}

		const float Entropy = CalculateShannonEntropy(Cell) + FMath::FRand() / 1000.f;
		if (Entropy < MinEntropy)
		{
			MinEntropy = Entropy;
			BestCellIndex = CellIndex;
		}
	}

	return BestCellIndex;
}

FWFCTileId UWFCEntropyGenerator::SelectNextTileForCell(FWFCCellIndex Index)
{
	const FWFCCell& Cell = GetCell(Index);

	if (Cell.HasNoCandidates())
	{
		return INDEX_NONE;
	}

	// select a candidate, applying weighted probabilities
	float TotalWeight = 0.f;
	TArray<float> TileWeights;
	for (const FWFCTileId& TileId : Cell.TileCandidates)
	{
		const float TileWeight = Config.Model->GetTileWeightUnchecked(TileId);
		TileWeights.Add(TileWeight);
		TotalWeight += TileWeight;
	}

	float Rand = FMath::FRand() * TotalWeight;
	for (int32 Idx = 0; Idx < Cell.TileCandidates.Num(); ++Idx)
	{
		if (Rand >= TileWeights[Idx])
		{
			Rand -= TileWeights[Idx];
		}
		else
		{
			return Cell.TileCandidates[Idx];
		}
	}

	return Cell.TileCandidates[0];
}

float UWFCEntropyGenerator::CalculateShannonEntropy(const FWFCCell& Cell) const
{
	float SumOfWeights = 0.f;
	float SumOfLogWeights = 0.f;

	for (const FWFCTileId& TileId : Cell.TileCandidates)
	{
		const float Weight = Config.Model->GetTileWeightUnchecked(TileId);
		if (Weight <= 0.f)
		{
			continue;
		}

		SumOfWeights += Weight;
		SumOfLogWeights += Weight * FMath::Loge(Weight);
	}

	return FMath::Loge(SumOfWeights) - (SumOfLogWeights / SumOfWeights);
}
