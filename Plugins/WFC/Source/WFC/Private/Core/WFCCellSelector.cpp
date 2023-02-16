// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/WFCCellSelector.h"

#include "Core/WFCGenerator.h"


// UWFCCellSelector
// ----------------

void UWFCCellSelector::Initialize(UWFCGenerator* InGenerator)
{
	Generator = InGenerator;
}

void UWFCCellSelector::Reset()
{
}

FWFCCellIndex UWFCCellSelector::SelectNextCell()
{
	return INDEX_NONE;
}


// UWFCRandomCellSelector
// ----------------------

FWFCCellIndex UWFCRandomCellSelector::SelectNextCell()
{
	TArray<FWFCCellIndex> OpenCells;
	for (FWFCCellIndex CellIndex = 0; CellIndex < Generator->GetNumCells(); ++CellIndex)
	{
		if (Generator->GetCell(CellIndex).HasSelectionOrNoCandidates())
		{
			continue;
		}
		OpenCells.Add(CellIndex);
	}

	if (!OpenCells.IsEmpty())
	{
		return OpenCells[FMath::RandHelper(OpenCells.Num())];
	}
	return INDEX_NONE;
}
