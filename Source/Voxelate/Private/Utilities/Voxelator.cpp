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


#include "Utilities/Voxelator.h"

#include "Engine/OverlapResult.h"
#include "PhysicsEngine/BodySetup.h"


void FVoxelator::Init(UWorld* InWorld)
{
	World = InWorld;
}

/**
 * Voxelate a single actor in the world
 * @param InActor the actor to voxelate
 * @param OutVoxelData the voxel grid to populate
 * @return Array of booleans representing the voxelated actor
 */
void FVoxelator::VoxelateActor(const AActor* InActor, FVoxelData& OutVoxelData) const
{
	const FVoxelGrid& InVoxelGrid = OutVoxelData.GetVoxelGrid();

	// Get the primitive components of the actor - these may have collision geometry
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	InActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for(const auto Component : PrimitiveComponents)
	{
		if(Component)
		{
			FVoxelGrid LocalVoxelGrid = InVoxelGrid.GetSubGrid(Component->Bounds.GetBox());
			FVoxelData LocalVoxelData(LocalVoxelGrid);

			// Process the primitive component
			ProcessPrimitiveComponent(*Component, LocalVoxelData);

			// Apply the local voxel data to the output voxel data
			OutVoxelData.And(LocalVoxelData);
		}
	}
}

/**
 * Voxelate the navigable geometry of the world
 * @param OutVoxelData the voxel grid data to populate
 * @return Array of booleans representing the voxelated world
 */
void FVoxelator::VoxelateNavigableGeometry(FVoxelData& OutVoxelData) const
{
	checkf(World, TEXT("World is null"));

	const FVoxelGrid& InVoxelGrid = OutVoxelData.GetVoxelGrid();
	
	const FVector& BoxOrigin = InVoxelGrid.GetBounds().GetCenter();
	const FVector& BoxExtent = InVoxelGrid.GetBounds().GetExtent();

	// Gets all overlapping objects in the world
	TArray<FOverlapResult> OverlappingActorsResults;
	World->OverlapMultiByObjectType(OverlappingActorsResults, BoxOrigin, FQuat::Identity, FCollisionObjectQueryParams::AllObjects, FCollisionShape::MakeBox(BoxExtent));

	// TODO: Return occupancy results

	// Iterate over each overlapping actor - only process navigation relevant components
	for(const auto OverlapResult : OverlappingActorsResults)
	{
		if(OverlapResult.GetComponent()->IsNavigationRelevant())
		{
			if(UPrimitiveComponent* PrimitiveComponent = OverlapResult.GetComponent(); PrimitiveComponent && PrimitiveComponent->IsNavigationRelevant())
			{
				FVoxelGrid LocalVoxelGrid = InVoxelGrid.GetSubGrid(PrimitiveComponent->GetNavigationBounds());
				FVoxelData LocalVoxelData(LocalVoxelGrid);
				
				ProcessPrimitiveComponent(*PrimitiveComponent, LocalVoxelGrid);
			}
		}
	}
}

void FVoxelator::ProcessPrimitiveComponent(UPrimitiveComponent& InPrimitiveComponent, const FVoxelData& InVoxelData) const
{
	// Check for Landscape
	if(InPrimitiveComponent.IsA(ULandscapeHeightfieldCollisionComponent::StaticClass()))
	{
		ULandscapeHeightfieldCollisionComponent* LandscapeComponent = Cast<ULandscapeHeightfieldCollisionComponent>(&InPrimitiveComponent);
		ProcessLandscape(*LandscapeComponent, InVoxelData);

		return;
	} 

	if(const UBodySetup* BodySetup = InPrimitiveComponent.GetBodySetup(); BodySetup)
	{
		const FKAggregateGeom& AggGeom = BodySetup->AggGeom;

		for(const auto& BoxElem : AggGeom.BoxElems)
		{
			ProcessCollisionBox(BoxElem, InVoxelData, InPrimitiveComponent.GetNavigableGeometryTransform());
		}
		
		for(const auto& SphereElem : AggGeom.SphereElems)
		{
			ProcessCollisionSphere(SphereElem, InVoxelData, InPrimitiveComponent.GetNavigableGeometryTransform());
		}

		for(const auto& SphylElem : AggGeom.SphylElems)
		{
			ProcessCollisionCapsule(SphylElem, InVoxelData, InPrimitiveComponent.GetNavigableGeometryTransform());
		}

		for(const auto& ConvexElem : AggGeom.ConvexElems)
		{
			ProcessCollisionConvex(ConvexElem, InVoxelData, InPrimitiveComponent.GetNavigableGeometryTransform());
		}
	}
}

