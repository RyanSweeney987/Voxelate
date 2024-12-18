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
	Transform = InLandscapeComponent->GetNavigableGeometryTransform();
	Bounds = InLandscapeComponent->Bounds.GetBox();
	
	const FVector Size = Transform.GetScale3D();

	HeightCellCount = FIntPoint(
		InLandscapeComponent->CollisionSizeQuads + 1,
		InLandscapeComponent->CollisionSizeQuads + 1);
	
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

double FLandscapeHeightProxy::GetHeight(const int32 Index) const
{
	checkf(CollisionHeights.IsValidIndex(Index), TEXT("Index out of bounds"));
	
	return CollisionHeights[Index];
}

double FLandscapeHeightProxy::GetHeight(const FIntPoint& Coordinate) const
{
	checkf(IsCoordinateValid(Coordinate), TEXT("Coordinate is not valid"));
	
	return CollisionHeights[Coordinate.X + Coordinate.Y * HeightCellCount.X];	
}

double FLandscapeHeightProxy::GetMaxHeight(const FVector2d& InLocation) const
{
	checkf(IsLocationInBounds(InLocation), TEXT("Location is not in bounds"));

	// TODO: Get the nearest 4 heights and get the max
}

double FLandscapeHeightProxy::GetMinHeight(const FVector2d& InLocation) const
{
	// TODO: Get the nearest 4 heights and get the min
}

double FLandscapeHeightProxy::GetMeanHeight(const FVector2d& InLocation) const
{
	// TODO: Get the nearest 4 heights and get the average
}

TArray<double> FLandscapeHeightProxy::GetHeights(const FVector2d& InLocation) const
{
	// TODO: Get the nearest 4 heights
}

TArray<double> FLandscapeHeightProxy::GetHeights(const FBox& InBounds) const
{
	// TODO: Get the heights for all the landscape heights in the bounds
}

FBox FLandscapeHeightProxy::GetBounds() const
{
	return HeightGrid.GetBounds();
}
