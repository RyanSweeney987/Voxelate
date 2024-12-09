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
#include "PhysicsEngine/SphylElem.h"
#include "UObject/Object.h"
#include "CapsuleProxy.generated.h"

/**
 * 
 */
USTRUCT()
struct VOXELATE_API FCapsuleProxy
{
	GENERATED_BODY()

	FVector Start = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	double Radius = 0.0;
public:
	FCapsuleProxy() = default;
	FCapsuleProxy(const FVector& InStart, const FVector& InEnd, const double InRadius);
	FCapsuleProxy(const FKSphylElem& InCapsuleElement, const FTransform& InTransform);

	bool Intersects(const FBox& Other) const;

protected:
	// Helper struct to project a point onto a line segment
	struct FPointLineProjection
	{
		FVector ProjectedPoint = FVector::ZeroVector;
		int32 RelationToSegment = 0;

		/**
		 * Project a point to a line segment
		 * @param LineStart The start of the line segment
		 * @param LineEnd The end of the line segment
		 * @param Point The point to project
		 * @return The projected point struct containing the relation to the segment and the projected point
		 */
		static FPointLineProjection ProjectPointToLineSegment(const FVector& LineStart, const FVector& LineEnd,
															 const FVector& Point)
		{
			// https://stackoverflow.com/a/6853926/8204221
			const FVector PointOffset = Point - LineStart;
			const FVector Edge = LineEnd - LineStart;

			const double LineLength = Edge.SquaredLength();

			if(LineLength < DBL_EPSILON)
			{
				return FPointLineProjection();
			}
	
			const double Dot = PointOffset.Dot(Edge);
			const double RelationToSegment = Dot / LineLength;

			FPointLineProjection Result;
			Result.RelationToSegment = RelationToSegment < 0 ? -1 : (RelationToSegment > 1 ? 1 : 0);
	
			if(RelationToSegment < 0)
			{
				Result.ProjectedPoint = LineStart;
			} else if(RelationToSegment > 1)
			{
				Result.ProjectedPoint = LineEnd;
			} else
			{
				Result.ProjectedPoint = LineStart + (RelationToSegment * Edge);
			}
	
			return Result;
		}
	};
};