void FVoxelator::ProcessLandscape(ULandscapeHeightfieldCollisionComponent& LandscapeComponent, const FVoxelData& InVoxelData) const
{
	const FIntVector LocalGridSize = InVoxelData.GetVoxelGridConst().GetVectorVoxelCount();

	const FVoxelGrid LandscapeVoxelGrid(LandscapeComponent);
	// DrawDebugBox(World, LandscapeVoxelGrid.GetBounds().GetCenter(), LandscapeVoxelGrid.GetBounds().GetExtent(), FColor::Purple, false, 5.0f);
	
	// https://dev.epicgames.com/documentation/en-us/unreal-engine/landscape-technical-guide-in-unreal-engine#calculatingheightmapzscale

	const FTransform InstanceTransform = LandscapeComponent.GetNavigableGeometryTransform();
	
	const int32 ComponentSize = LandscapeComponent.CollisionSizeQuads;
	const FVector Size = InstanceTransform.GetScale3D();
	const FVector Location = InstanceTransform.GetLocation();

	// Get landscape collision height data
	const uint16* CollisionHeightData = (uint16*)LandscapeComponent.CollisionHeightData.LockReadOnly();
	const int64 ElementCount = LandscapeComponent.CollisionHeightData.GetElementCount();
	
	TArray<double> CollisionHeights;
	CollisionHeights.SetNumUninitialized(ElementCount);
	
	for(int64 i = 0; i < ElementCount; i++)
	{
		const uint16 HeightValue = CollisionHeightData[i];
		const double Height = FMath::Lerp(-256.0, 255.992, static_cast<double>(HeightValue) / static_cast<double>(TNumericLimits<uint16>::Max())) * Size.Z;
		CollisionHeights[i] = Height;
	}	
	
	LandscapeComponent.CollisionHeightData.Unlock();

	// Draw the collision heights
	for(int32 Y = 0; Y < ComponentSize; Y++)
	{
		for(int32 X = 0; X < ComponentSize; X++)
		{
			// const FVector Current = FVector(X * Size.X, Y * Size.Y, CollisionHeights[X + Y * (ComponentSize + 1)]);
			// const FVector Right = FVector((X + 1) * Size.X, Y * Size.Y, CollisionHeights[(X + 1) + Y * (ComponentSize + 1)]);
			// const FVector Up = FVector(X * Size.X, (Y + 1) * Size.Y, CollisionHeights[X + (Y + 1) * (ComponentSize + 1)]);
			// const FVector Diagonal = FVector((X + 1) * Size.X, (Y + 1) * Size.Y, CollisionHeights[(X + 1) + (Y + 1) * (ComponentSize + 1)]);
			//
			// const FVector Min = FVector::Min(Current, FVector::Min(Right, FVector::Min(Up, Diagonal)));
			// const FVector Max = FVector::Max(Current, FVector::Max(Right, FVector::Max(Up, Diagonal)));
			//
			// const FBox Bounds = FBox(Min, Max);
			//
			// DrawDebugBox(World, Bounds.GetCenter(), Bounds.GetExtent(), FColor::Green, false, 5.0f);
			
			const FVector Start = FVector(X * Size.X, Y * Size.Y, CollisionHeights[X + Y * (ComponentSize + 1)]);
			const FVector StartWorldLocation = Start + Location;
			
			// DrawDebugBox(World, StartWorldLocation, FVector(10, 10, 10), FColor::Cyan, false, 5.0f);
		}
	}
	
	for(int32 Y = 0; Y < LandscapeVoxelGrid.GetVectorVoxelCount().Y; Y++)
	{
		for(int32 X = 0; X < LandscapeVoxelGrid.GetVectorVoxelCount().X; X++)
		{
			FBox VoxelBounds = LandscapeVoxelGrid.GetVoxelBounds(FIntVector(X, Y, 0));
			// DrawDebugBox(World, VoxelBounds.GetCenter(), VoxelBounds.GetExtent(), FColor::Blue, false, 5.0f);
		}
	}

	for(int32 Y = 0; Y < LocalGridSize.Y; Y++)
	{
		for(int32 X = 0; X < LocalGridSize.X; X++)
		{
	
 		}
	}
}

