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
#include "Grid2D.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeHeightProxy.generated.h"

/**
 * Proxy to make easier to get landscape heights
 */
USTRUCT()
struct VOXELATE_API FLandscapeHeightProxy
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	TArray<double> CollisionHeights;

	// Each height value is the center of a cell which means the edges can have overlapping cells
	UPROPERTY()
	FGrid2D LandscapeHeightGrid;
	
public:
	FLandscapeHeightProxy() = default;
	FLandscapeHeightProxy(const ULandscapeHeightfieldCollisionComponent* InLandscapeComponent);

	void Init(const ULandscapeHeightfieldCollisionComponent* InLandscapeComponent);

	FGrid2D GetGrid() const;
	FGrid2D& GetGrid();
	const FGrid2D& GetGridConst() const;
	
	double GetHeight(const int32 Index) const;
	double GetHeight(const FIntPoint& Coordinate) const;

	double GetMaxHeight(const FVector& InLocation) const;
	double GetMinHeight(const FVector& InLocation) const;
	double GetMeanHeight(const FVector& InLocation) const;
	double GetInterpolatedHeight(const FVector& InLocation) const;

	TArray<double> GetHeights(const FVector& InLocation) const;
	TArray<double> GetHeights(const FBox& InBounds, FGrid2D& OutLocalGrid) const;

	FBox GetBounds() const;
	// double GetExactHeight(const int32 InIndex) const;
	// double GetExactHeight(const FIntPoint& InCoordinate) const;
	// double GetExactHeight(const FVector2d& InLocation) const;

	// double GetInterpolatedHeight(const int32 InIndex) const;
	// double GetInterpolatedHeight(const FIntPoint& InCoordinate) const;
	// double GetInterpolatedHeight(const FVector2d& InLocation) const;
	
	// FBox GetBounds() const;
};


