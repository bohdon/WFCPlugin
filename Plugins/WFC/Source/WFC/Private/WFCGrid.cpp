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

int32 UWFCGrid2d::GetOppositeDirection(FWFCGridDirection Direction) const
{
	// {0, 1, 2, 3} represents {+X, -X, +Y, -Y}
	switch (Direction)
	{
	case 0:
		return 1;
	case 1:
		return 0;
	case 2:
		return 3;
	case 3:
		return 2;
	default:
		return INDEX_NONE;
	}
}

FWFCCellIndex UWFCGrid2d::GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const
{
	if (!IsValidCellIndex(CellIndex) || !IsValidDirection(Direction))
	{
		// invalid cell or direction
		return INDEX_NONE;
	}

	const FIntPoint GridLocation = GetLocationForCellIndex(CellIndex);
	const FIntPoint MovedGridLocation = GridLocation + GetDirectionVector(Direction);
	return GetCellIndexForLocation(MovedGridLocation);
}

int32 UWFCGrid2d::GetCellIndexForLocation(FIntPoint GridLocation) const
{
	return (GridLocation.Y * GridDimensions.X) + GridLocation.X;
}

FIntPoint UWFCGrid2d::GetLocationForCellIndex(int32 CellIndex) const
{
	const int32 X = CellIndex % GridDimensions.X;
	const int32 Y = (CellIndex - X) / GridDimensions.X;
	return FIntPoint(X, Y);
}
