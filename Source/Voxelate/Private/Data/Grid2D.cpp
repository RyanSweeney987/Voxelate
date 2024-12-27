/**
* MIT License
 *
 * Copyright (c) 2024 Ryan Sweeney
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Data/Grid2D.h"

FGrid2D::FGrid2D(const FVector& InCellSize, const FBox& InBounds)
{
	FGrid2D::Init(InCellSize, InBounds);
}

// FGrid2D::FGrid2D(const ULandscapeHeightfieldCollisionComponent& InLandscapeComponent)
// {
// 	FGrid2D::Init(InLandscapeComponent);
// }

FGrid2D::FGrid2D(const FGrid2D& InCellGrid, const FBox& InBounds)
{
	checkf(IsInsideXY(InBounds), TEXT("New bounds must be inside the existing bounds"));
	
	FGrid2D::Init(InCellGrid, InBounds);
}

void FGrid2D::Init(const FVector& InCellSize, const FBox& InBounds)
{
	CellSize = InCellSize;
	CellSize.Z = 100.0;

	Bounds = CalculateGridBounds(InCellSize, InBounds);

	CellCount = CalculateGridCount(InCellSize, Bounds.GetSize());
}

// void FGrid2D::Init(const ULandscapeHeightfieldCollisionComponent& InLandscapeComponent)
// {
// 	CellSize = InLandscapeComponent.GetComponentTransform().GetScale3D();
// 	Bounds = InLandscapeComponent.Bounds.GetBox();
//
// 	const FVector Min = InLandscapeComponent.Bounds.GetBox().Min - (CellSize / 2);
// 	const FVector Max = InLandscapeComponent.Bounds.GetBox().Max + (CellSize / 2);
// 	
// 	const FBox CellBounds = FBox(Min, Max);
// 	CellCount = FIntPoint((CellBounds.GetSize().X / CellSize.X), (CellBounds.GetSize().Y / CellSize.Y));
// }

void FGrid2D::Init(const FGrid2D& InCellGrid, const FBox& InBounds)
{
	checkf(InCellGrid.Bounds.IsInsideOrOn(InBounds), TEXT("New bounds must be inside the existing bounds"));

	// Initialise this grid
	Init(InCellGrid.CellSize, InBounds);

	// Calculate the offset of the new grid
	Offset = InCellGrid.GetCellCoordinate(InBounds.Min);
}

FBox FGrid2D::GetBounds() const
{
	return Bounds;
}

int32 FGrid2D::GetCellCount() const
{
	return CellCount.X * CellCount.Y;
}

FIntPoint FGrid2D::GetVectorCellCount() const
{
	return CellCount;
}

TOptional<FIntPoint> FGrid2D::GetOffset() const
{
	return Offset;
}

bool FGrid2D::IsCellIndexValid(const int32 InIndex) const
{
	return InIndex >= 0 && InIndex < GetCellCount();
}

bool FGrid2D::IsCellCoordinateValid(const FIntPoint& InCoordinate) const
{
	return InCoordinate.X >= 0 && InCoordinate.X < CellCount.X &&
		InCoordinate.Y >= 0 && InCoordinate.Y < CellCount.Y;
}

bool FGrid2D::IsLocationInBounds(const FVector& InLocation) const
{
	return IsInsideXY(InLocation);
}

bool FGrid2D::IsGridInside(const FGrid2D& InCellGrid) const
{
	return IsInsideXY(InCellGrid.Bounds);
}

int32 FGrid2D::GetCellIndex(const FVector& InLocation) const
{
	checkf(IsLocationInBounds(InLocation), TEXT("Location is out of bounds"));

	const FVector LocalLocation = InLocation + (Bounds.Min * -1);
	
	const int32 X = FMath::FloorToInt(LocalLocation.X / CellSize.X);
	const int32 Y = FMath::FloorToInt(LocalLocation.Y / CellSize.Y);

	return X + Y * CellCount.X;
}

int32 FGrid2D::GetCellIndex(const FIntPoint& InCoordinate) const
{
	checkf(IsCellCoordinateValid(InCoordinate), TEXT("Coordinate is out of bounds"));

	return InCoordinate.X + InCoordinate.Y * CellCount.X;
}

FIntPoint FGrid2D::GetCellCoordinate(const FVector& InLocation) const
{
	checkf(IsLocationInBounds(InLocation), TEXT("Location is out of bounds"));

	const FVector LocalLocation = InLocation + (Bounds.Min * -1);
	
	const int32 X = FMath::FloorToInt(LocalLocation.X / CellSize.X);
	const int32 Y = FMath::FloorToInt(LocalLocation.Y / CellSize.Y);
	
	return FIntPoint(X, Y);
}

FIntPoint FGrid2D::GetCellCoordinate(const int32 InIndex) const
{
	checkf(IsCellIndexValid(InIndex), TEXT("Index is out of bounds"));

	const int32 X = InIndex % CellCount.X;
	const int32 Y = InIndex / CellCount.X;

	return FIntPoint(X, Y);
}

FBox FGrid2D::GetCellBounds(const int32 InIndex) const
{
	checkf(IsCellIndexValid(InIndex), TEXT("Index is out of bounds"));

	const int32 X = InIndex % CellCount.X;
	const int32 Y = InIndex / CellCount.X;
	
	const FVector Min = Bounds.Min + FVector(X * CellSize.X, Y * CellSize.Y, 0);
	const FVector Max = Min + CellSize;
	
	return FBox(Min, Max);
}

FBox FGrid2D::GetCellBounds(const FIntPoint& InCoordinate) const
{
	checkf(IsCellCoordinateValid(InCoordinate), TEXT("Invalid cell coordinate %s"), *InCoordinate.ToString());

	const int32 X = InCoordinate.X;
	const int32 Y = InCoordinate.Y;
	
	const FVector Min = Bounds.Min + FVector(X * CellSize.X, Y * CellSize.Y, 0);
	const FVector Max = Min + CellSize;

	return FBox(Min, Max);
}

FBox FGrid2D::GetCellBounds(const FVector& InLocation) const
{
	checkf(IsLocationInBounds(InLocation), TEXT("Location is out of bounds"));
	
	return GetCellBounds(GetCellIndex(InLocation));
}

FVector FGrid2D::GetCellCenter(const int32 InIndex) const
{
	checkf(IsCellIndexValid(InIndex), TEXT("Index is out of bounds"));

	const int32 X = InIndex % CellCount.X;
	const int32 Y = InIndex / CellCount.X;
	
	const FVector Min = Bounds.Min + FVector(X * CellSize.X, Y * CellSize.Y, 0);
	
	return Min + (CellSize * FVector(0.5, 0.5, 1.0));
}

FVector FGrid2D::GetCellCenter(const FIntPoint& InCoordinate) const
{
	checkf(IsCellCoordinateValid(InCoordinate), TEXT("Invalid cell coordinate - InCoord: %s - Count: %s"), *InCoordinate.ToString(), *CellCount.ToString());

	const int32 X = InCoordinate.X;
	const int32 Y = InCoordinate.Y;
	
	const FVector Min = Bounds.Min + FVector(X * CellSize.X, Y * CellSize.Y, 0);

	return Min + (CellSize * FVector(0.5, 0.5, 1.0));
}

FVector FGrid2D::GetCellCenter(const FVector& InLocation) const
{
	checkf(IsLocationInBounds(InLocation), TEXT("Location is out of bounds"));

	return GetCellCenter(GetCellIndex(InLocation));
}

TArray<int32> FGrid2D::GetCellIndicesFromBounds(const FBox& InBounds) const
{
	const FBox& ClampedBounds = Bounds.Overlap(InBounds);

	checkf(ClampedBounds.GetVolume() > 0, TEXT("Bounds must intersect or be contained within the grid"));
	
	const FBox GridBounds = CalculateGridBounds(CellSize, ClampedBounds);
	const FIntPoint Count = CalculateGridCount(CellSize, GridBounds.GetSize());
	
	TArray<int32> Result;
	Result.Reserve(Count.X * Count.Y);
	
	const int32 IndexMin = GetCellIndex(GridBounds.Min);
	
	for(int32 Y = 0; Y < Count.Y; Y++)
	{
		for(int32 X = 0; X < Count.X; X++)
		{
			const int32 Index = IndexMin + X + (Y * CellCount.X);
			
			checkf(IsCellIndexValid(Index), TEXT("Invalid cell index %d"), Index);
			
			Result.Add(Index);
		}
	}
		
	return Result;
}

TArray<FIntPoint> FGrid2D::GetCellCoordinatesFromBounds(const FBox& InBounds) const
{
	const FBox& ClampedBounds = Bounds.Overlap(InBounds);

	checkf(ClampedBounds.GetVolume() > 0, TEXT("Bounds must intersect or be contained within the grid"));
	
	const FBox GridBounds = CalculateGridBounds(CellSize, InBounds);
	const FIntPoint Count = CalculateGridCount(CellSize, GridBounds.GetSize());
	
	TArray<FIntPoint> Result;
	Result.Reserve(Count.X * Count.Y);
	
	const FIntPoint CoordinateMin = GetCellCoordinate(GridBounds.Min);
	
	for(int32 Y = 0; Y < Count.Y; Y++)
	{
		for(int32 X = 0; X < Count.X; X++)
		{
			Result.Add(CoordinateMin + FIntPoint(X, Y));
		}
	}
		
	return Result;
}

/**
 * Takes the cell that the location returns and also returns the neighbouring cells
 * ie: If in the slightly NE of a cell, it will return the cell it's in and the cells to the E, N and NE
 * Order is always the following - A is the minimum index, D is the maximum index:
 * ---------
 * | C | D |
 * ---------
 * | A | B |
 * ---------
 * @param InLocation The location to get the indices for
 * @return The indices of the cells at the location
 */
