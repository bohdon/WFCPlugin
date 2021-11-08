// Copyright Bohdon Sayre. All Rights Reserved.


#include "WFCGrid.h"

#include "WFCTypes.h"


UWFCGrid::UWFCGrid()
{
}


UWFCGrid2d::UWFCGrid2d()
	: GridDimensions(FIntPoint(10, 10))
{
	// TODO: set default array of direction objects
}

int32 UWFCGrid2d::GetNumCells() const
{
	return GridDimensions.X * GridDimensions.Y;
}

FIntPoint UWFCGrid2d::GetCellLocation(int32 CellIndex) const
{
	const int32 X = CellIndex % GridDimensions.X;
	const int32 Y = (CellIndex - X) / GridDimensions.X;
	return FIntPoint(X, Y);
}
