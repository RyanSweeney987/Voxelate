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


#include "Data/TriangleProxy.h"


FTriangleProxy::FTriangleProxy()
{
	V[0] = FVector::ZeroVector;
	V[1] = FVector::ZeroVector;
	V[2] = FVector::ZeroVector;
}

FTriangleProxy::FTriangleProxy(const FVector& V0, const FVector& V1, const FVector& V2)
{
	V[0] = V0;
	V[1] = V1;
	V[2] = V2;
}

FTriangleProxy::FTriangleProxy(const FVector& V0, const FVector& V1, const FVector& V2, const ETriangleWinding Winding)
{
	V[0] = V0;
	V[1] = V1;
	V[2] = V2;

	if(GetTriangleWinding() != Winding) {
		Swap(V[1], V[2]);
	}
}

FTriangleProxy::FTriangleProxy(const FVector VIn[3])
{
	Move(VIn, V);
}

FTriangleProxy::FTriangleProxy(const FVector VIn[3], const ETriangleWinding Winding)
{
	Move(VIn, V);

	if(GetTriangleWinding() != Winding) {
		Swap(V[1], V[2]);
	}
}

/**
 * Set the vertices of the triangle
 * @param V0 The first vertex
 * @param V1 The second vertex
 * @param V2 The third vertex
 */
void FTriangleProxy::SetVertices(const FVector& V0, const FVector& V1, const FVector& V2)
{
	V[0] = V0;
	V[1] = V1;
	V[2] = V2;
}

/**
 * Get the point in barycentric coordinates
 * @param Bary0 The first barycentric coordinate
 * @param Bary1 The second barycentric coordinate
 * @param Bary2 The third barycentric coordinate
 * @return The point in barycentric coordinates
 */
FVector FTriangleProxy::BarycentricPoint(const double Bary0, const double Bary1, const double Bary2) const
{
	return BarycentricPoint(FVector(Bary0, Bary1, Bary2));
}

/**
 * Get the point in barycentric coordinates
 * @param BaryCoords The barycentric coordinates
 * @return The point in barycentric coordinates
 */
FVector FTriangleProxy::BarycentricPoint(const FVector& BaryCoords) const
{
	return BaryCoords[0] * V[0] +
		BaryCoords[1] * V[1] +
		BaryCoords[2] * V[2];
}

/**
 * Get the barycentric coordinates of a point
 * @param Point The point
 * @return The barycentric coordinates of the point
 */
FVector FTriangleProxy::GetBarycentricCoords(const FVector& Point) const
{
	const FVector KV02 = V[0] - V[2];
	const FVector KV12 = V[1] - V[2];
	const FVector kPV2 = Point - V[2];
	
	const double FM00 = KV02.Dot(KV02);
	const double FM01 = KV02.Dot(KV12);
	const double FM11 = KV12.Dot(KV12);
	
	const double FR0 = KV02.Dot(kPV2);
	const double FR1 = KV12.Dot(kPV2);
	
	const double FDet = FM00 * FM11 - FM01 * FM01;
	const double FInvDet = 1.0 / FDet;
	
	const double FBary1 = (FM11 * FR0 - FM01 * FR1) * FInvDet;
	const double FBary2 = (FM00 * FR1 - FM01 * FR0) * FInvDet;
	const double FBary3 = 1.0 - FBary1 - FBary2;
	
	return FVector(FBary1, FBary2, FBary3);
}

/**
 * Get the normal of the triangle
 * @return The normal of the triangle
 */
FVector FTriangleProxy::GetNormal() const
{
	FVector Edge1(V[1] - V[0]);
	FVector Edge2(V[2] - V[0]);
	Edge1.Normalize();
	Edge2.Normalize();
	// Unreal has Left-Hand Coordinate System so we need to reverse this cross-product to get proper triangle normal
	const FVector Cross(Edge2.Cross(Edge1));
	//TVector<RealType> vCross(edge1.Cross(edge2));
	return Normalized(Cross);
}

/**
 * Get the centroid of the triangle
 * @return The centroid of the triangle
 */
FVector FTriangleProxy::GetCentroid() const
{
	constexpr double f = 1.0 / 3.0;
	return FVector(
		(V[0].X + V[1].X + V[2].X) * f,
		(V[0].Y + V[1].Y + V[2].Y) * f,
		(V[0].Z + V[1].Z + V[2].Z) * f
	);
}

/**
 * Get the winding of the triangle
 * @return The winding of the triangle
 */
ETriangleWinding FTriangleProxy::GetTriangleWinding() const
{
	const FVector Edge1 = V[1] - V[0];
	const FVector Edge2 = V[2] - V[0];
	
	const FVector Cross = Edge1.Cross(Edge2);
	const double Dot = Cross.Dot(V[0]);
	
	if (Dot > 0) {
		return ETriangleWinding::CCW;
	}
	
	if (Dot < 0) {
		return ETriangleWinding::CW;
	}
	
	return ETriangleWinding::Colinear;
}

/**
 * Get the triangle with the specified winding
 * @param NewWinding The new winding of the triangle
 * @return The triangle with the specified winding
 */
FTriangleProxy FTriangleProxy::GetTriangleWithWinding(const ETriangleWinding NewWinding) const
{
	if(GetTriangleWinding() == NewWinding) {
		return *this;
	}
	return FTriangleProxy(V[0], V[2], V[1]);
}

/**
 * Get the triangles from the vertices and indices
 * @param Vertices The vertices
 * @param Indices The indices
 * @return The triangles
 */
