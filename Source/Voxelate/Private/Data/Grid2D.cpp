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

FGrid2D::FGrid2D(const FVector2d& InCellSize, const FBox& InBounds)
{
}

FGrid2D::FGrid2D(const ULandscapeHeightfieldCollisionComponent& InLandscapeComponent)
{
}

FGrid2D::FGrid2D(const FGrid2D& InCellGrid, const FBox& InBounds)
{
}

void FGrid2D::Init(const FVector2d& InCellSize, const FBox& InBounds)
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
}

int32 FGrid2D::GetCellCount() const
{
}

FIntPoint FGrid2D::GetVectorCellCount() const
{
}

TOptional<FIntPoint> FGrid2D::GetOffset() const
{
}

bool FGrid2D::IsCellIndexValid(const int32 InIndex) const
{
}

bool FGrid2D::IsCellCoordinateValid(const FIntPoint& InCoordinate) const
{
}

bool FGrid2D::IsLocationInBounds(const FVector2d& InLocation) const
{
}

bool FGrid2D::IsGridInside(const FGrid2D& InCellGrid) const
{
}

int32 FGrid2D::GetCellIndex(const FVector2d& InLocation) const
{
}

int32 FGrid2D::GetCellIndex(const FIntPoint& InCoordinate) const
{
}

FIntPoint FGrid2D::GetCellCoordinate(const FVector2d& InLocation) const
{
}

FIntPoint FGrid2D::GetCellCoordinate(const int32 InIndex) const
{
}

FBox FGrid2D::GetCellBounds(const int32 InIndex) const
{
}

FBox FGrid2D::GetCellBounds(const FIntPoint& InCoordinate) const
{
}

FBox FGrid2D::GetCellBounds(const FVector2d& InLocation) const
{
}

TArray<int32> FGrid2D::GetCellIndicesFromBounds(const FBox& InBounds) const
{
}

TArray<FIntPoint> FGrid2D::GetCellCoordinatesFromBounds(const FBox& InBounds) const
{
}

FGrid2D FGrid2D::GetSubGrid(const FBox& InBounds) const
{
}

bool FGrid2D::operator==(const FGrid2D& InCellGrid) const
{
}

bool FGrid2D::operator!=(const FGrid2D& InCellGrid) const
{
}
