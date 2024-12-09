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

#include "Data/OOBBoxProxy.h"


FOOBBoxProxy::FOOBBoxProxy(const FBoxSphereBounds& LocalBounds, const FTransform& InstanceTransform,
                                      const bool& InSlerpRotation) : FOOBBoxProxy(LocalBounds.GetBox(), InstanceTransform, InSlerpRotation)
{
}

FOOBBoxProxy::FOOBBoxProxy(const FBox& LocalBounds, const FTransform& InstanceTransform,
	const bool& InSlerpRotation)
{
	// Calculate the OBB center in world space
	Center = InstanceTransform.TransformPosition(LocalBounds.GetCenter());

	// Extract rotation and absolute scale from the instance transform
	Orientation = InstanceTransform.GetRotation();
	const FVector Scale3D = InstanceTransform.GetScale3D().GetAbs();

	// Scale the extents by the absolute instance scale to handle negative scaling
	Extents = LocalBounds.GetExtent() * Scale3D;

	bSlerpRotation = InSlerpRotation;
}

/**
 * Get the OBB's axis in world space
 * @param OutAxisX The X-axis of the OBB
 * @param OutAxisY The Y-axis of the OBB
 * @param OutAxisZ The Z-axis of the OBB
 */
void FOOBBoxProxy::GetAxis(FVector& OutAxisX, FVector& OutAxisY, FVector& OutAxisZ) const
{
	OutAxisX = Orientation.GetAxisX();
	OutAxisY = Orientation.GetAxisY();
	OutAxisZ = Orientation.GetAxisZ();
}

/**
 * Get the eight corners of the OBB
 * @param OutCorners The array to store the eight corners
 */
void FOOBBoxProxy::GetCorners(TArray<FVector>& OutCorners) const
{
	FVector AxisX, AxisY, AxisZ;
	GetAxis(AxisX, AxisY, AxisZ);

	// Half-size vectors along each axis
	const FVector HalfSizeX = AxisX * Extents.X;
	const FVector HalfSizeY = AxisY * Extents.Y;
	const FVector HalfSizeZ = AxisZ * Extents.Z;

	// Initialize the corners array
	OutCorners.SetNumUninitialized(8);

	// Calculate the eight corners of the OBB
	OutCorners[0] = Center + HalfSizeX + HalfSizeY + HalfSizeZ;
	OutCorners[1] = Center + HalfSizeX + HalfSizeY - HalfSizeZ;
	OutCorners[2] = Center + HalfSizeX - HalfSizeY + HalfSizeZ;
	OutCorners[3] = Center + HalfSizeX - HalfSizeY - HalfSizeZ;
	OutCorners[4] = Center - HalfSizeX + HalfSizeY + HalfSizeZ;
	OutCorners[5] = Center - HalfSizeX + HalfSizeY - HalfSizeZ;
	OutCorners[6] = Center - HalfSizeX - HalfSizeY + HalfSizeZ;
	OutCorners[7] = Center - HalfSizeX - HalfSizeY - HalfSizeZ;
}

/**
 * Combine two OBBs by updating the extents and center
 * @param Other The other OBB to combine with
 * @return The combined OBB
 */
FOOBBoxProxy& FOOBBoxProxy::operator+=(const FOOBBoxProxy& Other)
{
	// Step 1: Transform the corners of the other OBB into this OBB's local space
	TArray<FVector> OtherCorners;
	Other.GetCorners(OtherCorners);

	// Inverse rotation to transform into local space
	const FQuat InverseOrientation = Orientation.Inverse();

	// Store min and max extents in local space
	FVector MinExtents = -Extents;
	FVector MaxExtents = Extents;

	for (const FVector& Corner : OtherCorners)
	{
		// Vector from this OBB's center to the corner
		FVector LocalVec = InverseOrientation.RotateVector(Corner - Center);

		// Update min and max extents
		MinExtents = MinExtents.ComponentMin(LocalVec);
		MaxExtents = MaxExtents.ComponentMax(LocalVec);
	}

	// Step 2: Update extents and center
	// New extents are half the size of the new bounds
	const FVector NewExtents = (MaxExtents - MinExtents) * 0.5f;

	// The center offset in local space
	const FVector LocalCenterOffset = (MaxExtents + MinExtents) * 0.5f;

	// Update the center in world space
	Center = Center + Orientation.RotateVector(LocalCenterOffset);

	// Update extents
	Extents = NewExtents;

	// Keep the current rotation or handle rotation interpolation if needed
	if (bSlerpRotation)
	{
		const FQuat CombinedQuat = FQuat::Slerp(this->Orientation, Other.Orientation, 0.5f);
		Orientation = CombinedQuat;
	}

	return *this;
}

/**
 * Check if a point is inside or on the OBB
 * @param Point The point to check
 * @return True if the point is inside or on the OBB
 */
bool FOOBBoxProxy::IsInsideOrOn(const FVector& Point) const
{
	// Transform the point to the OBB's local space
	const FVector LocalPoint = Orientation.UnrotateVector(Point - Center);

	// Check if the point lies within the extents
	return FMath::Abs(LocalPoint.X) <= Extents.X &&
		FMath::Abs(LocalPoint.Y) <= Extents.Y &&
		FMath::Abs(LocalPoint.Z) <= Extents.Z;
}

/**
 * Check if this OBB is entirely inside or on another OBB
 * @param Other The other OBB to check against
 * @return True if this OBB is entirely inside or on the other OBB
 */