void FVoxelator::ProcessCollisionBox(const FKBoxElem& BoxElement, const FVoxelData& InVoxelData,
	const FTransform& InstanceTransform) const
{
	const FVoxelGrid& LocalVoxelGrid = InVoxelData.GetVoxelGridConst();
	const FIntVector LocalGridSize = LocalVoxelGrid.GetVectorVoxelCount();

	// FByteBulkData BulkData;
	
	// Use OOBB for collision detection of the box collision element
	const FOOBBoxProxy BoxProxy(BoxElement, InstanceTransform, true);
	// TODO: Needs more testing but works ok if going from the AABB to OOBB rather than OOBB to AABB
	// TODO: Alternatively (but slower), could get the 8 corners, make triangles and use the same method as the convex mesh
	
	// Iterate over the local grid along the horizontal XY plane
	for(int32 Y = 0; Y < LocalGridSize.Y; Y++)
	{
		for(int32 X = 0; X < LocalGridSize.X; X++)
		{
			// Iterate over each voxel in the local grid along the vertical Z axis
			for(int32 Z = 0; Z < LocalGridSize.Z; Z++)
			{
				const FBox VoxelBounds = LocalVoxelGrid.GetVoxelBounds(FIntVector(X, Y, Z));
				const FOOBBoxProxy VoxelBoundsProxy(VoxelBounds, FTransform::Identity, false);
				
				bool bIntersects = false;

				if(VoxelBoundsProxy.IsInsideOrOn(BoxProxy) || VoxelBoundsProxy.Intersect(BoxProxy))
                {
					// TODO: Add the voxel to the voxel grid
                    bIntersects = true;
                }
				
				// Update the span min and max if the box bounds intersect with the OOBB voxel bounds
				if (bIntersects)
				{
					DrawDebugBox(World, VoxelBoundsProxy.GetCenter(), VoxelBoundsProxy.GetExtent(), FColor::Green, false, 5.0f);
				}
			}
		}
	}
}

void FVoxelator::ProcessCollisionSphere(const FKSphereElem& SphereElement, const FVoxelData& InVoxelData,
	const FTransform& InstanceTransform) const
{
	// AABB test
	const FVoxelGrid& LocalVoxelGrid = InVoxelData.GetVoxelGridConst();
	const FIntVector LocalGridSize = LocalVoxelGrid.GetVectorVoxelCount();

	const FVector SphereCenter = InstanceTransform.TransformPosition(SphereElement.Center);
	const FVector Scale = InstanceTransform.GetScale3D();
	// Adjust the radius by the scaling factor - using the smallest scale factor as in the engine
	const double SphereRadius = SphereElement.Radius * FMath::Min3(Scale.X, Scale.Y, Scale.Z);

	DrawDebugSphere(World, SphereCenter, SphereRadius, 12, FColor::Red, false, 5.0f);
	
	for(int32 Y = 0; Y < LocalGridSize.Y; Y++)
	{
		for(int32 X = 0; X < LocalGridSize.X; X++)
		{
			for(int32 Z = 0; Z < LocalGridSize.Z; Z++)
			{
				bool bIntersects = false;

				const FBox VoxelBounds = LocalVoxelGrid.GetVoxelBounds(FIntVector(X, Y, Z));
				
				if(FMath::SphereAABBIntersection(SphereCenter, FMath::Square(SphereRadius), VoxelBounds))
				{
					bIntersects = true;
				}

				if (bIntersects)
				{
					DrawDebugBox(World, VoxelBounds.GetCenter(), VoxelBounds.GetExtent(), FColor::Green, false, 5.0f);
				}
			}
		}
	}
}

