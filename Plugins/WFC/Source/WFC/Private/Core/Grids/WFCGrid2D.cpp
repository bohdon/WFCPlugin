﻿// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Grids/WFCGrid2D.h"


UWFCGrid2D::UWFCGrid2D()
	: GridDimensions(FIntPoint(10, 10))
{
	// TODO: set default array of direction objects
}

int32 UWFCGrid2D::GetOppositeDirection(FWFCGridDirection Direction) const
{
	// {0, 1, 2, 3} represents {+X, +Y, -X, -Y}
	switch (Direction)
	{
	case 0:
		return 2;
	case 2:
		return 0;
	case 1:
		return 3;
	case 3:
		return 1;
	default:
		return INDEX_NONE;
	}
}

FWFCGridDirection UWFCGrid2D::GetRotatedDirection(FWFCGridDirection Direction, int32 Rotation) const
{
	if (!IsValidDirection(Direction))
	{
		return Direction;
	}
	// rotation is CW, and the 2d directions are also CW {+X, +Y, -X, -Y},
	// so add the rotation to the direction and get the remainder
	return (Direction + Rotation) % 4;
}

FWFCCellIndex UWFCGrid2D::GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const
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

int32 UWFCGrid2D::GetCellIndexForLocation(FIntPoint GridLocation) const
{
	return (GridLocation.Y * GridDimensions.X) + GridLocation.X;
}

FIntPoint UWFCGrid2D::GetLocationForCellIndex(int32 CellIndex) const
{
	const int32 X = CellIndex % GridDimensions.X;
	const int32 Y = (CellIndex - X) / GridDimensions.X;
	return FIntPoint(X, Y);
}