bool FOOBBoxProxy::IsInsideOrOn(const FOOBBoxProxy& Other) const
{
	// Step 1: Get the corners of this OBB
	TArray<FVector> Corners;
	Other.GetCorners(Corners);

	// Step 2: Check if each corner is inside or on the other OBB
	for (const FVector& Corner : Corners)
	{
		if (this->IsInsideOrOn(Corner))
		{
			// At least one corner is outside the other OBB
			return true;
		}
	}

	// All corners are inside or on the other OBB
	return false;
}

/**
 * Check if this OBB is entirely inside or on an AABB
 * @param Other The AABB to check against
 * @return True if this OBB is entirely inside or on the AABB
 */
bool FOOBBoxProxy::IsInsideOrOn(const FBox& Other) const
{
	return IsInsideOrOn(FOOBBoxProxy(Other, FTransform::Identity, true));
}

/**
 * Check if this OBB intersects with another OBB
 * @param Other The other OBB to check against
 * @return True if this OBB intersects with the other OBB
 */
bool FOOBBoxProxy::Intersect(const FOOBBoxProxy& Other) const
{
	// Use the Separating Axis Theorem (SAT)
	// Step 1: Get the axes of both OBBs
	FVector AxesA[3];
	this->GetAxis(AxesA[0], AxesA[1], AxesA[2]);

	FVector AxesB[3];
	Other.GetAxis(AxesB[0], AxesB[1], AxesB[2]);

	// Step 2: Compute the rotation matrix expressing Other in A's coordinate frame
	float R[3][3];
	float AbsR[3][3];

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			R[i][j] = FVector::DotProduct(AxesA[i], AxesB[j]);
			AbsR[i][j] = FMath::Abs(R[i][j]) + KINDA_SMALL_NUMBER; // Add epsilon to avoid division by zero
		}
	}

	// Step 3: Compute the translation vector
	const FVector T = Other.Center - this->Center;
	// Bring translation into A's coordinate frame
	const FVector TA(FVector::DotProduct(T, AxesA[0]),
		FVector::DotProduct(T, AxesA[1]),
		FVector::DotProduct(T, AxesA[2]));

	// Step 4: Test axes
	float RA, RB;

	// Test axes L = A0, A1, A2
	for (int i = 0; i < 3; ++i)
	{
		RA = this->Extents[i];
		RB = Other.Extents.X * AbsR[i][0] + Other.Extents.Y * AbsR[i][1] + Other.Extents.Z * AbsR[i][2];
		if (FMath::Abs(TA[i]) > RA + RB)
			return false;
	}

	// Test axes L = B0, B1, B2
	for (int i = 0; i < 3; ++i)
	{
		RA = this->Extents.X * AbsR[0][i] + this->Extents.Y * AbsR[1][i] + this->Extents.Z * AbsR[2][i];
		RB = Other.Extents[i];
		if (FMath::Abs(TA[0] * R[0][i] + TA[1] * R[1][i] + TA[2] * R[2][i]) > RA + RB)
			return false;
	}

	// Test axis L = A0 x B0
	RA = this->Extents.Y * AbsR[2][0] + this->Extents.Z * AbsR[1][0];
	RB = Other.Extents.Y * AbsR[0][2] + Other.Extents.Z * AbsR[0][1];
	if (FMath::Abs(TA[2] * R[1][0] - TA[1] * R[2][0]) > RA + RB)
		return false;

	// Continue testing cross products of axes...
	// There are 9 cross-product axes to test in total

	// For brevity, we'll implement the rest in a loop

	static const int TestCases[9][4] = {
		{0, 1, 2, 0}, // L = A0 x B0
		{0, 1, 2, 1}, // L = A0 x B1
		{0, 1, 2, 2}, // L = A0 x B2
		{1, 2, 0, 0}, // L = A1 x B0
		{1, 2, 0, 1}, // L = A1 x B1
		{1, 2, 0, 2}, // L = A1 x B2
		{2, 0, 1, 0}, // L = A2 x B0
		{2, 0, 1, 1}, // L = A2 x B1
		{2, 0, 1, 2}  // L = A2 x B2
	};

	for (int k = 0; k < 9; ++k)
	{
		const int I = TestCases[k][0];
		const int J = TestCases[k][1];
		const int M = TestCases[k][2];
		const int N = TestCases[k][3];

		RA = this->Extents[M] * AbsR[(I + 1) % 3][N] + this->Extents[(I + 2) % 3] * AbsR[(I + 2) % 3][N];
		RB = Other.Extents[(N + 1) % 3] * AbsR[I][(N + 2) % 3] + Other.Extents[(N + 2) % 3] * AbsR[I][(N + 1) % 3];
		
		if(FMath::Abs(TA[(I + 2) % 3] * R[(I + 1) % 3][N] - TA[(I + 1) % 3] * R[(I + 2) % 3][N]) > (RA + RB))
		{
			return false;
		}
	}

	// No separating axis found, the OBBs intersect
	return true;
}

/**
 * Check if this OBB intersects with an AABB
 * @param Other The AABB to check against
 * @return True if this OBB intersects with the AABB
 */
bool FOOBBoxProxy::Intersect(const FBox& Other) const
{
	return Intersect(FOOBBoxProxy(Other, FTransform::Identity, true));
}

/**
 * Convert the OBB to an FTransform
 * @return The OBB as an FTransform
 */
FTransform FOOBBoxProxy::ToTransform() const
{
	// The position is the center of the OBB
	const FVector Position = Center;

	// The rotation is the orientation of the OBB
	const FQuat Rotation = Orientation;

	// The scale is twice the extents (since extents are half-sizes)
	const FVector Scale = Extents * 2.0f;

	// Construct and return the FTransform
	return FTransform(Rotation, Position, Scale);
}