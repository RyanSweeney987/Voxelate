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
#include "TriangleProxy.generated.h"

/**
 * Winding of a triangle
 */
UENUM()
enum class ETriangleWinding : uint8
{
	CW			= 0,
	CCW			= 1,
	Colinear	= 2
};

/**
 * Proxy for a triangle - makes it easier to voxelise triangles
 * 
 */
USTRUCT()
struct VOXELATE_API FTriangleProxy
{
	GENERATED_BODY()

	FVector V[3];

	FTriangleProxy();
	FTriangleProxy(const FVector& V0, const FVector& V1, const FVector& V2);
	FTriangleProxy(const FVector& V0, const FVector& V1, const FVector& V2, const ETriangleWinding Winding);
	FTriangleProxy(const FVector VIn[3]);
	FTriangleProxy(const FVector VIn[3], const ETriangleWinding Winding);

	void SetVertices(const FVector& V0, const FVector& V1, const FVector& V2);

	FVector BarycentricPoint(const double Bary0, const double Bary1, const double Bary2) const;
	FVector BarycentricPoint(const FVector& BaryCoords) const;
	FVector GetBarycentricCoords(const FVector& Point) const;

	FVector GetNormal() const;
	FVector GetCentroid() const;
	
	ETriangleWinding GetTriangleWinding() const;
	FTriangleProxy GetTriangleWithWinding(const ETriangleWinding NewWinding) const;
	
	static TArray<FTriangleProxy> GetTriangles(const TArray<FVector>& Vertices, const TArray<int32>& Indices);
	static TArray<FTriangleProxy> GetTriangles(const TArray<FVector>& Vertices, const TArray<int32>& Indices, const ETriangleWinding Winding);

	void Expand(const double Delta);
	
	void Translate(const FVector& Translation);
	FTriangleProxy Translate(const FVector& Translation) const;
private:
	FVector Normalized(const FVector& Vector, const double Epsilon = 0.0) const;
};