﻿/**
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


#include "Data/VoxelGrid.h"

#include "LandscapeInfo.h"
#include "LandscapeProxy.h"

/**
 * Struct constructor
 * @param InVoxelSize The size of each voxel in world space
 * @param InBounds The bounds of the voxel grid in world space
 */
FVoxelGrid::FVoxelGrid(const FVector& InVoxelSize, const FBox& InBounds)
{
	FVoxelGrid::Init(InVoxelSize, InBounds);
}

FVoxelGrid::FVoxelGrid(const ULandscapeHeightfieldCollisionComponent& InLandscapeComponent)
{
	const int32 ComponentSize = InLandscapeComponent.CollisionHeightData.GetElementCount() + 1;
	const FBox ComponentBounds = InLandscapeComponent.Bounds.GetBox();
	
	const FVector QuadSize = ComponentBounds.GetSize() / FVector(
		FMath::Sqrt(static_cast<double>(ComponentSize)),
		FMath::Sqrt(static_cast<double>(ComponentSize)),
		1);

	FVoxelGrid::Init(QuadSize, ComponentBounds);
}

/**
 * Creates a new voxel grid from an existing voxel grid and new bounds
 * @param InVoxelGrid The voxel grid to copy
 * @param InBounds The bounds of the new voxel grid
 */
FVoxelGrid::FVoxelGrid(const FVoxelGrid& InVoxelGrid, const FBox& InBounds)
{
	checkf(InVoxelGrid.Bounds.IsInsideOrOn(InBounds), TEXT("New bounds must be inside the existing bounds"));

	FVoxelGrid::Init(InVoxelGrid, InBounds);
}


/**
 * Initializes the voxel grid
 * @param InVoxelSize The size of each voxel in world space
 * @param InBounds The bounds of the voxel grid in world space
 */
void FVoxelGrid::Init(const FVector& InVoxelSize, const FBox& InBounds)
{
	VoxelSize = InVoxelSize;
	// Round bounds up to the nearest voxel size inclusive (so anything partial gets included)
	FVector BoundsMin = InBounds.Min;
	BoundsMin.X = FMath::FloorToFloat(BoundsMin.X / VoxelSize.X) * VoxelSize.X;
	BoundsMin.Y = FMath::FloorToFloat(BoundsMin.Y / VoxelSize.Y) * VoxelSize.Y;
	BoundsMin.Z = FMath::FloorToFloat(BoundsMin.Z / VoxelSize.Z) * VoxelSize.Z;
	
	FVector BoundsMax = InBounds.Max;
	BoundsMax.X = FMath::CeilToFloat(BoundsMax.X / VoxelSize.X) * VoxelSize.X;
	BoundsMax.Y = FMath::CeilToFloat(BoundsMax.Y / VoxelSize.Y) * VoxelSize.Y;
	BoundsMax.Z = FMath::CeilToFloat(BoundsMax.Z / VoxelSize.Z) * VoxelSize.Z;
	
	Bounds = FBox(BoundsMin, BoundsMax);

	// Calculate the number of voxels in each dimension
	VoxelCount = FIntVector(
		FMath::CeilToInt(Bounds.GetSize().X / VoxelSize.X),
		FMath::CeilToInt(Bounds.GetSize().Y / VoxelSize.Y),
		FMath::CeilToInt(Bounds.GetSize().Z / VoxelSize.Z));
}

void FVoxelGrid::Init(const ULandscapeHeightfieldCollisionComponent& InLandscapeComponent)
{
	const int32 ComponentSize = InLandscapeComponent.CollisionHeightData.GetElementCount() + 1;
	const FBox ComponentBounds = InLandscapeComponent.Bounds.GetBox();
	
	const FVector QuadSize = ComponentBounds.GetSize() / FVector(
		FMath::Sqrt(static_cast<double>(ComponentSize)),
		FMath::Sqrt(static_cast<double>(ComponentSize)),
		1);

	Init(QuadSize, ComponentBounds);
}

