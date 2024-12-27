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

typedef TStaticArray<double, 4> FHeightQuadrantArray;
typedef TStaticArray<int32, 4> FHeightIndexQuadrantArray;

/**
 * Proxy to make easier to get landscape heights
 * Height data is stored scaled but not translated
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

	// Each cell center is in the center of 4 height values
	UPROPERTY()
	FGrid2D LandscapeHeightGrid;

	UPROPERTY()
	int32 ComponentSize = 0;

	UPROPERTY()
	FBox LandscapeComponentBounds;
public:
	FLandscapeHeightProxy() = default;
	FLandscapeHeightProxy(const ULandscapeHeightfieldCollisionComponent* InLandscapeComponent);

	void Init(const ULandscapeHeightfieldCollisionComponent* InLandscapeComponent);

	int32 GetComponentSize() const;

	TArray<double> GetCollisionHeights();
	const TArray<double>& GetCollisionHeightsConst() const;
	
	FGrid2D GetGrid() const;
	FGrid2D& GetGrid();
	const FGrid2D& GetGridConst() const;

	int32 GetHeightIndex(const FVector& InLocation) const;
	
	double GetHeight(const int32 HeightIndex) const;
	double GetHeightTranslated(const int32 HeightIndex) const;

	double GetHeightTranslation() const;

	double GetMaxHeight(const FVector& InLocation) const;
	double GetMinHeight(const FVector& InLocation) const;
	double GetMeanHeight(const FVector& InLocation) const;
	double GetInterpolatedHeight(const FVector& InLocation) const;

	FHeightQuadrantArray GetHeights(const FVector& InLocation) const;
	TArray<double> GetHeights(const FBox& InBounds, FGrid2D& OutLocalGrid) const;

	FHeightQuadrantArray GetHeightIndices(const FVector& InLocation) const;

	FBox GetLandscapeComponentBounds() const;
	FBox GetGridBounds() const;
};