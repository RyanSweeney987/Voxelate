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
#include "LandscapeHeightfieldCollisionComponent.h"
#include "UObject/Object.h"
#include "Grid2D.generated.h"

/**
 * 2D grid of cells
 */
USTRUCT()
struct VOXELATE_API FGrid2D
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	FVector2d CellSize = FVector2d::ZeroVector;

	UPROPERTY()
	FBox Bounds = FBox();

	UPROPERTY()
	FIntPoint CellCount = FIntPoint::ZeroValue;

	UPROPERTY()
	TOptional<FIntPoint> Offset;

public:
	FGrid2D() = default;
	FGrid2D(const FVector2d& InCellSize, const FBox& InBounds);
	FGrid2D(const ULandscapeHeightfieldCollisionComponent& InLandscapeComponent);
	FGrid2D(const FGrid2D& InCellGrid, const FBox& InBounds);

	virtual ~FGrid2D() = default;
	
	virtual void Init(const FVector2d& InCellSize, const FBox& InBounds);
	virtual void Init(const ULandscapeHeightfieldCollisionComponent& InLandscapeComponent);
	virtual void Init(const FGrid2D& InCellGrid, const FBox& InBounds);
	
	FBox GetBounds() const;
	int32 GetCellCount() const;
	FIntPoint GetVectorCellCount() const;
	TOptional<FIntPoint> GetOffset() const;

	bool IsCellIndexValid(const int32 InIndex) const;
	bool IsCellCoordinateValid(const FIntPoint& InCoordinate) const;
	bool IsLocationInBounds(const FVector2d& InLocation) const;

	bool IsGridInside(const FGrid2D& InCellGrid) const;
	
	int32 GetCellIndex(const FVector2d& InLocation) const;
	int32 GetCellIndex(const FIntPoint& InCoordinate) const;
	
	FIntPoint GetCellCoordinate(const FVector2d& InLocation) const;
	FIntPoint GetCellCoordinate(const int32 InIndex) const;
	
	FBox GetCellBounds(const int32 InIndex) const;
	FBox GetCellBounds(const FIntPoint& InCoordinate) const;
	FBox GetCellBounds(const FVector2d& InLocation) const;
	
	TArray<int32> GetCellIndicesFromBounds(const FBox& InBounds) const;
	TArray<FIntPoint> GetCellCoordinatesFromBounds(const FBox& InBounds) const;
	
	FGrid2D GetSubGrid(const FBox& InBounds) const;

	bool operator==(const FGrid2D& InCellGrid) const;
	bool operator!=(const FGrid2D& InCellGrid) const;
};