/**
 * Initializes the voxel grid based on an existing voxel grid and new bounds
 * @param InVoxelGrid The voxel grid to copy
 * @param InBounds The bounds of the new voxel grid
 */
void FVoxelGrid::Init(const FVoxelGrid& InVoxelGrid, const FBox& InBounds)
{
	checkf(InVoxelGrid.Bounds.IsInsideOrOn(InBounds), TEXT("New bounds must be inside the existing bounds"));

	// Initialise this grid
	Init(InVoxelGrid.VoxelSize, InBounds);

	// Calculate the offset of the new grid
	Offset = InVoxelGrid.GetVoxelCoordinate(InBounds.Min);
}

/**
 * Gets the bounds of the voxel grid
 * @return The bounds of the voxel grid
 */
FBox FVoxelGrid::GetBounds() const
{
	return Bounds;
}

/**
 * Gets the total number of voxels in the grid
 * @return The total number of voxels in the grid
 */
int32 FVoxelGrid::GetVoxelCount() const
{
	return VoxelCount.X * VoxelCount.Y * VoxelCount.Z;
}

/**
 * Gets the number of voxels in each dimension
 * @return The number of voxels in each dimension
 */
FIntVector FVoxelGrid::GetVectorVoxelCount() const
{
	return VoxelCount;
}

/**
 * Gets the offset of the grid
 * @return The offset of the grid
 */
TOptional<FIntVector> FVoxelGrid::GetOffset() const
{
	return Offset;
}

/**
 * Checks if a voxel index is valid
 * @param InIndex The index to check
 * @return true if the index is valid, false otherwise
 */
bool FVoxelGrid::IsVoxelIndexValid(const int32 InIndex) const
{
	return InIndex >= 0 && InIndex < GetVoxelCount();
}

/**
 * Checks if a voxel coordinate is valid (within the bounds of the grid)
 * @param InCoordinate The coordinate to check
 * @return true if the coordinate is valid, false otherwise
 */
bool FVoxelGrid::IsVoxelCoordinateValid(const FIntVector& InCoordinate) const
{
	return InCoordinate.X >= 0 && InCoordinate.X < VoxelCount.X &&
		InCoordinate.Y >= 0 && InCoordinate.Y < VoxelCount.Y &&
		InCoordinate.Z >= 0 && InCoordinate.Z < VoxelCount.Z;
}

/**
 * Checks if a location is within the bounds of the grid
 * @param InLocation The location to check
 * @return true if the location is within the bounds, false otherwise
 */
bool FVoxelGrid::IsLocationInBounds(const FVector& InLocation) const
{
	return Bounds.IsInsideOrOn(InLocation);
}

/**
 * Checks if the grid is inside this grid
 * @param InVoxelGrid The grid to check if it's inside this grid
 * @return true if the grid is inside this grid, false otherwise
 */
bool FVoxelGrid::IsGridInside(const FVoxelGrid& InVoxelGrid) const
{
	return Bounds.IsInsideOrOn(InVoxelGrid.Bounds);
}

/**
 * Gets the voxel index for a location
 * The location must be in the grid bounds
 * @param InLocation The location to get the index for
 * @return The index of the voxel at the location
 */
int32 FVoxelGrid::GetVoxelIndex(const FVector& InLocation) const
{
	checkf(IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));
	
	const FVector LocalLocation = InLocation + (Bounds.Min * -1);
	
	const int32 X = FMath::FloorToInt(LocalLocation.X / VoxelSize.X);
	const int32 Y = FMath::FloorToInt(LocalLocation.Y / VoxelSize.Y);
	const int32 Z = FMath::FloorToInt(LocalLocation.Z / VoxelSize.Z);
	
	const int32 Index = X +
		Y * FMath::CeilToInt(Bounds.GetSize().X / VoxelSize.X) +
		Z * FMath::CeilToInt(Bounds.GetSize().X / VoxelSize.X) * FMath::CeilToInt(Bounds.GetSize().Y / VoxelSize.Y);
		
	return Index;
}

