// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The planar mesh class is convenient for many applications involving
// searches for triangles containing a specified point.  A couple of
// issues can show up in practice when the input data to the constructors
// is very large (number of triangles on the order of 10^5 or larger).
//
// The first constructor builds an ETManifoldMesh mesh that contains
// std::map objects.  When such maps are large, the amount of time it
// takes to delete them is enormous.  Although you can control the level
// of debug support in MSVS 2013 (see _ITERATOR_DEBUG_LEVEL), turning off
// checking might very well affect other classes for which you want
// iterator checking to be on.  An alternative to reduce debugging time
// is to dynamically allocate the PlanarMesh object in the main thread but
// then launch another thread to delete the object and avoid stalling
// the main thread.  For example,
//
//    PlanarMesh<IT,CT,RT>* pmesh =
//        new PlanarMesh<IT,CT,RT>(numV, vertices, numT, indices);
//    <make various calls to pmesh>;
//    std::thread deleter = [pmesh](){ delete pmesh; };
//    deleter.detach();  // Do not wait for the thread to finish.
//
// The second constructor has the mesh passed in, but mTriIndexMap is used
// in both constructors and can take time to delete.
//
// The input mesh should be consistently oriented, say, the triangles are
// counterclockwise ordered.  The vertices should be consistent with this
// ordering.  However, floating-point rounding errors in generating the
// vertices can cause apparent fold-over of the mesh; that is, theoretically
// the vertex geometry supports counterclockwise geometry but numerical
// errors cause an inconsistency.  This can manifest in the mQuery.ToLine
// tests whereby cycles of triangles occur in the linear walk.  When cycles
// occur, GetContainingTriangle(P,startTriangle) will iterate numTriangle
// times before reporting that the triangle cannot be found, which is a
// very slow process (in debug or release builds).  The function
// GetContainingTriangle(P,startTriangle,visited) is provided to avoid the
// performance loss, trapping a cycle the first time and exiting, but
// again reporting that the triangle cannot be found.  If you know that the
// query should be (theoretically) successful, use the second version of
// GetContainingTriangle.  If it fails by returning -1, then perform an
// exhaustive search over the triangles.  For example,
//
//    int32_t triangle = pmesh->GetContainingTriangle(P,startTriangle,visited);
//    if (triangle >= 0)
//    {
//        <take action; for example, compute barycenteric coordinates>;
//    }
//    else
//    {
//        int32_t numTriangles = pmesh->GetNumTriangles();
//        for (triangle = 0; triangle < numTriangles; ++triangle)
//        {
//            if (pmesh->Contains(triangle, P))
//            {
//                <take action>;
//                break;
//            }
//        }
//        if (triangle == numTriangles)
//        {
//            <Triangle still not found, take appropriate action>;
//        }
//    }
//
// The PlanarMesh<*>::Contains function does not require the triangles to
// be ordered.

