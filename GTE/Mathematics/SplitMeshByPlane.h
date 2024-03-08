// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The algorithm for splitting a mesh by a plane is described in
// https://www.geometrictools.com/Documentation/ClipMesh.pdf
// Currently, the code here does not include generating a closed
// mesh (from the "positive" and "zero" vertices) by attaching
// triangulated faces to the mesh, where the those faces live in
// the splitting plane.  (TODO: Add this code.)

#include <Mathematics/DistPointHyperplane.h>
#include <Mathematics/EdgeKey.h>
#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

namespace gte
{
    template <typename Real>
    class SplitMeshByPlane
    {
    public:
        // The 'indices' are lookups into the 'vertices' array.  The indices
        // represent a triangle mesh.  The number of indices must be a
        // multiple of 3, each triple representing a triangle.  If t is a
        // triangle index, then the triangle is formed by
        // vertices[indices[3 * t + i]] for 0 <= i <= 2.  The outputs
        // 'negIndices' and 'posIndices' are formatted similarly.
        void operator()(
            std::vector<Vector3<Real>> const& vertices,
            std::vector<int32_t> const& indices,
            Plane3<Real> const& plane,
            std::vector<Vector3<Real>>& clipVertices,
            std::vector<int32_t>& negIndices,
            std::vector<int32_t>& posIndices)
        {
            mSignedDistances.resize(vertices.size());
            mEMap.clear();

            // Make a copy of the incoming vertices.  If the mesh intersects
            // the plane, new vertices must be generated.  These are appended
            // to the clipVertices array.
            clipVertices = vertices;

            ClassifyVertices(clipVertices, plane);
            ClassifyEdges(clipVertices, indices);
            ClassifyTriangles(indices, negIndices, posIndices);
        }

    private:
        void ClassifyVertices(std::vector<Vector3<Real>> const& clipVertices,
            Plane3<Real> const& plane)
        {
            DCPQuery<Real, Vector3<Real>, Plane3<Real>> query;
            for (size_t i = 0; i < clipVertices.size(); ++i)
            {
                mSignedDistances[i] = query(clipVertices[i], plane).signedDistance;
            }
        }

        void ClassifyEdges(std::vector<Vector3<Real>>& clipVertices,
            std::vector<int32_t> const& indices)
        {
            size_t const numTriangles = indices.size() / 3;
            int32_t nextIndex = static_cast<int32_t>(clipVertices.size());
            for (size_t i = 0; i < numTriangles; ++i)
            {
                size_t threeI = 3 * static_cast<size_t>(i);
                int32_t v0 = indices[threeI + 0];
                int32_t v1 = indices[threeI + 1];
                int32_t v2 = indices[threeI + 2];
                Real sDist0 = mSignedDistances[v0];
                Real sDist1 = mSignedDistances[v1];
                Real sDist2 = mSignedDistances[v2];

                EdgeKey<false> key;
                Real t;
                Vector3<Real> intr, diff;

                // The change-in-sign tests are structured this way to avoid
                // numerical round-off problems.  For example, sDist0 > 0 and
                // sDist1 < 0, but both are very small and sDist0 * sDist1 = 0
                // because of round-off errors.  The tests also guarantee
                // consistency between this function and ClassifyTriangles,
                // the latter function using sign tests only on the individual
                // sDist values.

                if ((sDist0 > (Real)0 && sDist1 < (Real)0)
                    || (sDist0 < (Real)0 && sDist1 >(Real)0))
                {
                    key = EdgeKey<false>(v0, v1);
                    if (mEMap.find(key) == mEMap.end())
                    {
                        t = sDist0 / (sDist0 - sDist1);
                        diff = clipVertices[v1] - clipVertices[v0];
                        intr = clipVertices[v0] + t * diff;
                        clipVertices.push_back(intr);
                        mEMap[key] = std::make_pair(intr, nextIndex);
                        ++nextIndex;
                    }
                }

                if ((sDist1 > (Real)0 && sDist2 < (Real)0)
                    || (sDist1 < (Real)0 && sDist2 >(Real)0))
                {
                    key = EdgeKey<false>(v1, v2);
                    if (mEMap.find(key) == mEMap.end())
                    {
                        t = sDist1 / (sDist1 - sDist2);
                        diff = clipVertices[v2] - clipVertices[v1];
                        intr = clipVertices[v1] + t * diff;
                        clipVertices.push_back(intr);
                        mEMap[key] = std::make_pair(intr, nextIndex);
                        ++nextIndex;
                    }
                }

                if ((sDist2 > (Real)0 && sDist0 < (Real)0)
                    || (sDist2 < (Real)0 && sDist0 >(Real)0))
                {
                    key = EdgeKey<false>(v2, v0);
                    if (mEMap.find(key) == mEMap.end())
                    {
                        t = sDist2 / (sDist2 - sDist0);
                        diff = clipVertices[v0] - clipVertices[v2];
                        intr = clipVertices[v2] + t * diff;
                        clipVertices.push_back(intr);
                        mEMap[key] = std::make_pair(intr, nextIndex);
                        ++nextIndex;
                    }
                }
            }
        }