/**
 * Gets the voxel index for a coordinate
 * The coordinate is a value between 0 and the number of voxels in each dimension
 * @param InCoordinate The coordinate to get the index for
 * @return The index of the voxel at the coordinate
 */
int32 FVoxelGrid::GetVoxelIndex(const FIntVector& InCoordinate) const
{
	checkf(IsVoxelCoordinateValid(InCoordinate), TEXT("Invalid voxel coordinate %s"), *InCoordinate.ToString());
	
	const int32 Index = InCoordinate.X + InCoordinate.Y * VoxelCount.X + InCoordinate.Z * VoxelCount.X * VoxelCount.Y;

	return Index;
}

/**
 * Gets the voxel coordinate for a location
 * The coordinate is a value between 0 and the number of voxels in each dimension
 * The location must be in the grid bounds
 * @param InLocation The location to get the coordinate for
 * @return The coordinate of the voxel at the location
 */
FIntVector FVoxelGrid::GetVoxelCoordinate(const FVector& InLocation) const
{
	checkf(IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));
	
	const FVector LocalLocation = InLocation + (Bounds.Min * -1);
	
	const int32 X = FMath::FloorToInt(LocalLocation.X / VoxelSize.X);
	const int32 Y = FMath::FloorToInt(LocalLocation.Y / VoxelSize.Y);
	const int32 Z = FMath::FloorToInt(LocalLocation.Z / VoxelSize.Z);
	
	return FIntVector(X, Y, Z);
}

/**
 * Gets the voxel coordinate for an index
 * The coordinate is a value between 0 and the number of voxels in each dimension
 * @param InIndex The index to get the coordinate for
 * @return The coordinate of the voxel at the index
 */
FIntVector FVoxelGrid::GetVoxelCoordinate(const int32 InIndex) const
{
	checkf(IsVoxelIndexValid(InIndex), TEXT("Invalid voxel index %d"), InIndex);
		
	const int32 Z = InIndex / (VoxelCount.X * VoxelCount.Y);
	const int32 Y = (InIndex - Z * VoxelCount.X * VoxelCount.Y) / VoxelCount.X;
	const int32 X = InIndex - Z * VoxelCount.X * VoxelCount.Y - Y * VoxelCount.X;
	
	return FIntVector(X, Y, Z);
}

/**
 * Gets the bounds of a voxel at a valid index
 * @param InIndex The index of the voxel
 * @return The bounds of the voxel
 */
FBox FVoxelGrid::GetVoxelBounds(const int32 InIndex) const
{
	checkf(IsVoxelIndexValid(InIndex), TEXT("Invalid voxel index %d"), InIndex);
	
	const int32 Z = InIndex / (VoxelCount.X * VoxelCount.Y);
	const int32 Y = (InIndex - Z * VoxelCount.X * VoxelCount.Y) / VoxelCount.X;
	const int32 X = InIndex - Z * VoxelCount.X * VoxelCount.Y - Y * VoxelCount.X;
	
	const FVector Min = Bounds.Min + FVector(X * VoxelSize.X, Y * VoxelSize.Y, Z * VoxelSize.Z);
	const FVector Max = Min + VoxelSize;
	
	return FBox(Min, Max);
}

/**
 * Gets the bounds of a voxel at a valid coordinate
 * @param InCoordinate The coordinate of the voxel
 * @return The bounds of the voxel
 */
FBox FVoxelGrid::GetVoxelBounds(const FIntVector& InCoordinate) const
{
	checkf(IsVoxelCoordinateValid(InCoordinate), TEXT("Invalid voxel coordinate %s"), *InCoordinate.ToString());

	const FVector Min = Bounds.Min + FVector(InCoordinate.X * VoxelSize.X, InCoordinate.Y * VoxelSize.Y, InCoordinate.Z * VoxelSize.Z);
	const FVector Max = Min + VoxelSize;

	return FBox(Min, Max);
}

