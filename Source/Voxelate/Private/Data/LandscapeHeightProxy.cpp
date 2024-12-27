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
	ComponentSize = InLandscapeComponent->CollisionSizeQuads;
	LandscapeComponentBounds = InLandscapeComponent->Bounds.GetBox();
	
	// The size results in a 63x63 grid if using the default 63x63 landscape component
	// Number of actual height data points is then 64x64
	const FVector Size = Transform.GetScale3D();
	LandscapeHeightGrid = FGrid2D(Size, InLandscapeComponent->Bounds.GetBox());
	
	// Get landscape collision height data
	const uint16* CollisionHeightData = (uint16*)InLandscapeComponent->CollisionHeightData.LockReadOnly();
	const int64 ElementCount = InLandscapeComponent->CollisionHeightData.GetElementCount();
	
	CollisionHeights.SetNumUninitialized(ElementCount);

	// https://dev.epicgames.com/documentation/en-us/unreal-engine/landscape-technical-guide-in-unreal-engine#calculatingheightmapzscale
	
	// Read the height data and calculate the heights
	for(int64 i = 0; i < ElementCount; i++)
	{
		const uint16 HeightValue = CollisionHeightData[i];
		const double Height = FMath::Lerp(-256.0, 256.0, static_cast<double>(HeightValue) / static_cast<double>(TNumericLimits<uint16>::Max())) * Size.Z;
		CollisionHeights[i] = Height;
	}

	InLandscapeComponent->CollisionHeightData.Unlock();
}

int32 FLandscapeHeightProxy::GetComponentSize() const
{
	return ComponentSize;
}

TArray<double> FLandscapeHeightProxy::GetCollisionHeights()
{
	return CollisionHeights;
}

const TArray<double>& FLandscapeHeightProxy::GetCollisionHeightsConst() const
{
	return CollisionHeights;
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

int32 FLandscapeHeightProxy::GetHeightIndex(const FVector& InLocation) const
{
	checkf(LandscapeHeightGrid.IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));

	const FIntPoint Coordinate = LandscapeHeightGrid.GetCellCoordinate(InLocation);
	
	return Coordinate.X + Coordinate.Y * (ComponentSize + 1);
}

/**
 * Gets the height at an index from the collision height data
 * @param HeightIndex The index based on the height data to get the height from
 * @return The height at the input index
 * @see GetHeightIndex
 */
double FLandscapeHeightProxy::GetHeight(const int32 HeightIndex) const
{
	checkf(CollisionHeights.IsValidIndex(HeightIndex), TEXT("Index out of bounds"));
	
	return CollisionHeights[HeightIndex];
}

/**
 * Gets the height at an index from the collision height data with the Z translation applied
 * @param HeightIndex The index based on the height data to get the height from
 * @return The height at the input index
 * @see GetHeightIndex
 */
double FLandscapeHeightProxy::GetHeightTranslated(const int32 HeightIndex) const
{
	checkf(CollisionHeights.IsValidIndex(HeightIndex), TEXT("Index out of bounds"));
	
	return CollisionHeights[HeightIndex] + Transform.GetLocation().Z;
}

/**
 * @return The Z translation of the landscape component
 */
double FLandscapeHeightProxy::GetHeightTranslation() const
{
	return Transform.GetLocation().Z;
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
	
	// Centers are the same as the height vertices on the XY plane
	const FBox CellBounds = LandscapeHeightGrid.GetCellBounds(InLocation);
	
	const FVector MinVector = CellBounds.Min;
	const FVector MaxVector = CellBounds.Max - MinVector;
	const FVector RelativeDistance = InLocation - MinVector;
 
	const double XInterp = RelativeDistance.X / MaxVector.X;
	const double YInterp = RelativeDistance.Y / MaxVector.Y;
	
	const FHeightQuadrantArray Heights = GetHeights(InLocation);
 
	const double InterpolatedHeight = FMath::Lerp(
        FMath::Lerp(Heights[0], Heights[1], XInterp),
        FMath::Lerp(Heights[2], Heights[3], XInterp),
        YInterp
    );
 
	return InterpolatedHeight;
}

/**
 * Order is always the following - A is the minimum location, D is the maximum location:
 * ---------
 * | C | D |
 * ---------
 * | A | B |
 * ---------
 * @param InLocation The world space location
 * @return The 4 heights surrounding the input location
 */
FHeightQuadrantArray FLandscapeHeightProxy::GetHeights(const FVector& InLocation) const
{
	checkf(LandscapeHeightGrid.IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));

	const FIntPoint Cell = LandscapeHeightGrid.GetCellCoordinate(InLocation);

	const int32 Width = ComponentSize + 1;
	
	const int32 IndexA = Cell.X + Cell.Y * Width;
	const int32 IndexB = IndexA + 1;
	const int32 IndexC = IndexA + Width;
	const int32 IndexD = IndexC + 1;
	
	const FHeightQuadrantArray Heights = {
		GetHeight(IndexA),
		GetHeight(IndexB),
		GetHeight(IndexC),
		GetHeight(IndexD)
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

FHeightQuadrantArray FLandscapeHeightProxy::GetHeightIndices(const FVector& InLocation) const
{
	const FIntPoint Cell = LandscapeHeightGrid.GetCellCoordinate(InLocation);
	
	const int32 IndexA = Cell.X + Cell.Y * ComponentSize;
	const int32 IndexB = IndexA + 1;
	const int32 IndexC = IndexA + ComponentSize;
	const int32 IndexD = IndexC + 1;

	return {
		IndexA,
		IndexB,
		IndexC,
		IndexD
	};
}

FBox FLandscapeHeightProxy::GetLandscapeComponentBounds() const
{
	return LandscapeComponentBounds;
}

FBox FLandscapeHeightProxy::GetGridBounds() const
{
	return LandscapeHeightGrid.GetBounds();
}