void FVoxelator::ProcessCollisionCapsule(const FKSphylElem& CapsuleElement, const FVoxelData& InVoxelData,
	const FTransform& InstanceTransform) const
{
	// AABB test
	const FVoxelGrid& LocalVoxelGrid = InVoxelData.GetVoxelGridConst();
	const FIntVector LocalGridSize = LocalVoxelGrid.GetVectorVoxelCount();

	const FCapsuleProxy CapsuleProxy(CapsuleElement, InstanceTransform);

	for(int32 Y = 0; Y < LocalGridSize.Y; Y++)
	{
		for(int32 X = 0; X < LocalGridSize.X; X++)
		{
			for(int32 Z = 0; Z < LocalGridSize.Z; Z++)
			{
				bool bIntersects = false;

				const FBox VoxelBounds = LocalVoxelGrid.GetVoxelBounds(FIntVector(X, Y, Z));

				if(CapsuleProxy.Intersects(VoxelBounds))
                {
                    bIntersects = true;
                }
				
				if (bIntersects)
				{
					DrawDebugBox(World, VoxelBounds.GetCenter(), VoxelBounds.GetExtent(), FColor::Green, false, 5.0f);
				}
			}
		}
	}
}

void FVoxelator::ProcessCollisionConvex(const FKConvexElem& ConvexElement, const FVoxelData& InVoxelData,
	const FTransform& InstanceTransform) const
{
	const TArray<int32> Indices = ConvexElement.IndexData;
	const TArray<FVector> Vertices = ConvexElement.VertexData;
	
	TArray<FTriangleProxy> Triangles;
	Triangles.Reserve(Indices.Num() / 3);

	for(int32 i = 0; i < Indices.Num(); i += 3)
	{
		Triangles.Add({
			InstanceTransform.TransformPosition(Vertices[Indices[i]]), 
			InstanceTransform.TransformPosition(Vertices[Indices[i + 1]]),
			InstanceTransform.TransformPosition(Vertices[Indices[i + 2]])
		});
	}
	
	const FVoxelGrid& LocalVoxelGrid = InVoxelData.GetVoxelGridConst();
	const FIntVector LocalGridSize = LocalVoxelGrid.GetVectorVoxelCount();
		
	for(int32 Y = 0; Y < LocalGridSize.Y; Y++)
	{
		for(int32 X = 0; X < LocalGridSize.X; X++)
		{
			for(int32 Z = 0; Z < LocalGridSize.Z; Z++)
			{
				const FBox VoxelBounds = LocalVoxelGrid.GetVoxelBounds(FIntVector(X, Y, Z));
				
				bool bIntersects = false;
				
				for(const auto& Triangle : Triangles)
				{
					if(Triangle.Intersects(VoxelBounds))
                    {
                        bIntersects = true;
                        break;
                    }
				}

				if (bIntersects)
				{
					DrawDebugBox(World, VoxelBounds.GetCenter(), VoxelBounds.GetExtent(), FColor::Green, false, 5.0f);
				}
			}
		}
	}
}

void FVoxelator::ProcessConvex() const
{
}