/**
 * Gets the bounds of a voxel at a valid location
 * The location must be in the grid bounds
 * @param InLocation The location of the voxel
 * @return The bounds of the voxel
 */
FBox FVoxelGrid::GetVoxelBounds(const FVector& InLocation) const
{
	return GetVoxelBounds(GetVoxelIndex(InLocation));
}

/**
 * Gets the indices of all voxels that are within the bounds (inclusive)
 * @param InBounds The bounds to get the voxel indices for
 * @return The indices of all voxels within the bounds
 */
TArray<int32> FVoxelGrid::GetVoxelIndicesFromBounds(const FBox& InBounds) const
{
	// Round bounds up to the nearest voxel size inclusive (so anything partial gets included)
	FVector BoundsMin = InBounds.Min;
	BoundsMin.X = FMath::FloorToFloat(BoundsMin.X / VoxelSize.X) * VoxelSize.X;
	BoundsMin.Y = FMath::FloorToFloat(BoundsMin.Y / VoxelSize.Y) * VoxelSize.Y;
	BoundsMin.Z = FMath::FloorToFloat(BoundsMin.Z / VoxelSize.Z) * VoxelSize.Z;
	
	FVector BoundsMax = InBounds.Max;
	BoundsMax.X = FMath::CeilToFloat(BoundsMax.X / VoxelSize.X) * VoxelSize.X;
	BoundsMax.Y = FMath::CeilToFloat(BoundsMax.Y / VoxelSize.Y) * VoxelSize.Y;
	BoundsMax.Z = FMath::CeilToFloat(BoundsMax.Z / VoxelSize.Z) * VoxelSize.Z;
	
	const FVector BoundsSize = BoundsMax - BoundsMin;
	
	const int32 NumVoxelsX = FMath::CeilToInt(BoundsSize.X / VoxelSize.X);
	const int32 NumVoxelsY = FMath::CeilToInt(BoundsSize.Y / VoxelSize.Y);
	const int32 NumVoxelsZ = FMath::CeilToInt(BoundsSize.Z / VoxelSize.Z);
	const int32 NumVoxels = NumVoxelsX * NumVoxelsY * NumVoxelsZ;
	
	TArray<int32> Result;
	Result.Reserve(NumVoxels);
	
	const int32 IndexMin = GetVoxelIndex(BoundsMin);
	
	for(int32 Z = 0; Z < NumVoxelsZ; Z++)
	{
		for(int32 Y = 0; Y < NumVoxelsY; Y++)
		{
			for(int32 X = 0; X < NumVoxelsX; X++)
			{
				const int32 Index = IndexMin + X + Y * VoxelCount.X + Z * VoxelCount.X * VoxelCount.Y;
			
				checkf(IsVoxelIndexValid(Index), TEXT("Invalid voxel index %d"), Index);
			
				Result.Add(Index);
			}
		}
	}
		
	return Result;
}

/**
 * Gets the coordinates of all voxels that are within the bounds (inclusive)
 * @param InBounds The bounds to get the voxel coordinates for
 * @return The coordinates of all voxels within the bounds
 */
