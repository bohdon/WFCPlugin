// Copyright Bohdon Sayre. All Rights Reserved.


#include "Core/Grids/WFCGrid3D.h"


UWFCGrid3DConfig::UWFCGrid3DConfig()
	: Dimensions(FIntVector(10, 10, 10)),
	  CellSize(FVector(100.f, 100.f, 100.f))
{
	GridClass = UWFCGrid3D::StaticClass();
}


UWFCGrid3D::UWFCGrid3D()
	: Dimensions(FIntVector(10, 10, 10)),
	  CellSize(FVector(100.f, 100.f, 100.f))
{
	// TODO: set default array of direction objects
}

void UWFCGrid3D::Initialize(const UWFCGridConfig* Config)
{
	Super::Initialize(Config);

	const UWFCGrid3DConfig* Config3D = Cast<UWFCGrid3DConfig>(Config);
	check(Config3D != nullptr);

	// TODO: flip this, let config configure the grid so configs can be subclassed more easily
	Dimensions = Config3D->Dimensions;
	CellSize = Config3D->CellSize;
}

int32 UWFCGrid3D::GetNumCells() const
{
	return Dimensions.X * Dimensions.Y * Dimensions.Z;
}

FString UWFCGrid3D::GetDirectionName(int32 Direction) const
{
	switch (Direction)
	{
	case 0: return FString(TEXT("+X"));
	case 1: return FString(TEXT("+Y"));
	case 2: return FString(TEXT("-X"));
	case 3: return FString(TEXT("-Y"));
	case 4: return FString(TEXT("+Z"));
	case 5: return FString(TEXT("-Z"));
	default: return FString(TEXT("Unknown"));
	}
}

FString UWFCGrid3D::GetCellName(int32 CellIndex) const
{
	const FIntVector Location = GetLocationForCellIndex(CellIndex);
	return FString::Printf(TEXT("%d,%d,%d"), Location.X, Location.Y, Location.Z);
}

int32 UWFCGrid3D::GetOppositeDirection(FWFCGridDirection Direction) const
{
	// {0, 1, 2, 3, 4, 5} represents {+X, +Y, -X, -Y, +Z, -Z}
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
	case 4:
		return 5;
	case 5:
		return 4;
	default:
		return INDEX_NONE;
	}
}

FWFCGridDirection UWFCGrid3D::RotateDirection(FWFCGridDirection Direction, int32 Rotation) const
{
	// don't rotate invalid direction, or Z directions (only yaw is currently supported)
	if (!IsValidDirection(Direction) || Direction >= 4)
	{
		return Direction;
	}
	// rotation is CW, and the 2d directions are also CW {+X, +Y, -X, -Y},
	// so add the rotation to the direction and get the remainder
	return (Direction + Rotation) % 4;
}

FWFCGridDirection UWFCGrid3D::InverseRotateDirection(FWFCGridDirection Direction, int32 Rotation) const
{
	// inverse the rotation, then rotate the direction, only applies to X & Y directions
	if (!IsValidDirection(Direction) || Direction >= 4)
	{
		return Direction;
	}

	const int32 InvRotation = (4 - Rotation) % 4;
	return RotateDirection(Direction, InvRotation);
}

FWFCCellIndex UWFCGrid3D::GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const
{
	if (!IsValidCellIndex(CellIndex) || !IsValidDirection(Direction))
	{
		// invalid cell or direction
		return INDEX_NONE;
	}

	const FIntVector GridLocation = GetLocationForCellIndex(CellIndex);
	const FIntVector MovedGridLocation = GridLocation + GetDirectionVector(Direction);
	return GetCellIndexForLocation(MovedGridLocation);
}

int32 UWFCGrid3D::GetCellIndexForLocation(FIntVector GridLocation) const
{
	if (GridLocation.X < 0 || GridLocation.X >= Dimensions.X ||
		GridLocation.Y < 0 || GridLocation.Y >= Dimensions.Y ||
		GridLocation.Z < 0 || GridLocation.Z >= Dimensions.Z)
	{
		return INDEX_NONE;
	}
	return GridLocation.X + (GridLocation.Y * Dimensions.X) + (GridLocation.Z * Dimensions.X * Dimensions.Y);
}

FIntVector UWFCGrid3D::GetLocationForCellIndex(int32 CellIndex) const
{
	// TODO: simplify
	const int32 DimXY = Dimensions.X * Dimensions.Y;
	const int32 Z = FMath::FloorToInt(static_cast<float>(CellIndex) / static_cast<float>(DimXY));
	const int32 Y = FMath::FloorToInt(static_cast<float>(CellIndex - Z * DimXY) / static_cast<float>(Dimensions.X));
	const int32 X = CellIndex % Dimensions.X;
	return FIntVector(X, Y, Z);
}

FIntVector UWFCGrid3D::GetDirectionVector(int32 Direction) const
{
	return GetDirectionVectorStatic(Direction);
}

FIntVector UWFCGrid3D::GetDirectionVectorStatic(int32 Direction)
{
	switch (Direction)
	{
	case 0:
		return FIntVector(1, 0, 0);
	case 1:
		return FIntVector(0, 1, 0);
	case 2:
		return FIntVector(-1, 0, 0);
	case 3:
		return FIntVector(0, -1, 0);
	case 4:
		return FIntVector(0, 0, 1);
	case 5:
		return FIntVector(0, 0, -1);
	default:
		return FIntVector();
	}
}

FVector UWFCGrid3D::GetCellWorldLocation(int32 CellIndex, bool bCenter) const
{
	if (!IsValidCellIndex(CellIndex))
	{
		return FVector::ZeroVector;
	}

	const FIntVector CellLocation = GetLocationForCellIndex(CellIndex);
	const FVector CellWorldLocation = FVector(CellLocation) * CellSize;
	if (bCenter)
	{
		return CellWorldLocation + CellSize * 0.5f;
	}
	return CellWorldLocation;
}

FTransform UWFCGrid3D::GetCellWorldTransform(int32 CellIndex, int32 Rotation) const
{
	FTransform Result = GetRotationTransform(Rotation);
	const FVector Offset = FVector(GetLocationForCellIndex(CellIndex)) * CellSize;
	Result.AddToTranslation(Offset);
	return Result;
}

FTransform UWFCGrid3D::GetRotationTransform(int32 Rotation) const
{
	const FVector ForwardVector = FVector(GetDirectionVector(RotateDirection(0, Rotation)));
	// apply rotation to a location offset by half cell size, then offset again to keep transform in 0..1 range
	const FMatrix Matrix = FTranslationMatrix(CellSize * -0.5f) *
		FRotationMatrix::MakeFromX(ForwardVector) *
		FTranslationMatrix(CellSize * 0.5f);
	return FTransform(Matrix);
}