// FIndexQuadrantArray FGrid2D::GetCellIndicesQuadrantFromLocation(const FVector& InLocation) const
// {
// 	const FCoordinateQuadrantArray Coordinates = GetCellCoordinatesQuadrantFromLocation(InLocation);
// 	
// 	const FIndexQuadrantArray Result = {
// 		GetCellIndex(Coordinates[0]),
// 		GetCellIndex(Coordinates[1]),
// 		GetCellIndex(Coordinates[2]),
// 		GetCellIndex(Coordinates[3])
// 	};
// 	
// 	return Result;
// }

/**
 * Takes the cell that the location returns and also returns the neighbouring cells
 * ie: If in the slightly NE of a cell, it will return the cell it's in and the cells to the E, N and NE
 * Order is always the following - A is the minimum coordinate, D is the maximum coordinate:
 * ---------
 * | C | D |
 * ---------
 * | A | B |
 * ---------
 * @param InLocation The location to get the coordinates for
 * @return The coordinates of the cells at the location
 */
// FCoordinateQuadrantArray FGrid2D::GetCellCoordinatesQuadrantFromLocation(const FVector& InLocation) const
// {
// 	const int32 Index = GetCellIndex(InLocation);
// 	const FIntPoint Coordinate = GetCellCoordinate(Index);
// 	const FVector Center = GetCellCenter(Coordinate);
//
// 	// Get the difference between the location and the center of the cell so we can get the sign of the delta
// 	const FVector Delta = InLocation - Center;
// 	
// 	const int32 XSign = FMath::Sign(Delta.X);
// 	const int32 YSign = FMath::Sign(Delta.Y);
//
// 	// Get the minimum X and Y coordinates
// 	const int32 XMin = FMath::Min(Coordinate.X, Coordinate.X + XSign);
// 	const int32 YMin = FMath::Min(Coordinate.Y, Coordinate.Y + YSign);
//
// 	// Create the coordinates for the quadrants from the minimum X and Y coordinates
// 	const FIntPoint A = FIntPoint(XMin, YMin);
// 	const FIntPoint B = FIntPoint(XMin + 1, YMin);
// 	const FIntPoint C = FIntPoint(XMin, YMin + 1);
// 	const FIntPoint D = FIntPoint(XMin + 1, YMin + 1);
//
// 	const FCoordinateQuadrantArray Result = {
// 		A,
// 		B,
// 		C,
// 		D
// 	};
// 	
// 	return Result;
// }