TArray<FIntVector> FVoxelGrid::GetVoxelCoordinatesFromBounds(const FBox& InBounds) const
{
	// Round bounds up to the nearest voxel size inclusive (so anything partial gets included)
	FVector BoundsMin = InBounds.Min;
	BoundsMin.X = FMath::FloorToFloat(BoundsMin.X / VoxelSize.X) * VoxelSize.X;
	BoundsMin.Y = FMath::FloorToFloat(BoundsMin.Y / VoxelSize.Y) * VoxelSize.Y;
	BoundsMin.Z = FMath::FloorToFloat(BoundsMin.Z / VoxelSize.Z) * VoxelSize.Z;
	
	FVector BoundsMax = InBounds.Max;
	BoundsMax.X = FMath::CeilToFloat(BoundsMax.X / VoxelSize.X) * VoxelSize.X;
	BoundsMax.Y = FMath::CeilToFloat(BoundsMax.Y / VoxelSize.Y) * VoxelSize.Y;
	BoundsMax.Z = FMath::CeilToFloat(BoundsMax.Z / VoxelSize.Z) * VoxelSize.Z;
	
	const FVector BoundsSize = BoundsMax - BoundsMin;
	
	const int32 NumVoxelsX = FMath::CeilToInt(BoundsSize.X / VoxelSize.X);
	const int32 NumVoxelsY = FMath::CeilToInt(BoundsSize.Y / VoxelSize.Y);
	const int32 NumVoxelsZ = FMath::CeilToInt(BoundsSize.Z / VoxelSize.Z);
	const int32 NumVoxels = NumVoxelsX * NumVoxelsY * NumVoxelsZ;
	
	TArray<FIntVector> Result;
	Result.Reserve(NumVoxels);
	
	const FIntVector CoordinateMin = GetVoxelCoordinate(BoundsMin);
	
	for(int32 Z = 0; Z < NumVoxelsZ; Z++)
	{
		for(int32 Y = 0; Y < NumVoxelsY; Y++)
		{
			for(int32 X = 0; X < NumVoxelsX; X++)
			{
				Result.Add(CoordinateMin + FIntVector(X, Y, Z));
			}
		}
	}
		
	return Result;
}

/**
 * Gets a sub grid from the current grid based on the input bounds
 * The bounds are rounded up to the nearest voxel size inclusive
 * @param InBounds The bounds to get the sub grid for
 * @return The sub grid
 */
FVoxelGrid FVoxelGrid::GetSubGrid(const FBox& InBounds) const
{
	return FVoxelGrid(*this, InBounds);
}

/**
 * Checks if two voxel grids are equal
 * @param InVoxelGrid The voxel grid to compare with
 * @return true if the voxel grids are equal, false otherwise
 */
bool FVoxelGrid::operator==(const FVoxelGrid& InVoxelGrid) const
{
	return VoxelSize == InVoxelGrid.VoxelSize && Bounds == InVoxelGrid.Bounds;
}

/**
 * Checks if two voxel grids are not equal
 * @param InVoxelGrid The voxel grid to compare with
 * @return true if the voxel grids are not equal, false otherwise
 */
bool FVoxelGrid::operator!=(const FVoxelGrid& InVoxelGrid) const
{
	return !(*this == InVoxelGrid);
}

/**
 * Struct constructor
 * @param InVoxelGrid The voxel grid that this data is associated with
 */
FVoxelData::FVoxelData(const FVoxelGrid& InVoxelGrid) : VoxelGrid(InVoxelGrid)
{
	FVoxelData::Init(InVoxelGrid);
}

/**
 * Copy constructor
 * @param InVoxelData The voxel data to copy
 */
FVoxelData::FVoxelData(const FVoxelData& InVoxelData) : OccupancyData(InVoxelData.OccupancyData), VoxelGrid(InVoxelData.VoxelGrid)
{
}

void FVoxelData::Init(const FVoxelGrid& InVoxelGrid)
{
	const int32 NumVoxels = VoxelGrid.GetVoxelCount();
	OccupancyData.Init(false, NumVoxels);
}

void FVoxelData::Init(const FVoxelData& InVoxelData)
{
	VoxelGrid = InVoxelData.VoxelGrid;
	OccupancyData = InVoxelData.OccupancyData;
}

/**
 * Gets the occupancy of a voxel at an index
 * The index must be in the grid bounds
 * @param InIndex The index of the voxel
 * @return The occupancy of the voxel
 */