#include <Mathematics/ContPointInPolygon2.h>
#include <Mathematics/ETManifoldMesh.h>
#include <Mathematics/PrimalQuery2.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace gte
{
    template <typename InputType, typename ComputeType, typename RationalType>
    class PlanarMesh
    {
    public:
        // Construction.  The inputs must represent a manifold mesh of
        // triangles in the plane.  The index array must have 3*numTriangles
        // elements, each triple of indices representing a triangle in the
        // mesh.  Each index is into the 'vertices' array.
        PlanarMesh(int32_t numVertices, Vector2<InputType> const* vertices, int32_t numTriangles, int32_t const* indices)
            :
            mNumVertices(0),
            mVertices(nullptr),
            mNumTriangles(0)
        {
            LogAssert(numVertices >= 3 && vertices != nullptr && numTriangles >= 1
                && indices != nullptr, "Invalid input.");

            // Create a mesh in order to get adjacency information.
            int32_t const* current = indices;
            for (int32_t t = 0; t < numTriangles; ++t)
            {
                int32_t v0 = *current++;
                int32_t v1 = *current++;
                int32_t v2 = *current++;
                if (!mMesh.Insert(v0, v1, v2))
                {
                    // TODO: Fix this comment once the exception handling is
                    // tested.
                    //
                    // The 'mesh' object will throw on nonmanifold inputs.
                    return;
                }
            }

            // We have a valid mesh.
            CreateVertices(numVertices, vertices);

            // Build the adjacency graph using the triangle ordering implied
            // by the indices, not the mesh triangle map, to preserve the
            // triangle ordering of the input indices.
            mNumTriangles = numTriangles;
            int32_t const numIndices = 3 * numTriangles;
            mIndices.resize(numIndices);

            std::copy(indices, indices + numIndices, mIndices.begin());
            for (int32_t t = 0, vIndex = 0; t < numTriangles; ++t)
            {
                int32_t v0 = indices[vIndex++];
                int32_t v1 = indices[vIndex++];
                int32_t v2 = indices[vIndex++];
                TriangleKey<true> key(v0, v1, v2);
                mTriIndexMap.insert(std::make_pair(key, t));
            }

            mAdjacencies.resize(numIndices);
            auto const& tmap = mMesh.GetTriangles();
            for (int32_t t = 0, base = 0; t < numTriangles; ++t, base += 3)
            {
                int32_t v0 = indices[base];
                int32_t v1 = indices[base + 1];
                int32_t v2 = indices[base + 2];
                TriangleKey<true> key(v0, v1, v2);
                auto element = tmap.find(key);
                for (int32_t i = 0; i < 3; ++i)
                {
                    auto adj = element->second->T[i];
                    if (adj)
                    {
                        key = TriangleKey<true>(adj->V[0], adj->V[1], adj->V[2]);
                        mAdjacencies[static_cast<size_t>(base) + i] = mTriIndexMap.find(key)->second;
                    }
                    else
                    {
                        mAdjacencies[static_cast<size_t>(base) + i] = -1;
                    }
                }
            }
        }

        PlanarMesh(int32_t numVertices, Vector2<InputType> const* vertices, ETManifoldMesh const& mesh)
            :
            mNumVertices(0),
            mVertices(nullptr),
            mNumTriangles(0)
        {
            if (numVertices < 3 || !vertices || mesh.GetTriangles().size() < 1)
            {
                LogError("Invalid input in PlanarMesh constructor.");
            }

            // We have a valid mesh.
            CreateVertices(numVertices, vertices);

            // Build the adjacency graph using the triangle ordering implied
            // by the mesh triangle map.
            auto const& tmap = mesh.GetTriangles();
            mNumTriangles = static_cast<int32_t>(tmap.size());
            mIndices.resize(3 * static_cast<size_t>(mNumTriangles));

            int32_t tIndex = 0, vIndex = 0;
            for (auto const& element : tmap)
            {
                mTriIndexMap.insert(std::make_pair(element.first, tIndex++));
                for (int32_t i = 0; i < 3; ++i, ++vIndex)
                {
                    mIndices[vIndex] = element.second->V[i];
                }
            }

            mAdjacencies.resize(3 * static_cast<size_t>(mNumTriangles));
            vIndex = 0;
            for (auto const& element : tmap)
            {
                for (int32_t i = 0; i < 3; ++i, ++vIndex)
                {
                    auto adj = element.second->T[i];
                    if (adj)
                    {
                        TriangleKey<true> key(adj->V[0], adj->V[1], adj->V[2]);
                        mAdjacencies[vIndex] = mTriIndexMap.find(key)->second;
                    }
                    else
                    {
                        mAdjacencies[vIndex] = -1;
                    }
                }
            }
        }

        // Mesh information.
        inline int32_t GetNumVertices() const
        {
            return mNumVertices;
        }

        inline int32_t GetNumTriangles() const
        {
            return mNumTriangles;
        }

        inline Vector2<InputType> const* GetVertices() const
        {
            return mVertices;
        }

        inline int32_t const* GetIndices() const
        {
            return mIndices.data();
        }

        inline int32_t const* GetAdjacencies() const
        {
            return mAdjacencies.data();
        }

        // Containment queries.  The function GetContainingTriangle works
        // correctly when the planar mesh is a convex set.  If the mesh is not
        // convex, it is possible that the linear-walk search algorithm exits
        // the mesh before finding a containing triangle.  For example, a
        // C-shaped mesh can contain a point in the top branch of the "C".
        // A starting point in the bottom branch of the "C" will lead to the
        // search exiting the bottom branch and having no path to walk to the
        // top branch.  If your mesh is not convex and you want a correct
        // containment query, you will have to append "outside" triangles to
        // your mesh to form a convex set.
        int32_t GetContainingTriangle(Vector2<InputType> const& P, int32_t startTriangle = 0) const
        {
            Vector2<ComputeType> test{ P[0], P[1] };

            // Use triangle edges as binary separating lines.
            int32_t triangle = startTriangle;
            for (int32_t i = 0; i < mNumTriangles; ++i)
            {
                int32_t ibase = 3 * triangle;
                int32_t const* v = &mIndices[ibase];

                if (mQuery.ToLine(test, v[0], v[1]) > 0)
                {
                    triangle = mAdjacencies[ibase];
                    if (triangle == -1)
                    {
                        return -1;
                    }
                    continue;
                }

                if (mQuery.ToLine(test, v[1], v[2]) > 0)
                {
                    triangle = mAdjacencies[static_cast<size_t>(ibase) + 1];
                    if (triangle == -1)
                    {
                        return -1;
                    }
                    continue;
                }

                if (mQuery.ToLine(test, v[2], v[0]) > 0)
                {
                    triangle = mAdjacencies[static_cast<size_t>(ibase) + 2];
                    if (triangle == -1)
                    {
                        return -1;
                    }
                    continue;
                }

                return triangle;
            }

            return -1;
        }

        int32_t GetContainingTriangle(Vector2<InputType> const& P, int32_t startTriangle, std::set<int32_t>& visited) const
        {
            Vector2<ComputeType> test{ P[0], P[1] };
            visited.clear();

            // Use triangle edges as binary separating lines.
            int32_t triangle = startTriangle;
            for (int32_t i = 0; i < mNumTriangles; ++i)
            {
                visited.insert(triangle);
                int32_t ibase = 3 * triangle;
                int32_t const* v = &mIndices[ibase];

                if (mQuery.ToLine(test, v[0], v[1]) > 0)
                {
                    triangle = mAdjacencies[ibase];
                    if (triangle == -1 || visited.find(triangle) != visited.end())
                    {
                        return -1;
                    }
                    continue;
                }

                if (mQuery.ToLine(test, v[1], v[2]) > 0)
                {
                    triangle = mAdjacencies[static_cast<size_t>(ibase) + 1];
                    if (triangle == -1 || visited.find(triangle) != visited.end())
                    {
                        return -1;
                    }
                    continue;
                }

                if (mQuery.ToLine(test, v[2], v[0]) > 0)
                {
                    triangle = mAdjacencies[static_cast<size_t>(ibase) + 2];
                    if (triangle == -1 || visited.find(triangle) != visited.end())
                    {
                        return -1;
                    }
                    continue;
                }

                return triangle;
            }

            return -1;
        }

        bool GetVertices(int32_t t, std::array<Vector2<InputType>, 3>& vertices) const
        {
            if (0 <= t && t < mNumTriangles)
            {
                for (int32_t i = 0, vIndex = 3 * t; i < 3; ++i, ++vIndex)
                {
                    vertices[i] = mVertices[mIndices[vIndex]];
                }
                return true;
            }
            return false;
        }

        bool GetIndices(int32_t t, std::array<int32_t, 3>& indices) const
        {
            if (0 <= t && t < mNumTriangles)
            {
                for (int32_t i = 0, vIndex = 3 * t; i < 3; ++i, ++vIndex)
                {
                    indices[i] = mIndices[vIndex];
                }
                return true;
            }
            return false;
        }

        bool GetAdjacencies(int32_t t, std::array<int32_t, 3>& adjacencies) const
        {
            if (0 <= t && t < mNumTriangles)
            {
                for (int32_t i = 0, vIndex = 3 * t; i < 3; ++i, ++vIndex)
                {
                    adjacencies[i] = mAdjacencies[vIndex];
                }
                return true;
            }
            return false;
        }

        bool GetBarycentrics(int32_t t, Vector2<InputType> const& P, std::array<InputType, 3>& bary) const
        {
            std::array<int32_t, 3> indices;
            if (GetIndices(t, indices))
            {
                Vector2<RationalType> rtP{ P[0], P[1] };
                std::array<Vector2<RationalType>, 3> rtV;
                for (int32_t i = 0; i < 3; ++i)
                {
                    Vector2<ComputeType> const& V = mComputeVertices[indices[i]];
                    for (int32_t j = 0; j < 2; ++j)
                    {
                        rtV[i][j] = (RationalType)V[j];
                    }
                };

                std::array<RationalType, 3> rtBary{};
                if (ComputeBarycentrics(rtP, rtV[0], rtV[1], rtV[2], rtBary))
                {
                    for (int32_t i = 0; i < 3; ++i)
                    {
                        bary[i] = (InputType)rtBary[i];
                    }
                    return true;
                }
            }
            return false;
        }

        bool Contains(int32_t triangle, Vector2<InputType> const& P) const
        {
            Vector2<ComputeType> test{ P[0], P[1] };
            Vector2<ComputeType> v[3];
            size_t base = 3 * static_cast<size_t>(triangle);
            v[0] = mComputeVertices[mIndices[base + 0]];
            v[1] = mComputeVertices[mIndices[base + 1]];
            v[2] = mComputeVertices[mIndices[base + 2]];
            PointInPolygon2<ComputeType> pip(3, v);
            return pip.Contains(test);
        }

    public:
        void CreateVertices(int32_t numVertices, Vector2<InputType> const* vertices)
        {
            mNumVertices = numVertices;
            mVertices = vertices;
            mComputeVertices.resize(mNumVertices);
            for (int32_t i = 0; i < mNumVertices; ++i)
            {
                for (int32_t j = 0; j < 2; ++j)
                {
                    mComputeVertices[i][j] = (ComputeType)mVertices[i][j];
                }
            }
            mQuery.Set(mNumVertices, &mComputeVertices[0]);
        }

        int32_t mNumVertices;
        Vector2<InputType> const* mVertices;
        int32_t mNumTriangles;
        std::vector<int32_t> mIndices;
        ETManifoldMesh mMesh;
        std::map<TriangleKey<true>, int32_t> mTriIndexMap;
        std::vector<int32_t> mAdjacencies;
        std::vector<Vector2<ComputeType>> mComputeVertices;
        PrimalQuery2<ComputeType> mQuery;
    };
}