/**
 * Takes the cell center that the location returns and also returns the neighbouring cell centers
 * ie: If in the slightly NE of a cell, it will return the cell it's in and the cells to the E, N and NE
 * Order is always the following - A is the minimum location, D is the maximum location:
 * ---------
 * | C | D |
 * ---------
 * | A | B |
 * ---------
 * @param InLocation The location to get the centers for
 * @return The centers of the cells at the location
 */
// FCenterQuadrantArray FGrid2D::GetCellCentersQuadrantFromLocation(const FVector& InLocation) const
// {
// 	const FCoordinateQuadrantArray Coordinates = GetCellCoordinatesQuadrantFromLocation(InLocation);
// 	
// 	const FCenterQuadrantArray Result = {
// 		GetCellCenter(Coordinates[0]),
// 		GetCellCenter(Coordinates[1]),
// 		GetCellCenter(Coordinates[2]),
// 		GetCellCenter(Coordinates[3])
// 	};
//
// 	return Result;
// }

FGrid2D FGrid2D::GetSubGrid(const FBox& InBounds) const
{
	const FBox& Overlap = OverlapXY(InBounds);
	// Make sure that the bounds intersect or are completely inside the grid bounds
	checkf(Overlap.GetVolume() > 0, TEXT("Bounds must overlap or be inside the grid bounds"));
	// checkf(Bounds.Intersect(InBounds) || Bounds.IsInsideOrOn(InBounds), TEXT("Bounds must be inside the grid bounds"));

	return FGrid2D(*this, Overlap);
}