bool FVoxelData::GetOccupancy(const int32 InIndex) const
{
	checkf(VoxelGrid.IsVoxelIndexValid(InIndex), TEXT("Invalid voxel index %d"), InIndex);
	
	return OccupancyData[InIndex];
}

/**
 * Gets the occupancy of a voxel at a location
 * The location must be in the grid bounds
 * @param InVoxelCoordinate The coordinate of the voxel
 * @return The occupancy of the voxel
 */
bool FVoxelData::GetOccupancy(const FIntVector& InVoxelCoordinate) const
{
	checkf(VoxelGrid.IsVoxelCoordinateValid(InVoxelCoordinate), TEXT("Invalid voxel coordinate %s"), *InVoxelCoordinate.ToString());

	return GetOccupancy(VoxelGrid.GetVoxelIndex(InVoxelCoordinate));
}

/**
 * Gets the occupancy of a voxel at a location
 * The location must be in the grid bounds
 * @param InLocation The location of the voxel
 * @return The occupancy of the voxel
 */
bool FVoxelData::GetOccupancy(const FVector& InLocation) const
{
	checkf(VoxelGrid.IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));

	return GetOccupancy(VoxelGrid.GetVoxelIndex(InLocation));
}

/**
 * Sets the occupancy of a voxel at an index
 * The index must be in the grid bounds
 * @param InIndex The index of the voxel
 * @param bOccupied The occupancy of the voxel
 */
void FVoxelData::SetOccupancy(const int32 InIndex, const bool bOccupied)
{
	checkf(VoxelGrid.IsVoxelIndexValid(InIndex), TEXT("Invalid voxel index %d"), InIndex);

	OccupancyData[InIndex] = bOccupied;
}

/**
 * Sets the occupancy of a voxel at a location
 * The location must be in the grid bounds
 * @param InVoxelCoordinate The coordinate of the voxel
 * @param bOccupied The occupancy of the voxel
 */
void FVoxelData::SetOccupancy(const FIntVector& InVoxelCoordinate, const bool bOccupied)
{
	checkf(VoxelGrid.IsVoxelCoordinateValid(InVoxelCoordinate), TEXT("Invalid voxel coordinate %s"), *InVoxelCoordinate.ToString());

	OccupancyData[VoxelGrid.GetVoxelIndex(InVoxelCoordinate)] = bOccupied;
}

/**
 * Sets the occupancy of a voxel at a location
 * The location must be in the grid bounds
 * @param InLocation The location of the voxel
 * @param bOccupied The occupancy of the voxel
 */
void FVoxelData::SetOccupancy(const FVector& InLocation, const bool bOccupied)
{
	checkf(VoxelGrid.IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));

	OccupancyData[VoxelGrid.GetVoxelIndex(InLocation)] = bOccupied;
}

/**
 * Performs a bitwise AND operation on the voxel data
 * @param InVoxelData The voxel data to AND with
 * @return The resulting voxel data
 */
FVoxelData& FVoxelData::And(const FVoxelData& InVoxelData)
{
	checkf(VoxelGrid.IsGridInside(InVoxelData.VoxelGrid), TEXT("Input voxel grid out of bounds"));
	checkf(OccupancyData.Num() >= InVoxelData.OccupancyData.Num(), TEXT("Too much input voxel data - input %d - expected %d"), InVoxelData.OccupancyData.Num(), OccupancyData.Num());

	if(const TOptional<FIntVector> Offset = InVoxelData.GetVoxelGridConst().GetOffset(); Offset.IsSet())
	{
		for(int32 i = 0; i < InVoxelData.OccupancyData.Num(); i++)
		{
			const int32 Index = VoxelGrid.GetVoxelIndex(Offset.GetValue() + InVoxelData.VoxelGrid.GetVoxelCoordinate(i));

			OccupancyData[Index] = OccupancyData[Index] && InVoxelData.OccupancyData[i];
		}
	} else
	{
		for(int32 i = 0; i < OccupancyData.Num(); i++)
		{
			OccupancyData[i] = OccupancyData[i] && InVoxelData.OccupancyData[i];
		}
	}
	
	return *this;
}

