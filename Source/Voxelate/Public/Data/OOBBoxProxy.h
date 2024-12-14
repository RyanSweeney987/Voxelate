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
#include "PhysicsEngine/BoxElem.h"
#include "OOBBoxProxy.generated.h"


/**
 * Based on code found here:
 * https://forums.unrealengine.com/t/object-oriented-bounding-box-from-either-aactor-or-mesh/326571/7
 */
USTRUCT()
struct VOXELATE_API FOOBBoxProxy
{
	GENERATED_BODY()

	// Center of the OBB in world space
	FVector Center = FVector::ZAxisVector;
	// Half-size extents along each of the OBB's local axes
	FVector Extents = FVector::ZAxisVector;
	// Orientation of the OBB as a quaternion
	FQuat Orientation = FQuat::Identity;
	// Choose to keep the current rotation or handle rotation interpolation
	bool bSlerpRotation = false;
	
	FOOBBoxProxy() = default;
	FOOBBoxProxy(const FBoxSphereBounds& LocalBounds, const FTransform& InstanceTransform, const bool& InSlerpRotation = false);
	FOOBBoxProxy(const FBox& LocalBounds, const FTransform& InstanceTransform, const bool& InSlerpRotation = false);
	FOOBBoxProxy(const FKBoxElem& BoxElement, const FTransform& InstanceTransform, const bool& InSlerpRotation = false);

	FVector GetCenter() const;
	FVector GetExtent() const;
	FVector GetSize() const;
	
	void GetAxis(FVector& OutAxisX, FVector& OutAxisY, FVector& OutAxisZ) const;
	void GetCorners(TArray<FVector>& OutCorners) const;
	
	FOOBBoxProxy& operator+=(const FOOBBoxProxy& Other);
	
	bool IsInsideOrOn(const FVector& Point) const;
	bool IsInsideOrOn(const FOOBBoxProxy& Other) const;
	// bool IsInsideOrOn(const FBox& Other) const;
	
	bool Intersect(const FOOBBoxProxy& Other) const;
	// bool Intersect(const FBox& Other) const;
	
	FTransform ToTransform() const;

	static void DrawOBB(const UWorld* World, const FOOBBoxProxy& InOBB, const FColor& InColor, const bool bPersistentLines = false, const float LifeTime = -1.0, const uint8 DepthPriority = 0, const float Thickness = 0.f)
	{
		TArray<FVector> Corners;
		InOBB.GetCorners(Corners);

		// Optionally, visualize the OBB using debug drawing
		for (int32 i = 0; i < 8; ++i)
		{
			for (int32 j = i + 1; j < 8; ++j)
			{
				if (FMath::CountBits(i ^ j) == 1) // Edges differ by one bit
				{
					DrawDebugLine(
						World,
						Corners[i],
						Corners[j],
						InColor,
						bPersistentLines,
						LifeTime,
						DepthPriority,
						Thickness
					);
				}
			}
		}
	}
};