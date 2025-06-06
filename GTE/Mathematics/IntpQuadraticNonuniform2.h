// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

// Quadratic interpolation of a network of triangles whose vertices are of
// the form (x,y,f(x,y)). This code is an implementation of the algorithm
// found in
//
//   Zoltan J. Cendes and Steven H. Wong,
//   C1 quadratic interpolation over arbitrary point sets,
//   IEEE Computer Graphics & Applications,
//   pp. 8-16, 1987
//
// The TriangleMesh interface must support the following:
//   int32_t GetNumVertices() const;
//   int32_t GetNumTriangles() const;
//   Vector2<T> const* GetVertices() const;
//   int32_t const* GetIndices() const;
//   bool GetVertices(int32_t, std::array<Vector2<T>, 3>&) const;
//   bool GetIndices(int32_t, std::array<int32_t, 3>&) const;
//   bool GetAdjacencies(int32_t, std::array<int32_t, 3>&) const;
//   bool GetBarycentrics(int32_t, Vector2<T> const&, std::array<T, 3>&) const;
//   int32_t GetContainingTriangle(Vector2<T> const&) const;

#include <Mathematics/Delaunay2.h>
#include <Mathematics/ContScribeCircle2.h>
#include <Mathematics/DistPointAlignedBox.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <typename T, typename TriangleMesh>
    class IntpQuadraticNonuniform2
    {
    public:
        // The first constructor requires only F and a measure of the rate of
        // change of the function values relative to changes in the spatial
        // variables. The df/dx and df/dy values are estimated at the sample
        // points using mesh normals and spatialDelta.
        //
        // The second constructor requires you to specify function values F
        // and first-order partial derivative values df/dx and df/dy.

        IntpQuadraticNonuniform2(TriangleMesh const& mesh, T const* F, T spatialDelta)
            :
            mMesh(&mesh),
            mF(F),
            mFX(nullptr),
            mFY(nullptr),
            mFXStorage{},
            mFYStorage{},
            mTData{}
        {
            EstimateDerivatives(spatialDelta);
            ProcessTriangles();
        }

        IntpQuadraticNonuniform2(TriangleMesh const& mesh, T const* F, T const* FX, T const* FY)
            :
            mMesh(&mesh),
            mF(F),
            mFX(FX),
            mFY(FY),
            mFXStorage{},
            mFYStorage{},
            mTData{}
        {
            ProcessTriangles();
        }

        // Quadratic interpolation.  The return value is 'true' if and only if
        // the input point is in the convex hull of the input vertices, in
        // which case the interpolation is valid.
        bool operator()(Vector2<T> const& P, T& F, T& FX, T& FY) const
        {
            std::int32_t t = static_cast<std::int32_t>(mMesh->GetContainingTriangle(P));
            if (t == static_cast<std::int32_t>(mMesh->GetInvalidIndex()))
            {
                // The point is outside the triangulation.
                return false;
            }

            // Get the vertices of the triangle.
            std::array<Vector2<T>, 3> V{};
            mMesh->GetVertices(t, V);

            // Get the additional information for the triangle.
            TriangleData const& tData = mTData[t];

            // Determine which of the six subtriangles contains the target
            // point.  Theoretically, P must be in one of these subtriangles.
            Vector2<T> sub0 = tData.center;
            Vector2<T> sub1{};
            Vector2<T> sub2 = tData.intersect[2];
            Vector3<T> bary{};
            int32_t index{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            AlignedBox3<T> barybox({ zero, zero, zero }, { one, one, one });
            DCPQuery<T, Vector3<T>, AlignedBox3<T>> pbQuery;
            int32_t minIndex = 0;
            T minDistance = -one;
            Vector3<T> minBary{ zero, zero, zero };
            Vector2<T> minSub0{ zero, zero };
            Vector2<T> minSub1{ zero, zero };
            Vector2<T> minSub2{ zero, zero };

            for (index = 1; index <= 6; ++index)
            {
                sub1 = sub2;
                if ((index % 2) != 0) // index is odd
                {
                    size_t lookup = static_cast<size_t>(index) / 2;
                    sub2 = V[lookup];
                }
                else // index is even
                {
                    size_t lookup = (static_cast<size_t>(index) - 1) / 2;
                    sub2 = tData.intersect[lookup];
                }

                std::array<T, 3> localBary{};
                bool valid = ComputeBarycentrics(P, sub0, sub1, sub2, localBary);
                for (int32_t k = 0; k < 3; ++k)
                {
                    bary[k] = localBary[k];
                }
                if (valid
                    && zero <= bary[0] && bary[0] <= one
                    && zero <= bary[1] && bary[1] <= one
                    && zero <= bary[2] && bary[2] <= one)
                {
                    // P is in triangle <Sub0,Sub1,Sub2>
                    break;
                }

                // When computing with floating-point arithmetic, rounding
                // errors can cause us to reach this code when, theoretically,
                // the point is in the subtriangle.  Keep track of the
                // (b0,b1,b2) that is closest to the barycentric cube [0,1]^3
                // and choose the triangle corresponding to it when all 6
                // tests previously fail.
                T distance = pbQuery(bary, barybox).distance;
                if (minIndex == 0 || distance < minDistance)
                {
                    minDistance = distance;
                    minIndex = index;
                    minBary = bary;
                    minSub0 = sub0;
                    minSub1 = sub1;
                    minSub2 = sub2;
                }
            }

            // If the subtriangle was not found, rounding errors caused
            // problems.  Choose the barycentric point closest to the box.
            if (index > 6)
            {
                index = minIndex;
                bary = minBary;
                sub0 = minSub0;
                sub1 = minSub1;
                sub2 = minSub2;
            }

            // Fetch Bezier control points.
            std::array<T, 6> bez =
            {
                tData.coeff[0],
                tData.coeff[12 + static_cast<size_t>(index)],
                tData.coeff[13 + (static_cast<size_t>(index) % 6)],
                tData.coeff[static_cast<size_t>(index)],
                tData.coeff[6 + static_cast<size_t>(index)],
                tData.coeff[1 + (static_cast<size_t>(index) % 6)]
            };

            // Evaluate Bezier quadratic.
            F = bary[0] * (bez[0] * bary[0] + bez[1] * bary[1] + bez[2] * bary[2]) +
                bary[1] * (bez[1] * bary[0] + bez[3] * bary[1] + bez[4] * bary[2]) +
                bary[2] * (bez[2] * bary[0] + bez[4] * bary[1] + bez[5] * bary[2]);

            // Evaluate barycentric derivatives of F.
            T const two = static_cast<T>(2);
            T FU = two * (bez[0] * bary[0] + bez[1] * bary[1] + bez[2] * bary[2]);
            T FV = two * (bez[1] * bary[0] + bez[3] * bary[1] + bez[4] * bary[2]);
            T FW = two * (bez[2] * bary[0] + bez[4] * bary[1] + bez[5] * bary[2]);
            T duw = FU - FW;
            T dvw = FV - FW;

            // Convert back to (x,y) coordinates.
            T m00 = sub0[0] - sub2[0];
            T m10 = sub0[1] - sub2[1];
            T m01 = sub1[0] - sub2[0];
            T m11 = sub1[1] - sub2[1];
            T inv = static_cast<T>(1) / (m00 * m11 - m10 * m01);

            FX = inv * (m11 * duw - m10 * dvw);
            FY = inv * (m00 * dvw - m01 * duw);
            return true;
        }

    private:
        void EstimateDerivatives(T spatialDelta)
        {
            T const zero = static_cast<T>(0);

            std::int32_t numVertices = static_cast<std::int32_t>(mMesh->GetNumVertices());
            Vector2<T> const* vertices = mMesh->GetVertices();
            std::int32_t numTriangles = static_cast<std::int32_t>(mMesh->GetNumTriangles());
            int32_t const* indices = mMesh->GetIndices();

            mFXStorage.resize(numVertices);
            mFYStorage.resize(numVertices);
            std::vector<T> FZ(numVertices);
            std::fill(mFXStorage.begin(), mFXStorage.end(), zero);
            std::fill(mFYStorage.begin(), mFYStorage.end(), zero);
            std::fill(FZ.begin(), FZ.end(), zero);

            mFX = mFXStorage.data();
            mFY = mFYStorage.data();

            // Accumulate normals at spatial locations (averaging process).
            for (std::int32_t t = 0; t < numTriangles; ++t)
            {
                // Get three vertices of triangle.
                int32_t v0 = *indices++;
                int32_t v1 = *indices++;
                int32_t v2 = *indices++;

                // Compute normal vector of triangle (with positive
                // z-component).
                T dx1 = vertices[v1][0] - vertices[v0][0];
                T dy1 = vertices[v1][1] - vertices[v0][1];
                T dz1 = mF[v1] - mF[v0];
                T dx2 = vertices[v2][0] - vertices[v0][0];
                T dy2 = vertices[v2][1] - vertices[v0][1];
                T dz2 = mF[v2] - mF[v0];
                T nx = dy1 * dz2 - dy2 * dz1;
                T ny = dz1 * dx2 - dz2 * dx1;
                T nz = dx1 * dy2 - dx2 * dy1;
                if (nz < zero)
                {
                    nx = -nx;
                    ny = -ny;
                    nz = -nz;
                }

                mFXStorage[v0] += nx;  mFYStorage[v0] += ny;  FZ[v0] += nz;
                mFXStorage[v1] += nx;  mFYStorage[v1] += ny;  FZ[v1] += nz;
                mFXStorage[v2] += nx;  mFYStorage[v2] += ny;  FZ[v2] += nz;
            }

            // Scale the normals to form (x,y,-1).
            for (std::int32_t i = 0; i < numVertices; ++i)
            {
                if (FZ[i] != zero)
                {
                    T inv = -spatialDelta / FZ[i];
                    mFXStorage[i] *= inv;
                    mFYStorage[i] *= inv;
                }
                else
                {
                    mFXStorage[i] = (T)0;
                    mFYStorage[i] = (T)0;
                }
            }
        }

        void ProcessTriangles()
        {
            // Add degenerate triangles to boundary triangles so that
            // interpolation at the boundary can be treated in the same way
            // as interpolation in the interior.

            // Compute centers of inscribed circles for triangles.
            Vector2<T> const* vertices = mMesh->GetVertices();
            std::int32_t numTriangles = static_cast<std::int32_t>(mMesh->GetNumTriangles());
            int32_t const* indices = mMesh->GetIndices();
            mTData.resize(numTriangles);
            for (std::int32_t t = 0; t < numTriangles; ++t)
            {
                int32_t v0 = *indices++;
                int32_t v1 = *indices++;
                int32_t v2 = *indices++;
                Circle2<T> circle{};
                Inscribe(vertices[v0], vertices[v1], vertices[v2], circle);
                mTData[t].center = circle.center;
            }

            // Compute cross-edge intersections.
            for (std::int32_t t = 0; t < numTriangles; ++t)
            {
                ComputeCrossEdgeIntersections(t);
            }

            // Compute Bezier coefficients.
            for (std::int32_t t = 0; t < numTriangles; ++t)
            {
                ComputeCoefficients(t);
            }
        }

        void ComputeCrossEdgeIntersections(int32_t t)
        {
            // Get the vertices of the triangle.
            std::array<Vector2<T>, 3> V{};
            mMesh->GetVertices(t, V);

            // Get the centers of adjacent triangles.
            T const one = static_cast<T>(1);
            T const half = static_cast<T>(0.5);
            TriangleData& tData = mTData[t];
            std::array<int32_t, 3> adjacencies = { 0, 0, 0 };
            mMesh->GetAdjacencies(t, adjacencies);
            for (int32_t j0 = 2, j1 = 0; j1 < 3; j0 = j1++)
            {
                int32_t a = adjacencies[j0];
                if (a >= 0)
                {
                    // Get center of adjacent triangle's inscribing circle.
                    Vector2<T> U = mTData[a].center;
                    T m00 = V[j0][1] - V[j1][1];
                    T m01 = V[j1][0] - V[j0][0];
                    T m10 = tData.center[1] - U[1];
                    T m11 = U[0] - tData.center[0];
                    T r0 = m00 * V[j0][0] + m01 * V[j0][1];
                    T r1 = m10 * tData.center[0] + m11 * tData.center[1];
                    T invDet = one / (m00 * m11 - m01 * m10);
                    tData.intersect[j0][0] = (m11 * r0 - m01 * r1) * invDet;
                    tData.intersect[j0][1] = (m00 * r1 - m10 * r0) * invDet;
                }
                else
                {
                    // No adjacent triangle, use center of edge.
                    tData.intersect[j0] = half * (V[j0] + V[j1]);
                }
            }
        }

        void ComputeCoefficients(int32_t t)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const half = static_cast<T>(0.5);

            // Get the vertices of the triangle.
            std::array<Vector2<T>, 3> V{};
            mMesh->GetVertices(t, V);

            // Get the additional information for the triangle.
            TriangleData& tData = mTData[t];

            // Get the sample data at main triangle vertices.
            std::array<int32_t, 3> indices = { 0, 0, 0 };
            mMesh->GetIndices(t, indices);
            std::array<Jet, 3> jet{};
            for (std::size_t j = 0; j < 3; ++j)
            {
                int32_t k = indices[j];
                jet[j].F = mF[k];
                jet[j].FX = mFX[k];
                jet[j].FY = mFY[k];
            }

            // Get centers of adjacent triangles.
            std::array<int32_t, 3> adjacencies = { 0, 0, 0 };
            mMesh->GetAdjacencies(t, adjacencies);
            std::array<Vector2<T>, 3> U{};
            for (std::size_t j0 = 2, j1 = 0; j1 < 3; j0 = j1++)
            {
                int32_t a = adjacencies[j0];
                if (a >= 0)
                {
                    // Get center of adjacent triangle's circumscribing 
                    // circle.
                    U[j0] = mTData[a].center;
                }
                else
                {
                    // No adjacent triangle, use center of edge.
                    U[j0] = half * (V[j0] + V[j1]);
                }
            }

            // Compute intermediate terms.
            std::array<T, 3> cenT{}, cen0{}, cen1{}, cen2{};
            mMesh->GetBarycentrics(t, tData.center, cenT);
            mMesh->GetBarycentrics(t, U[0], cen0);
            mMesh->GetBarycentrics(t, U[1], cen1);
            mMesh->GetBarycentrics(t, U[2], cen2);

            T alpha = (cenT[1] * cen1[0] - cenT[0] * cen1[1]) / (cen1[0] - cenT[0]);
            T beta = (cenT[2] * cen2[1] - cenT[1] * cen2[2]) / (cen2[1] - cenT[1]);
            T gamma = (cenT[0] * cen0[2] - cenT[2] * cen0[0]) / (cen0[2] - cenT[2]);
            T oneMinusAlpha = one - alpha;
            T oneMinusBeta = one - beta;
            T oneMinusGamma = one - gamma;

            std::array<T, 9> A{};
            std::array<T, 9> B{};
            A.fill(zero);
            B.fill(zero);

            T tmp = cenT[0] * V[0][0] + cenT[1] * V[1][0] + cenT[2] * V[2][0];
            A[0] = half * (tmp - V[0][0]);
            A[1] = half * (tmp - V[1][0]);
            A[2] = half * (tmp - V[2][0]);
            A[3] = half * beta * (V[2][0] - V[0][0]);
            A[4] = half * oneMinusGamma * (V[1][0] - V[0][0]);
            A[5] = half * gamma * (V[0][0] - V[1][0]);
            A[6] = half * oneMinusAlpha * (V[2][0] - V[1][0]);
            A[7] = half * alpha * (V[1][0] - V[2][0]);
            A[8] = half * oneMinusBeta * (V[0][0] - V[2][0]);

            tmp = cenT[0] * V[0][1] + cenT[1] * V[1][1] + cenT[2] * V[2][1];
            B[0] = half * (tmp - V[0][1]);
            B[1] = half * (tmp - V[1][1]);
            B[2] = half * (tmp - V[2][1]);
            B[3] = half * beta * (V[2][1] - V[0][1]);
            B[4] = half * oneMinusGamma * (V[1][1] - V[0][1]);
            B[5] = half * gamma * (V[0][1] - V[1][1]);
            B[6] = half * oneMinusAlpha * (V[2][1] - V[1][1]);
            B[7] = half * alpha * (V[1][1] - V[2][1]);
            B[8] = half * oneMinusBeta * (V[0][1] - V[2][1]);

            // Compute Bezier coefficients.
            tData.coeff[2] = jet[0].F;
            tData.coeff[4] = jet[1].F;
            tData.coeff[6] = jet[2].F;

            tData.coeff[14] = jet[0].F + A[0] * jet[0].FX + B[0] * jet[0].FY;
            tData.coeff[7] = jet[0].F + A[3] * jet[0].FX + B[3] * jet[0].FY;
            tData.coeff[8] = jet[0].F + A[4] * jet[0].FX + B[4] * jet[0].FY;
            tData.coeff[16] = jet[1].F + A[1] * jet[1].FX + B[1] * jet[1].FY;
            tData.coeff[9] = jet[1].F + A[5] * jet[1].FX + B[5] * jet[1].FY;
            tData.coeff[10] = jet[1].F + A[6] * jet[1].FX + B[6] * jet[1].FY;
            tData.coeff[18] = jet[2].F + A[2] * jet[2].FX + B[2] * jet[2].FY;
            tData.coeff[11] = jet[2].F + A[7] * jet[2].FX + B[7] * jet[2].FY;
            tData.coeff[12] = jet[2].F + A[8] * jet[2].FX + B[8] * jet[2].FY;

            tData.coeff[5] = alpha * tData.coeff[10] + oneMinusAlpha * tData.coeff[11];
            tData.coeff[17] = alpha * tData.coeff[16] + oneMinusAlpha * tData.coeff[18];
            tData.coeff[1] = beta * tData.coeff[12] + oneMinusBeta * tData.coeff[7];
            tData.coeff[13] = beta * tData.coeff[18] + oneMinusBeta * tData.coeff[14];
            tData.coeff[3] = gamma * tData.coeff[8] + oneMinusGamma * tData.coeff[9];
            tData.coeff[15] = gamma * tData.coeff[14] + oneMinusGamma * tData.coeff[16];
            tData.coeff[0] = cenT[0] * tData.coeff[14] + cenT[1] * tData.coeff[16] + cenT[2] * tData.coeff[18];
        }

        class TriangleData
        {
        public:
            TriangleData()
                :
                center{},
                intersect{},
                coeff{}
            {
                T const zero = static_cast<T>(0);
                Vector2<T> const vzero{ zero, zero };
                center = vzero;
                intersect.fill(vzero);
                coeff.fill(zero);
            }

            Vector2<T> center;
            std::array<Vector2<T>, 3> intersect;
            std::array<T, 19> coeff;
        };

        class Jet
        {
        public:
            Jet()
                :
                F(static_cast<T>(0)),
                FX(static_cast<T>(0)),
                FY(static_cast<T>(0))
            {
            }

            T F, FX, FY;
        };

        TriangleMesh const* mMesh;
        T const* mF;
        T const* mFX;
        T const* mFY;
        std::vector<T> mFXStorage;
        std::vector<T> mFYStorage;
        std::vector<TriangleData> mTData;
    };
}

