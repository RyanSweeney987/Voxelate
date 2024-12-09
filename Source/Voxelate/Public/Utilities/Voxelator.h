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
#include "PhysicsEngine/BoxElem.h"
#include "PhysicsEngine/ConvexElem.h"
#include "Voxelator.generated.h"

/**
 * TODO: Implement functionality to voxelate the world based on navigable geometry
 * TODO: Implement multithreaded batching
 * TODO: Implement a way to efficiently visualize the voxelated results - debug draw too expensive
 * TODO: Figure out the best way to store the voxelated results? Here? Some other object? Sparse Voxel Tree?
 */
USTRUCT()
struct VOXELATE_API FVoxelator
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
public:
	FVoxelator(UWorld* InWorld);
	
	TArray<bool> VoxelateNavigableGeometry(const FVoxelGrid& InVoxelGrid);

private:
	void ProcessPrimitiveComponent(UPrimitiveComponent* InPrimitiveComponent, const FVoxelGrid& InVoxelGrid);

	void ProcessLandscape(ULandscapeHeightfieldCollisionComponent& LandscapeComponent, const FVoxelGrid& LocalVoxelGrid, const FTransform& InstanceTransform);
	
	void ProcessCollisionBox(const FKBoxElem& BoxElement, const FVoxelGrid& LocalVoxelGrid, const FTransform& InstanceTransform);
	void ProcessCollisionSphere(const FKSphereElem& SphereElement, const FVoxelGrid& LocalVoxelGrid, const FTransform& InstanceTransform);
	void ProcessCollisionCapsule(const FKSphylElem& CapsuleElement, const FVoxelGrid& LocalVoxelGrid, const FTransform& InstanceTransform);
	void ProcessCollisionConvex(const FKConvexElem& ConvexElement, const FVoxelGrid& LocalVoxelGrid, const FTransform& InstanceTransform);
};

