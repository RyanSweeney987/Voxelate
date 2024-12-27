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

#include "Data/LandscapeHeightProxy.h"
#include "Engine/OverlapResult.h"
#include "PhysicsEngine/BodySetup.h"


FVoxelator::FVoxelator(UWorld* InWorld) : World(InWorld)
{
}

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
	const FGrid3D& InVoxelGrid = OutVoxelData.GetVoxelGrid();

	// Get the primitive components of the actor - these may have collision geometry
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	InActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for(const auto Component : PrimitiveComponents)
	{
		if(Component)
		{
			FGrid3D LocalVoxelGrid = InVoxelGrid.GetSubGrid(Component->Bounds.GetBox());
			FVoxelData LocalVoxelData(LocalVoxelGrid);

			// Process the primitive component
			ProcessPrimitiveComponent(*Component, LocalVoxelData);

			// Apply the local voxel data to the output voxel data
			OutVoxelData.Or(LocalVoxelData);
		}
	}

	for(const auto Index : OutVoxelData.GetOccupiedIndices())
	{
		const FBox Bounds = InVoxelGrid.GetVoxelBounds(Index);
		DrawDebugBox(World, Bounds.GetCenter(), Bounds.GetExtent(), FColor::Green, false, 5.0f);
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

	const FGrid3D& InVoxelGrid = OutVoxelData.GetVoxelGrid();

	// DrawDebugBox(World, InVoxelGrid.GetBounds().GetCenter(), InVoxelGrid.GetBounds().GetExtent(), FColor::Blue, false, 5.0f);
	
	const FVector& BoxOrigin = InVoxelGrid.GetBounds().GetCenter();
	const FVector& BoxExtent = InVoxelGrid.GetBounds().GetExtent();

	// Gets all overlapping objects in the world
	TArray<FOverlapResult> OverlappingActorsResults;
	World->OverlapMultiByObjectType(OverlappingActorsResults, BoxOrigin, FQuat::Identity, FCollisionObjectQueryParams::AllObjects, FCollisionShape::MakeBox(BoxExtent));
	
	// Iterate over each overlapping actor - only process navigation relevant components
	for(const auto OverlapResult : OverlappingActorsResults)
	{
		if(OverlapResult.GetComponent()->IsNavigationRelevant())
		{
			if(UPrimitiveComponent* PrimitiveComponent = OverlapResult.GetComponent(); PrimitiveComponent && PrimitiveComponent->IsNavigationRelevant())
			{
				const FBox PrimCompBounds = PrimitiveComponent->Bounds.GetBox();
				const FBox OverlapBounds = InVoxelGrid.GetBoundsOverlap(PrimCompBounds);

				// We don't want any non-zero volume overlaps within an error tolerance
				if(OverlapBounds.GetVolume() <= DBL_EPSILON) 
				{
					continue;
				}
								
				FGrid3D LocalVoxelGrid = InVoxelGrid.GetSubGrid(OverlapBounds);
				FVoxelData LocalVoxelData(LocalVoxelGrid);
				
				// Process the primitive component
				ProcessPrimitiveComponent(*PrimitiveComponent, LocalVoxelData);
				
				// Apply the local voxel data to the output voxel data
				OutVoxelData.Or(LocalVoxelData);
			}
		}
	}

	for(const auto Index : OutVoxelData.GetOccupiedIndices())
	{
		const FBox Bounds = InVoxelGrid.GetVoxelBounds(Index);
		DrawDebugBox(World, Bounds.GetCenter(), Bounds.GetExtent(), FColor::Green, false, 5.0f);
	}
}

void FVoxelator::ProcessPrimitiveComponent(UPrimitiveComponent& InPrimitiveComponent, FVoxelData& InVoxelData) const
{
	// Check for Landscape
	if(InPrimitiveComponent.IsA(ULandscapeHeightfieldCollisionComponent::StaticClass()))
	{
		ULandscapeHeightfieldCollisionComponent* LandscapeComponent = Cast<ULandscapeHeightfieldCollisionComponent>(&InPrimitiveComponent);
		ProcessLandscape(*LandscapeComponent, InVoxelData);
	}

#if false

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

#endif
}

void FVoxelator::ProcessLandscape(ULandscapeHeightfieldCollisionComponent& LandscapeComponent, FVoxelData& InVoxelData) const
{
	// Landscape proxy to make sampling the height data easier
	FLandscapeHeightProxy LandscapeHeightProxy(&LandscapeComponent);
	const FGrid2D& LandscapeHeightProxyGrid = LandscapeHeightProxy.GetGridConst();
	
	TArray<double> CollisionHeights = LandscapeHeightProxy.GetCollisionHeights();

	// Local voxel grid used just for the landscape component
	const FGrid3D LocalVoxelGrid = InVoxelData.GetVoxelGrid().GetSubGrid(LandscapeHeightProxy.GetLandscapeComponentBounds());
	const FIntVector InGridSize = LocalVoxelGrid.GetVectorVoxelCount();
	
	for(int32 Y = 0; Y < InGridSize.Y; Y++)
	{
		for(int32 X = 0; X < InGridSize.X; X++)
		{
			const FBox VoxelBounds = LocalVoxelGrid.GetVoxelBounds(FIntVector(X, Y, 0));

			if(const FVector VoxelCenter = VoxelBounds.GetCenter(); LandscapeHeightProxyGrid.IsLocationInBounds(VoxelCenter))
			{
				const double InterpHeight = LandscapeHeightProxy.GetInterpolatedHeight(VoxelCenter) + LandscapeHeightProxy.GetHeightTranslation() + SMALL_NUMBER;

				if(const FVector VoxelLocation = FVector(VoxelCenter.X, VoxelCenter.Y, InterpHeight); LocalVoxelGrid.IsLocationInBounds(VoxelLocation))
				{
					InVoxelData.SetOccupancy(VoxelLocation, true);
				} else
				{
					DrawDebugBox(World, VoxelCenter, VoxelBounds.GetExtent(), FColor::Red, false, 5.0f);
				}
			}
		}
	}
}

