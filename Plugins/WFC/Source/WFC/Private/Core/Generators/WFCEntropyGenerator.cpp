// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Generators/WFCEntropyGenerator.h"

#include "Core/WFCModel.h"


FWFCCellIndex UWFCEntropyGenerator::SelectNextCellIndex()
{
	int32 MinNumCandidates = INT_MAX;
	TArray<FWFCCellIndex> BestCells;

	for (FWFCCellIndex CellIndex = 0; CellIndex < NumCells; ++CellIndex)
	{
		const FWFCCell& Cell = GetCell(CellIndex);
		const int32 NumCandidates = Cell.TileCandidates.Num();

		if (Cell.HasSelectionOrNoCandidates())
		{
			// don't include cells without multiple candidates
			continue;
		}

		if (NumCandidates < MinNumCandidates)
		{
			MinNumCandidates = NumCandidates;
			BestCells.Reset();
			BestCells.Add(CellIndex);
		}
		else if (NumCandidates == MinNumCandidates)
		{
			BestCells.Add(CellIndex);
		}
	}

	if (BestCells.Num() == 0)
	{
		return INDEX_NONE;
	}

	// TODO: add weighting / prioritization, or other possible random selection logic
	return BestCells[FMath::RandHelper(BestCells.Num())];
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
