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
#include "LandscapeProxy.h"
#include "Data/CapsuleProxy.h"
#include "Data/OOBBoxProxy.h"
#include "Data/SphereProxy.h"
#include "Data/TriangleProxy.h"
#include "Data/VoxelGrid.h"
#include "Voxelator.generated.h"

/**
 * 
 */
USTRUCT()
struct VOXELATE_API FVoxelator
{
	GENERATED_BODY()
	
public:
	FVoxelator() = default;

	TArray<bool> VoxelateWorld(UWorld* InWorld, const FVoxelGrid& InVoxelGrid);
	TArray<bool> VoxelateActor(AActor* InActor, const FVoxelGrid& InVoxelGrid);
	TArray<bool> VoxelateStaticMesh(UStaticMesh* InComponent, const FVoxelGrid& InVoxelGrid);
	TArray<bool> VoxelatePhysicsBody(UPrimitiveComponent* InComponent, const FVoxelGrid& InVoxelGrid);
	TArray<bool> VoxelateLandscape(ALandscapeProxy* InLandscape, const FVoxelGrid& InVoxelGrid);
	TArray<bool> VoxelateNavigableGeometry(UWorld* InWorld, const FVoxelGrid& InVoxelGrid);

	bool AABBIntersectsTriangle(const FBox& InAABB, const FTriangleProxy& InTriangle);
	bool AABBIntersectsOBB(const FBox& InAABB, const FOOBBoxProxy& InOOBB);
	bool AABBIntersectsSphere(const FBox& InAABB, const FSphereProxy& InSphere);
	bool AABBIntersectsCapsule(const FBox& InAABB, const FCapsuleProxy& InCapsule);
	bool AABBIntersectsAABB(const FBox& InAABB1, const FBox& InAABB2);
};