        void ClassifyTriangles(std::vector<int32_t> const& indices,
            std::vector<int32_t>& negIndices, std::vector<int32_t>& posIndices)
        {
            size_t const numTriangles = static_cast<int32_t>(indices.size() / 3);
            for (size_t i = 0; i < numTriangles; ++i)
            {
                size_t threeI = 3 * static_cast<size_t>(i);
                int32_t v0 = indices[threeI + 0];
                int32_t v1 = indices[threeI + 1];
                int32_t v2 = indices[threeI + 2];
                Real sDist0 = mSignedDistances[v0];
                Real sDist1 = mSignedDistances[v1];
                Real sDist2 = mSignedDistances[v2];

                if (sDist0 > (Real)0)
                {
                    if (sDist1 > (Real)0)
                    {
                        if (sDist2 > (Real)0)
                        {
                            // +++
                            AppendTriangle(posIndices, v0, v1, v2);
                        }
                        else if (sDist2 < (Real)0)
                        {
                            // ++-
                            SplitTrianglePPM(negIndices, posIndices, v0, v1, v2);
                        }
                        else
                        {
                            // ++0
                            AppendTriangle(posIndices, v0, v1, v2);
                        }
                    }
                    else if (sDist1 < (Real)0)
                    {
                        if (sDist2 > (Real)0)
                        {
                            // +-+
                            SplitTrianglePPM(negIndices, posIndices, v2, v0, v1);
                        }
                        else if (sDist2 < (Real)0)
                        {
                            // +--
                            SplitTriangleMMP(negIndices, posIndices, v1, v2, v0);
                        }
                        else
                        {
                            // +-0
                            SplitTrianglePMZ(negIndices, posIndices, v0, v1, v2);
                        }
                    }
                    else
                    {
                        if (sDist2 > (Real)0)
                        {
                            // +0+
                            AppendTriangle(posIndices, v0, v1, v2);
                        }
                        else if (sDist2 < (Real)0)
                        {
                            // +0-
                            SplitTriangleMPZ(negIndices, posIndices, v2, v0, v1);
                        }
                        else
                        {
                            // +00
                            AppendTriangle(posIndices, v0, v1, v2);
                        }
                    }
                }
                else if (sDist0 < (Real)0)
                {
                    if (sDist1 > (Real)0)
                    {
                        if (sDist2 > (Real)0)
                        {
                            // -++
                            SplitTrianglePPM(negIndices, posIndices, v1, v2, v0);
                        }
                        else if (sDist2 < (Real)0)
                        {
                            // -+-
                            SplitTriangleMMP(negIndices, posIndices, v2, v0, v1);
                        }
                        else
                        {
                            // -+0
                            SplitTriangleMPZ(negIndices, posIndices, v0, v1, v2);
                        }
                    }
                    else if (sDist1 < (Real)0)
                    {
                        if (sDist2 > (Real)0)
                        {
                            // --+
                            SplitTriangleMMP(negIndices, posIndices, v0, v1, v2);
                        }
                        else if (sDist2 < (Real)0)
                        {
                            // ---
                            AppendTriangle(negIndices, v0, v1, v2);
                        }
                        else
                        {
                            // --0
                            AppendTriangle(negIndices, v0, v1, v2);
                        }
                    }
                    else
                    {
                        if (sDist2 > (Real)0)
                        {
                            // -0+
                            SplitTrianglePMZ(negIndices, posIndices, v2, v0, v1);
                        }
                        else if (sDist2 < (Real)0)
                        {
                            // -0-
                            AppendTriangle(negIndices, v0, v1, v2);
                        }
                        else
                        {
                            // -00
                            AppendTriangle(negIndices, v0, v1, v2);
                        }
                    }
                }
                else
                {
                    if (sDist1 > (Real)0)
                    {
                        if (sDist2 > (Real)0)
                        {
                            // 0++
                            AppendTriangle(posIndices, v0, v1, v2);
                        }
                        else if (sDist2 < (Real)0)
                        {
                            // 0+-
                            SplitTrianglePMZ(negIndices, posIndices, v1, v2, v0);
                        }
                        else
                        {
                            // 0+0
                            AppendTriangle(posIndices, v0, v1, v2);
                        }
                    }
                    else if (sDist1 < (Real)0)
                    {
                        if (sDist2 > (Real)0)
                        {
                            // 0-+
                            SplitTriangleMPZ(negIndices, posIndices, v1, v2, v0);
                        }
                        else if (sDist2 < (Real)0)
                        {
                            // 0--
                            AppendTriangle(negIndices, v0, v1, v2);
                        }
                        else
                        {
                            // 0-0
                            AppendTriangle(negIndices, v0, v1, v2);
                        }
                    }
                    else
                    {
                        if (sDist2 > (Real)0)
                        {
                            // 00+
                            AppendTriangle(posIndices, v0, v1, v2);
                        }
                        else if (sDist2 < (Real)0)
                        {
                            // 00-
                            AppendTriangle(negIndices, v0, v1, v2);
                        }
                        else
                        {
                            // 000, reject triangles lying in the plane
                        }
                    }
                }
            }
        }