TArray<FTriangleProxy> FTriangleProxy::GetTriangles(const TArray<FVector>& Vertices, const TArray<int32>& Indices)
{
	TArray<FTriangleProxy> Triangles;
	Triangles.Reserve(Indices.Num() / 3);
	
	for(int32 i = 0; i < Indices.Num(); i += 3) {
		Triangles.Add({
			Vertices[Indices[i]],
			Vertices[Indices[i + 1]],
			Vertices[Indices[i + 2]]
		});
	}
	
	return Triangles;
}

/**
 * Get the triangles from the vertices and indices with a specified winding
 * @param Vertices The vertices
 * @param Indices The indices
 * @param Winding The winding of the triangles
 * @return The triangles
 */
TArray<FTriangleProxy> FTriangleProxy::GetTriangles(const TArray<FVector>& Vertices, const TArray<int32>& Indices,
	const ETriangleWinding Winding)
{
	TArray<FTriangleProxy> Triangles;
	Triangles.Reserve(Indices.Num() / 3);
	
	for(int32 i = 0; i < Indices.Num(); i += 3) {
		Triangles.Add({
			Vertices[Indices[i]],
			Vertices[Indices[i + 1]],
			Vertices[Indices[i + 2]],
			Winding
		});
	}
	
	return Triangles;
}

/**
 * Expand the triangle by a delta
 * @param Delta The delta to expand the triangle by
 */
void FTriangleProxy::Expand(const double Delta)
{
	const FVector Centroid = this->GetCentroid();
	V[0] += Delta * Normalized(V[0] - Centroid);
	V[1] += Delta * Normalized(V[1] - Centroid);
	V[2] += Delta * Normalized(V[2] - Centroid);
}

/**
 * Translates this triangle by a translation vector
 * @param Translation The translation vector
 */
void FTriangleProxy::Translate(const FVector& Translation)
{
	V[0] += Translation;
	V[1] += Translation;
	V[2] += Translation;
}

/**
 * Returns a new translated triangle
 * @param Translation The translation vector
 * @return The translated triangle
 */
FTriangleProxy FTriangleProxy::Translate(const FVector& Translation) const
{
	return FTriangleProxy(V[0] + Translation, V[1] + Translation, V[2] + Translation);
}

/**
 * Check if the triangle intersects with an AABB
 * @param Other The AABB to check for intersection
 * @return True if the triangle intersects with the AABB
 */
bool FTriangleProxy::Intersects(const FBox& Other) const
{
	const FTriangleProxy TranslatedTriangle = Translate(-Other.GetCenter());
	const FVector A = TranslatedTriangle.V[0];
	const FVector B = TranslatedTriangle.V[1];
	const FVector C = TranslatedTriangle.V[2];

	FVector AB = B - A;
	FVector BC = C - B;
	FVector CA = A - C;

	AB.Normalize();
	BC.Normalize();
	CA.Normalize();

	const FVector E = Other.GetExtent();

	// Cross AB, BV and CA with (1, 0, 0)
	const FVector A00 = FVector(0, -AB.Z, AB.Y);
	const FVector A01 = FVector(0, -BC.Z, BC.Y);
	const FVector A02 = FVector(0, -CA.Z, CA.Y);

	// Cross AB, BC and CA with (0, 1, 0)
	const FVector A10 = FVector(AB.Z, 0, -AB.X);
	const FVector A11 = FVector(BC.Z, 0, -BC.X);
	const FVector A12 = FVector(CA.Z, 0, -CA.X);

	// Cross AB, BC and CA with (0, 0, 1)
	const FVector A20 = FVector(-AB.Y, AB.X, 0);
	const FVector A21 = FVector(-BC.Y, BC.X, 0);
	const FVector A22 = FVector(-CA.Y, CA.X, 0);

	return IntersectsTriangleAABBSat(TranslatedTriangle, E, A00) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, A01) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, A02) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, A10) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, A11) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, A12) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, A20) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, A21) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, A22) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, FVector(1, 0, 0)) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, FVector(0, 1, 0)) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, FVector(0, 0, 1)) &&
		IntersectsTriangleAABBSat(TranslatedTriangle, E, AB.Cross(BC));
}

bool FTriangleProxy::IntersectsTriangleAABBSat(const FTriangleProxy& Triangle, const FVector& Extent, const FVector& Axis) const
{
	const double P0 = Triangle.V[0].Dot(Axis);
	const double P1 = Triangle.V[1].Dot(Axis);
	const double P2 = Triangle.V[2].Dot(Axis);

	const double R = Extent.X * FMath::Abs(FVector(1, 0, 0).Dot(Axis)) +
			Extent.Y * FMath::Abs(FVector(0, 1, 0).Dot(Axis)) +
			Extent.Z * FMath::Abs(FVector(0, 0, 1).Dot(Axis));

	const double MinP = FMath::Min3(P0, P1, P2);
	const double MaxP = FMath::Max3(P0, P1, P2);

	return !(FMath::Max(-MaxP, MinP) > R);
}

/**
 * Normalize a vector - internal function
 * Used in Geometry for triangle
 * @param Vector The vector to normalize
 * @param Epsilon The epsilon value
 * @return The normalized vector
 */
FVector FTriangleProxy::Normalized(const FVector& Vector, const double Epsilon) const
{
	if (const double Length = Vector.Length(); Length > Epsilon) {
		const double InvLength = 1.0 / Length;
		return FVector(Vector.X * InvLength,
			Vector.Y * InvLength,
			Vector.Z * InvLength);
	}
	
	return FVector(0.0, 0.0, 0.0);
}
