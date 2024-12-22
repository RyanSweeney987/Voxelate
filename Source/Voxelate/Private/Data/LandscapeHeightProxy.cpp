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

#include "Data/LandscapeHeightProxy.h"

/**
 * Struct constructor
 * @param InLandscapeComponent The landscape component to create the proxy from
 */
FLandscapeHeightProxy::FLandscapeHeightProxy(const ULandscapeHeightfieldCollisionComponent* InLandscapeComponent)
{
	Init(InLandscapeComponent);
}

/**
 * Initializes the height proxy
 * @param InLandscapeComponent The landscape component to create the proxy from
 */
void FLandscapeHeightProxy::Init(const ULandscapeHeightfieldCollisionComponent* InLandscapeComponent)
{
	checkf(InLandscapeComponent, TEXT("Landscape component is null"));
	
	Transform = InLandscapeComponent->GetNavigableGeometryTransform();
	LandscapeHeightGrid = FGrid2D(*InLandscapeComponent);
	
	const FVector Size = Transform.GetScale3D();
	
	// Get landscape collision height data
	const uint16* CollisionHeightData = (uint16*)InLandscapeComponent->CollisionHeightData.LockReadOnly();
	const int64 ElementCount = InLandscapeComponent->CollisionHeightData.GetElementCount();
	
	CollisionHeights.SetNumUninitialized(ElementCount);

	// Read the height data and calculate the heights
	for(int64 i = 0; i < ElementCount; i++)
	{
		const uint16 HeightValue = CollisionHeightData[i];
		const double Height = FMath::Lerp(-256.0, 255.992, static_cast<double>(HeightValue) / static_cast<double>(TNumericLimits<uint16>::Max())) * Size.Z;
		CollisionHeights[i] = Height;
	}

	InLandscapeComponent->CollisionHeightData.Unlock();
}

FGrid2D FLandscapeHeightProxy::GetGrid() const
{
	return LandscapeHeightGrid;
}

FGrid2D& FLandscapeHeightProxy::GetGrid()
{
	return LandscapeHeightGrid;
}

const FGrid2D& FLandscapeHeightProxy::GetGridConst() const
{
	return LandscapeHeightGrid;
}

/**
 * Gets the nearest height sample at an index
 * @param Index The index to get the height from
 * @return The height at the input index
 */
double FLandscapeHeightProxy::GetHeight(const int32 Index) const
{
	checkf(CollisionHeights.IsValidIndex(Index), TEXT("Index out of bounds"));
	
	return CollisionHeights[Index];
}

/**
 * Gets the nearest height sample at a coordinate
 * @param Coordinate The coordinate to get the height from
 * @return The height at the input coordinate
 */
double FLandscapeHeightProxy::GetHeight(const FIntPoint& Coordinate) const
{
	checkf(LandscapeHeightGrid.IsCellCoordinateValid(Coordinate), TEXT("Coordinate is not valid"));

	const int32 Index = LandscapeHeightGrid.GetCellIndex(Coordinate);

	return GetHeight(Index);
}

/**
 * Gets the nearest height sample at a location
 * @param InLocation The world space location to sample
 * @return The height at the input location
 */
double FLandscapeHeightProxy::GetHeight(const FVector& InLocation) const
{
    checkf(LandscapeHeightGrid.IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));

	const int32 Index = LandscapeHeightGrid.GetCellIndex(InLocation);
    
    return GetHeight(Index);
}

/**
 * @param InLocation The world space location to sample
 * @return The largest of the 4 heights that surround the current position
 */
double FLandscapeHeightProxy::GetMaxHeight(const FVector& InLocation) const
{
	checkf(LandscapeHeightGrid.IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));

	const FHeightQuadrantArray Heights = GetHeights(InLocation);
	double MaxHeight = Heights[0];
	
	for(int32 i = 1; i < Heights.Num(); i++)
    {
        MaxHeight = FMath::Max(MaxHeight, Heights[i]);
    }

	return MaxHeight;
}

/**
 * 
 * @param InLocation The wrold space location to sample
 * @return The smallest of the 4 heights that surround the current position
 */
double FLandscapeHeightProxy::GetMinHeight(const FVector& InLocation) const
{
	checkf(LandscapeHeightGrid.IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));

	const FHeightQuadrantArray Heights = GetHeights(InLocation);
	double MinHeight = Heights[0];
	
	for(int32 i = 1; i < Heights.Num(); i++)
	{
		MinHeight = FMath::Min(MinHeight, Heights[i]);
	}
	
	return MinHeight;
}

/**
 * 
 * @param InLocation The world space location
 * @return The average of the 4 heights surrounding the current position
 */
double FLandscapeHeightProxy::GetMeanHeight(const FVector& InLocation) const
{
	checkf(LandscapeHeightGrid.IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));

	const FHeightQuadrantArray Heights = GetHeights(InLocation);
	const double MeanHeight = (Heights[0] + Heights[1] + Heights[2] + Heights[3]) / 4.0;
	
	return MeanHeight;
}

/**
 * 
 * @param InLocation The world space location
 * @return The interpolated height based on the input's relative distance from the 4 surrounding heights
 */
double FLandscapeHeightProxy::GetInterpolatedHeight(const FVector& InLocation) const
{
	checkf(LandscapeHeightGrid.IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));

	const FCenterQuadrantArray Centers = LandscapeHeightGrid.GetCellCentersQuadrantFromLocation(InLocation);

	// TODO: get percentage of distance from the center of the cells

	const FHeightQuadrantArray Heights = GetHeights(InLocation);

	// TODO: interpolate

	return 0;
}

/**
 * @param InLocation The world space location
 * @return The 4 heights surrounding the input location
 */
FHeightQuadrantArray FLandscapeHeightProxy::GetHeights(const FVector& InLocation) const
{
	// TODO: Get the nearest 4 heights
	checkf(LandscapeHeightGrid.IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));
	
	const FIndexQuadrantArray Indices = LandscapeHeightGrid.GetCellIndicesQuadrantFromLocation(InLocation);

	const FHeightQuadrantArray Heights = {
		GetHeight(Indices[0]),
		GetHeight(Indices[1]),
		GetHeight(Indices[2]),
		GetHeight(Indices[3])
	};
	
	return Heights;
}

/**
 * @param InBounds The bounds to get the heights from
 * @param OutLocalGrid The local grid that the data is for
 * @return Returns all heights that exist within the input bounds
 */
TArray<double> FLandscapeHeightProxy::GetHeights(const FBox& InBounds, FGrid2D& OutLocalGrid) const
{
	// TODO: Get the heights for all the landscape heights in the bounds
	
	return TArray<double>();
}

FBox FLandscapeHeightProxy::GetBounds() const
{
	return LandscapeHeightGrid.GetBounds();
}