        void AppendTriangle(std::vector<int32_t>& indices, int32_t v0, int32_t v1, int32_t v2)
        {
            indices.push_back(v0);
            indices.push_back(v1);
            indices.push_back(v2);
        }

        void SplitTrianglePPM(std::vector<int32_t>& negIndices,
            std::vector<int32_t>& posIndices, int32_t v0, int32_t v1, int32_t v2)
        {
            int32_t v12 = mEMap[EdgeKey<false>(v1, v2)].second;
            int32_t v20 = mEMap[EdgeKey<false>(v2, v0)].second;
            posIndices.push_back(v0);
            posIndices.push_back(v1);
            posIndices.push_back(v12);
            posIndices.push_back(v0);
            posIndices.push_back(v12);
            posIndices.push_back(v20);
            negIndices.push_back(v2);
            negIndices.push_back(v20);
            negIndices.push_back(v12);
        }

        void SplitTriangleMMP(std::vector<int32_t>& negIndices,
            std::vector<int32_t>& posIndices, int32_t v0, int32_t v1, int32_t v2)
        {
            int32_t v12 = mEMap[EdgeKey<false>(v1, v2)].second;
            int32_t v20 = mEMap[EdgeKey<false>(v2, v0)].second;
            negIndices.push_back(v0);
            negIndices.push_back(v1);
            negIndices.push_back(v12);
            negIndices.push_back(v0);
            negIndices.push_back(v12);
            negIndices.push_back(v20);
            posIndices.push_back(v2);
            posIndices.push_back(v20);
            posIndices.push_back(v12);
        }

        void SplitTrianglePMZ(std::vector<int32_t>& negIndices,
            std::vector<int32_t>& posIndices, int32_t v0, int32_t v1, int32_t v2)
        {
            int32_t v01 = mEMap[EdgeKey<false>(v0, v1)].second;
            posIndices.push_back(v2);
            posIndices.push_back(v0);
            posIndices.push_back(v01);
            negIndices.push_back(v2);
            negIndices.push_back(v01);
            negIndices.push_back(v1);
        }

        void SplitTriangleMPZ(std::vector<int32_t>& negIndices,
            std::vector<int32_t>& posIndices, int32_t v0, int32_t v1, int32_t v2)
        {
            int32_t v01 = mEMap[EdgeKey<false>(v0, v1)].second;
            negIndices.push_back(v2);
            negIndices.push_back(v0);
            negIndices.push_back(v01);
            posIndices.push_back(v2);
            posIndices.push_back(v01);
            posIndices.push_back(v1);
        }

        // Stores the signed distances from the vertices to the plane.
        std::vector<Real> mSignedDistances;

        // Stores the edges whose vertices are on opposite sides of the
        // plane.  The key is a pair of indices into the vertex array.
        // The value is the point of intersection of the edge with the
        // plane and an index into m_kVertices (the index is larger or
        // equal to the number of vertices of incoming rkVertices).
        std::map<EdgeKey<false>, std::pair<Vector3<Real>, int32_t>> mEMap;
    };
}