bool FGrid2D::operator==(const FGrid2D& InCellGrid) const
{
	return CellSize == InCellGrid.CellSize && Bounds == InCellGrid.Bounds;
}

bool FGrid2D::operator!=(const FGrid2D& InCellGrid) const
{
	return !(*this == InCellGrid);
}

bool FGrid2D::IsInsideXY(const FBox& InBounds) const
{
	return Bounds.IsInsideXY(InBounds.Min) && Bounds.IsInsideXY(InBounds.Max);
}

bool FGrid2D::IsInsideXY(const FVector& InLocation) const
{
	return Bounds.IsInsideXY(InLocation);
}

bool FGrid2D::IntersectXY(const FBox& InBounds) const
{
	const FVector Min = Bounds.Min;
	const FVector Max = Bounds.Max;
	
	if ((Min.X > InBounds.Max.X) || (InBounds.Min.X > Max.X))
	{
		return false;
	}

	if ((Min.Y > InBounds.Max.Y) || (InBounds.Min.Y > Max.Y))
	{
		return false;
	}

	return true;
}

FBox FGrid2D::OverlapXY(const FBox& InBounds) const
{
	if (IntersectXY(InBounds) == false)
	{
		return FBox();
	}

	const FVector Min = Bounds.Min;
	const FVector Max = Bounds.Max;
	
	// otherwise they overlap
	// so find overlapping box
	FVector MinVector, MaxVector;

	MinVector.X = FMath::Max(Min.X, InBounds.Min.X);
	MaxVector.X = FMath::Min(Max.X, InBounds.Max.X);

	MinVector.Y = FMath::Max(Min.Y, InBounds.Min.Y);
	MaxVector.Y = FMath::Min(Max.Y, InBounds.Max.Y);

	MinVector.Z = Min.Z;
	MaxVector.Z = Max.Z;

	return FBox(MinVector, MaxVector);
}

FBox FGrid2D::CalculateGridBounds(const FVector& InCellSize, const FBox& InBounds) const
{
	// Round bounds up to the nearest voxel size inclusive (so anything partial gets included)
	FVector BoundsMin = InBounds.Min;
	BoundsMin.X = FMath::FloorToFloat(BoundsMin.X / InCellSize.X) * InCellSize.X;
	BoundsMin.Y = FMath::FloorToFloat(BoundsMin.Y / InCellSize.Y) * InCellSize.Y;
	// BoundsMin.Z = FMath::FloorToFloat(BoundsMin.Z / InCellSize.Z) * InCellSize.Z;
	
	FVector BoundsMax = InBounds.Max;
	BoundsMax.X = FMath::CeilToFloat(BoundsMax.X / InCellSize.X) * InCellSize.X;
	BoundsMax.Y = FMath::CeilToFloat(BoundsMax.Y / InCellSize.Y) * InCellSize.Y;
	// BoundsMax.Z = FMath::CeilToFloat(BoundsMax.Z / InCellSize.Z) * InCellSize.Z;
	
	return FBox(BoundsMin, BoundsMax);
}

FIntPoint FGrid2D::CalculateGridCount(const FVector& InCellSize, const FVector& InBoundsSize) const
{
	const int32 NumCellsX = FMath::CeilToInt(InBoundsSize.X / InCellSize.X);
	const int32 NumCellsY = FMath::CeilToInt(InBoundsSize.Y / InCellSize.Y);

	return FIntPoint(NumCellsX, NumCellsY);
}

// FIntPoint FGrid2D::ClampCoordinate(const FIntPoint& InCoordinate) const
// {
// 	return FIntPoint(
// 		FMath::Clamp(InCoordinate.X, 0, CellCount.X - 1),
// 		FMath::Clamp(InCoordinate.Y, 0, CellCount.Y - 1)
// 	);
// }
