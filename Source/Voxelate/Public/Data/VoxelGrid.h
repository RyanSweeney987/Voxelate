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

#pragma once

#include "CoreMinimal.h"
#include "LandscapeProxy.h"
#include "VoxelGrid.generated.h"


/**
 * This struct handles the logic for working with a voxel grid
 * It does not handle the actual voxel data, just the grid itself
 * TODO: Fix issue with being able to get voxels outside of bounds
 */
USTRUCT()
struct VOXELATE_API FVoxelGrid
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	FVector VoxelSize = FVector::ZeroVector;

	UPROPERTY()
	FBox Bounds = FBox();

	UPROPERTY()
	FIntVector VoxelCount = FIntVector::ZeroValue;

public:
	FVoxelGrid() = default;
	FVoxelGrid(const FVector& InVoxelSize, const FBox& InBounds);
	FVoxelGrid(const ULandscapeHeightfieldCollisionComponent& InLandscapeComponent);
	
	void Init(const FVector& InVoxelSize, const FBox& InBounds);

	FBox GetBounds() const;
	
	int32 GetVoxelCount() const;
	FIntVector GetVectorVoxelCount() const;

	bool IsVoxelIndexValid(const int32 InIndex) const;
	bool IsVoxelCoordinateValid(const FIntVector& InCoordinate) const;
	bool IsLocationInBounds(const FVector& InLocation) const;
	
	int32 GetVoxelIndex(const FVector& InLocation) const;
	int32 GetVoxelIndex(const FIntVector& InCoordinate) const;
	
	FIntVector GetVoxelCoordinate(const FVector& InLocation) const;
	FIntVector GetVoxelCoordinate(const int32 InIndex) const;
	
	FBox GetVoxelBounds(const int32 InIndex) const;
	FBox GetVoxelBounds(const FIntVector& InCoordinate) const;
	FBox GetVoxelBounds(const FVector& InLocation) const;
	
	TArray<int32> GetVoxelIndicesFromBounds(const FBox& InBounds) const;
	TArray<FIntVector> GetVoxelCoordinatesFromBounds(const FBox& InBounds) const;
	
	FVoxelGrid GetSubGrid(const FBox& InBounds) const;
private:
	
};