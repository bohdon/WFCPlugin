// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Grids/WFCGrid2D.h"


UWFCGrid2DConfig::UWFCGrid2DConfig()
	: Dimensions(FIntPoint(10, 10)),
	  CellSize(FVector2D(100.f, 100.f))
{
	GridClass = UWFCGrid2D::StaticClass();
}


UWFCGrid2D::UWFCGrid2D()
	: Dimensions(FIntPoint(10, 10)),
	  CellSize(FVector2D(100.f, 100.f))
{
	// TODO: set default array of direction objects
}

void UWFCGrid2D::Initialize(const UWFCGridConfig* Config)
{
	Super::Initialize(Config);

	const UWFCGrid2DConfig* Config2D = Cast<UWFCGrid2DConfig>(Config);
	check(Config2D != nullptr);

	Dimensions = Config2D->Dimensions;
	CellSize = Config2D->CellSize;
}

int32 UWFCGrid2D::GetNumCells() const
{
	return Dimensions.X * Dimensions.Y;
}

FString UWFCGrid2D::GetDirectionName(int32 Direction) const
{
	switch (Direction)
	{
	case 0: return FString(TEXT("+X"));
	case 1: return FString(TEXT("+Y"));
	case 2: return FString(TEXT("-X"));
	case 3: return FString(TEXT("-Y"));
	default: return FString(TEXT("Unknown"));
	}
}

FString UWFCGrid2D::GetCellName(int32 CellIndex) const
{
	const FIntPoint Location = GetLocationForCellIndex(CellIndex);
	return FString::Printf(TEXT("%d,%d"), Location.X, Location.Y);
}

int32 UWFCGrid2D::GetOppositeDirection(FWFCGridDirection Direction) const
{
	// {0, 1, 2, 3} represents {+X, +Y, -X, -Y}
	switch (Direction)
	{
	case 0: return 2;
	case 2: return 0;
	case 1: return 3;
	case 3: return 1;
	default: return INDEX_NONE;
	}
}

FWFCGridDirection UWFCGrid2D::RotateDirection(FWFCGridDirection Direction, int32 Rotation) const
{
	if (!IsValidDirection(Direction))
	{
		return Direction;
	}
	// rotation is CW, and the 2d directions are also CW {+X, +Y, -X, -Y},
	// so add the rotation to the direction and get the remainder
	return (Direction + Rotation) % 4;
}

FWFCGridDirection UWFCGrid2D::InverseRotateDirection(FWFCGridDirection Direction, int32 Rotation) const
{
	// inverse the rotation, then rotate the direction
	const int32 InvRotation = (4 - Rotation) % 4;
	return RotateDirection(Direction, InvRotation);
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
	if (GridLocation.X < 0 || GridLocation.X >= Dimensions.X ||
		GridLocation.Y < 0 || GridLocation.Y >= Dimensions.Y)
	{
		return INDEX_NONE;
	}
	return GridLocation.X + (GridLocation.Y * Dimensions.X);
}

FIntPoint UWFCGrid2D::GetLocationForCellIndex(int32 CellIndex) const
{
	const int32 X = CellIndex % Dimensions.X;
	const int32 Y = (CellIndex - X) / Dimensions.X;
	return FIntPoint(X, Y);
}

FVector UWFCGrid2D::GetCellWorldLocation(int32 CellIndex, bool bCenter) const
{
	if (!IsValidCellIndex(CellIndex))
	{
		return FVector::ZeroVector;
	}

	const FIntPoint CellLocation = GetLocationForCellIndex(CellIndex);
	const FVector CellSize3D = FVector(CellSize.X, CellSize.Y, 1.f);
	const FVector CellWorldLocation = FVector(CellLocation) * CellSize3D;
	if (bCenter)
	{
		return CellWorldLocation + CellSize3D * 0.5f;
	}
	return CellWorldLocation;
}

FIntPoint UWFCGrid2D::GetDirectionVector(int32 Direction)
{
	switch (Direction)
	{
	case 0:
		return FIntPoint(1, 0);
	case 1:
		return FIntPoint(0, 1);
	case 2:
		return FIntPoint(-1, 0);
	case 3:
		return FIntPoint(0, -1);
	default:
		return FIntPoint();
	}
}
