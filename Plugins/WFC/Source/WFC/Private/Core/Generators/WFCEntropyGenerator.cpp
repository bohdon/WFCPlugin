// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Generators/WFCEntropyGenerator.h"


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

	return Cell.TileCandidates[FMath::RandHelper(Cell.TileCandidates.Num())];
}