void FVoxelator::ProcessCollisionBox(const FKBoxElem& BoxElement, FVoxelData& InVoxelData,
	const FTransform& InstanceTransform) const
{
	const FGrid3D& LocalVoxelGrid = InVoxelData.GetVoxelGridConst();
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
				const int32 Index = LocalVoxelGrid.GetVoxelIndex(FIntVector(X, Y, Z));

				if(InVoxelData.GetOccupancy(Index))
				{
					continue;
				}
				
				const FBox VoxelBounds = LocalVoxelGrid.GetVoxelBounds(FIntVector(X, Y, Z));
				const FOOBBoxProxy VoxelBoundsProxy(VoxelBounds, FTransform::Identity, false);
				
				if(VoxelBoundsProxy.IsInsideOrOn(BoxProxy) || VoxelBoundsProxy.Intersect(BoxProxy))
                {
					InVoxelData.SetOccupancy(Index, true);
                }
			}
		}
	}
}

void FVoxelator::ProcessCollisionSphere(const FKSphereElem& SphereElement, FVoxelData& InVoxelData,
	const FTransform& InstanceTransform) const
{
	// AABB test
	const FGrid3D& LocalVoxelGrid = InVoxelData.GetVoxelGridConst();
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
				const int32 Index = LocalVoxelGrid.GetVoxelIndex(FIntVector(X, Y, Z));

				if(InVoxelData.GetOccupancy(Index))
				{
					continue;
				}
				
				const FBox VoxelBounds = LocalVoxelGrid.GetVoxelBounds(FIntVector(X, Y, Z));
				
				if(FMath::SphereAABBIntersection(SphereCenter, FMath::Square(SphereRadius), VoxelBounds))
				{
					InVoxelData.SetOccupancy(Index, true);
				}
			}
		}
	}
}

void FVoxelator::ProcessCollisionCapsule(const FKSphylElem& CapsuleElement, FVoxelData& InVoxelData,
	const FTransform& InstanceTransform) const
{
	// AABB test
	const FGrid3D& LocalVoxelGrid = InVoxelData.GetVoxelGridConst();
	const FIntVector LocalGridSize = LocalVoxelGrid.GetVectorVoxelCount();

	const FCapsuleProxy CapsuleProxy(CapsuleElement, InstanceTransform);

	for(int32 Y = 0; Y < LocalGridSize.Y; Y++)
	{
		for(int32 X = 0; X < LocalGridSize.X; X++)
		{
			for(int32 Z = 0; Z < LocalGridSize.Z; Z++)
			{
				const int32 Index = LocalVoxelGrid.GetVoxelIndex(FIntVector(X, Y, Z));

				if(InVoxelData.GetOccupancy(Index))
				{
					continue;
				}

				const FBox VoxelBounds = LocalVoxelGrid.GetVoxelBounds(FIntVector(X, Y, Z));

				if(CapsuleProxy.Intersects(VoxelBounds))
                {
					InVoxelData.SetOccupancy(Index, true);	
                }
			}
		}
	}
}

void FVoxelator::ProcessCollisionConvex(const FKConvexElem& ConvexElement, FVoxelData& InVoxelData,
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
	
	const FGrid3D& LocalVoxelGrid = InVoxelData.GetVoxelGridConst();
	const FIntVector LocalGridSize = LocalVoxelGrid.GetVectorVoxelCount();
		
	for(int32 Y = 0; Y < LocalGridSize.Y; Y++)
	{
		for(int32 X = 0; X < LocalGridSize.X; X++)
		{
			for(int32 Z = 0; Z < LocalGridSize.Z; Z++)
			{
				const int32 Index = LocalVoxelGrid.GetVoxelIndex(FIntVector(X, Y, Z));

				if(InVoxelData.GetOccupancy(Index))
                {
                    continue;
                }
				
				const FBox VoxelBounds = LocalVoxelGrid.GetVoxelBounds(FIntVector(X, Y, Z));
								
				for(const auto& Triangle : Triangles)
				{
					if(Triangle.Intersects(VoxelBounds))
                    {
                        // bIntersects = true;
						InVoxelData.SetOccupancy(Index, true);
                        break;
                    }
				}
			}
		}
	}
}