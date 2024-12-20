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

FGrid2D::FGrid2D(const ULandscapeHeightfieldCollisionComponent& InLandscapeComponent)
{
	const int32 ComponentSize = InLandscapeComponent.CollisionHeightData.GetElementCount() + 1;
	const FBox ComponentBounds = InLandscapeComponent.Bounds.GetBox();
	
	const FVector QuadSize = ComponentBounds.GetSize() / FVector(
		FMath::Sqrt(static_cast<double>(ComponentSize)),
		FMath::Sqrt(static_cast<double>(ComponentSize)),
		1);

	FGrid2D::Init(QuadSize, ComponentBounds);
}

FGrid2D::FGrid2D(const FGrid2D& InCellGrid, const FBox& InBounds)
{
	checkf(IsInsideOrOnXY(InBounds), TEXT("New bounds must be inside the existing bounds"));
	
	FGrid2D::Init(InCellGrid, InBounds);
}

void FGrid2D::Init(const FVector& InCellSize, const FBox& InBounds)
{
}

void FGrid2D::Init(const ULandscapeHeightfieldCollisionComponent& InLandscapeComponent)
{
}

void FGrid2D::Init(const FGrid2D& InCellGrid, const FBox& InBounds)
{
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
	return IsInsideOrOnXY(InLocation);
}

bool FGrid2D::IsGridInside(const FGrid2D& InCellGrid) const
{
	return IsInsideOrOnXY(InCellGrid.Bounds);
}

int32 FGrid2D::GetCellIndex(const FVector& InLocation) const
{
	checkf(IsLocationInBounds(InLocation), TEXT("Location is out of bounds"));

	const FVector LocalLocation = InLocation + (Bounds.Min * -1);
	
	const int32 X = FMath::FloorToInt(LocalLocation.X / CellCount.X);
	const int32 Y = FMath::FloorToInt(LocalLocation.Y / CellCount.Y);

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
	
	const int32 X = FMath::FloorToInt(LocalLocation.X / CellCount.X);
	const int32 Y = FMath::FloorToInt(LocalLocation.Y / CellCount.Y);
	
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
	
	const int32 Y = (InIndex - CellCount.X * CellCount.Y) / CellCount.X;
	const int32 X = InIndex - CellCount.X * CellCount.Y - Y * CellCount.X;
	
	const FVector Min = Bounds.Min + FVector(X * CellSize.X, Y * CellSize.Y, CellSize.Z);
	const FVector Max = Min + CellSize;
	
	return FBox(Min, Max);
}

FBox FGrid2D::GetCellBounds(const FIntPoint& InCoordinate) const
{
	return FBox();
}

FBox FGrid2D::GetCellBounds(const FVector& InLocation) const
{
	return FBox();
}

TArray<int32> FGrid2D::GetCellIndicesFromBounds(const FBox& InBounds) const
{
	return TArray<int32>();
}

TArray<FIntPoint> FGrid2D::GetCellCoordinatesFromBounds(const FBox& InBounds) const
{
	return TArray<FIntPoint>();
}

FGrid2D FGrid2D::GetSubGrid(const FBox& InBounds) const
{
	return FGrid2D();
}

bool FGrid2D::operator==(const FGrid2D& InCellGrid) const
{
	return CellSize == InCellGrid.CellSize && Bounds == InCellGrid.Bounds;
}

bool FGrid2D::operator!=(const FGrid2D& InCellGrid) const
{
	return !(*this == InCellGrid);
}

bool FGrid2D::IsInsideOrOnXY(const FBox& InBounds) const
{
	return Bounds.IsInsideOrOnXY(InBounds.Min) && Bounds.IsInsideOrOnXY(InBounds.Max);
}

bool FGrid2D::IsInsideOrOnXY(const FVector& InLocation) const
{
	return Bounds.IsInsideOrOnXY(InLocation);
}

FCellData::FCellData(const FGrid2D& InCellGrid)
{
}

FCellData::FCellData(const FCellData& InCellData)
{
}

void FCellData::Init(const FGrid2D& InCellGrid)
{
}

void FCellData::Init(const FCellData& InCellData)
{
}

bool FCellData::GetOccupancy(const int32 InIndex) const
{
    checkf(OccupancyData.IsValidIndex(InIndex), TEXT("Invalid cell index %d"), InIndex);

	return OccupancyData[InIndex];
}

bool FCellData::GetOccupancy(const FIntPoint& InCellCoordinate) const
{
	checkf(CellGrid.IsCellCoordinateValid(InCellCoordinate), TEXT("Invalid coordinate %s"), *InCellCoordinate.ToString());

	return OccupancyData[CellGrid.GetCellIndex(InCellCoordinate)];
}

bool FCellData::GetOccupancy(const FVector2d& InLocation) const
{
	return false;
	// checkf(CellGrid.IsLocationInBounds(InLocation), TEXT("Invalid location %d"), InLocation);
	//
	// return Data[CellGrid.GetCellIndex(InLocation)];
}

void FCellData::SetOccupancy(const int32 InIndex, const bool bOccupied)
{
	checkf(OccupancyData.IsValidIndex(InIndex), TEXT("Invalid cell index %d"), InIndex);

	OccupancyData[InIndex] = bOccupied;
}

void FCellData::SetOccupancy(const FIntPoint& InCellCoordinate, const bool bOccupied)
{
	checkf(CellGrid.IsCellCoordinateValid(InCellCoordinate), TEXT("Invalid coordinate %s"), *InCellCoordinate.ToString());

	OccupancyData[CellGrid.GetCellIndex(InCellCoordinate)] = bOccupied;
}

void FCellData::SetOccupancy(const FVector2d& InLocation, const bool bOccupied)
{
	// checkf(CellGrid.IsLocationInBounds(InLocation), TEXT("Invalid location %d"), InLocation);
	//
	// Data[CellGrid.GetCellIndex(InLocation)] = bOccupied;
}

FCellData& FCellData::And(const FCellData& InCellData)
{
	return *this;
}

FCellData& FCellData::Or(const FCellData& InCellData)
{
	return *this;
}

FGrid2D& FCellData::GetCellGrid()
{
	return CellGrid;
}

const FGrid2D& FCellData::GetCellGridConst() const
{
	return CellGrid;
}

TArray<bool>& FCellData::GetOccupancyData()
{
	return OccupancyData;
}

const TArray<bool>& FCellData::GetOccupancyDataConst() const
{
	return OccupancyData;
}

TArray<int32> FCellData::GetOccupiedIndices() const
{
	TArray<int32> Result;
	Result.Reserve(OccupancyData.Num());

	for(int32 i = 0; i < OccupancyData.Num(); i++)
	{
		if(OccupancyData[i])
		{
			Result.Add(i);
		}
	}

	Result.Shrink();

	return Result;
}
