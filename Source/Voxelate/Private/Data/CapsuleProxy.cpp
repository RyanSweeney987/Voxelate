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

#include "Data/CapsuleProxy.h"

FCapsuleProxy::FCapsuleProxy(const FVector& InStart, const FVector& InEnd, const double InRadius) : Start(InStart), End(InEnd), Radius(InRadius)
{
}

FCapsuleProxy::FCapsuleProxy(const FKSphylElem& InCapsuleElement, const FTransform& InTransform) 
{
	Radius = InCapsuleElement.Radius;
	
	const FVector CapsuleCenter = InTransform.TransformPosition(InCapsuleElement.Center);
	const double CapsuleHalfHeight = InCapsuleElement.Length / 2;

	Start = CapsuleCenter + (InCapsuleElement.Rotation.RotateVector(InTransform.GetRotation().GetUpVector()) * CapsuleHalfHeight);
	End = CapsuleCenter - (InCapsuleElement.Rotation.RotateVector(InTransform.GetRotation().GetUpVector()) * CapsuleHalfHeight);
}

bool FCapsuleProxy::Intersects(const FBox& Other) const
{
	const FPointLineProjection Projection = FPointLineProjection::ProjectPointToLineSegment(Start, End, Other.GetCenter());

	// Sphere check on the ends
	if(Projection.RelationToSegment < 0) {
		return FMath::SphereAABBIntersection(Start, FMath::Square(Radius), Other);
	}

	if(Projection.RelationToSegment > 0)
	{
		return FMath::SphereAABBIntersection(End, FMath::Square(Radius), Other);
	}
	
	// Check along the segment
	const FVector NormalisedProjection = (Other.GetCenter() - Projection.ProjectedPoint).GetSafeNormal();
	return Other.IsInsideOrOn(Projection.ProjectedPoint + (NormalisedProjection * Radius));
}