/**
 * Performs a bitwise OR operation on the voxel data
 * @param InVoxelData The voxel data to OR with
 * @return The resulting voxel data
 */
FVoxelData& FVoxelData::Or(const FVoxelData& InVoxelData)
{
	checkf(VoxelGrid.IsGridInside(InVoxelData.VoxelGrid), TEXT("Input voxel grid out of bounds"));
	checkf(OccupancyData.Num() >= InVoxelData.OccupancyData.Num(), TEXT("Too much input voxel data - input %d - expected %d"), InVoxelData.OccupancyData.Num(), OccupancyData.Num());

	if(const TOptional<FIntVector> Offset = InVoxelData.GetVoxelGridConst().GetOffset(); Offset.IsSet())
	{
		for(int32 i = 0; i < InVoxelData.OccupancyData.Num(); i++)
		{
			const int32 Index = VoxelGrid.GetVoxelIndex(Offset.GetValue() + InVoxelData.VoxelGrid.GetVoxelCoordinate(i));

			OccupancyData[Index] = OccupancyData[Index] || InVoxelData.OccupancyData[i];
		}
	} else
	{
		for(int32 i = 0; i < OccupancyData.Num(); i++)
		{
			OccupancyData[i] = OccupancyData[i] || InVoxelData.OccupancyData[i];
		}
	}

	return *this;
}

/**
 * Performs a bitwise XOR operation on the voxel data
 * @param InVoxelData The voxel data to XOR with
 * @return The resulting voxel data
 */
FVoxelData& FVoxelData::Xor(const FVoxelData& InVoxelData)
{
	checkf(VoxelGrid.IsGridInside(InVoxelData.VoxelGrid), TEXT("Input voxel grid out of bounds"));
	checkf(OccupancyData.Num() >= InVoxelData.OccupancyData.Num(), TEXT("Too much input voxel data - input %d - expected %d"), InVoxelData.OccupancyData.Num(), OccupancyData.Num());

	if(const TOptional<FIntVector> Offset = InVoxelData.GetVoxelGridConst().GetOffset(); Offset.IsSet())
	{
		for(int32 i = 0; i < InVoxelData.OccupancyData.Num(); i++)
		{
			const int32 Index = VoxelGrid.GetVoxelIndex(Offset.GetValue() + InVoxelData.VoxelGrid.GetVoxelCoordinate(i));

			OccupancyData[Index] = OccupancyData[Index] ^ InVoxelData.OccupancyData[i];
		}
	} else
	{
		for(int32 i = 0; i < OccupancyData.Num(); i++)
		{
			OccupancyData[i] = OccupancyData[i] ^ InVoxelData.OccupancyData[i];
		}
	}
	
	return *this;
}

/**
 * Gets the voxel grid associated with this data
 * @return The voxel grid associated with this data
 */
FVoxelGrid& FVoxelData::GetVoxelGrid()
{
	return VoxelGrid;
}

/**
 * Gets the voxel grid associated with this data
 * @return The voxel grid associated with this data
 */
const FVoxelGrid& FVoxelData::GetVoxelGridConst() const
{
	return VoxelGrid;
}

/**
 * Gets the occupancy data
 * @return The occupancy data
 */
TArray<bool>& FVoxelData::GetOccupancyData() {
	return OccupancyData;
}

/**
 * Gets the occupancy data
 * @return The occupancy data
 */
const TArray<bool>& FVoxelData::GetOccupancyDataConst() const
{
	return OccupancyData;
}

/**
 * Gets the indices of all voxels that are occupied
 * @return The indices of all occupied voxels
 */
TArray<int32> FVoxelData::GetOccupiedIndices() const
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
