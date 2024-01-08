// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2023.08.08

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Vector3.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <typename T>
    class MeshSmoother
    {
    public:
        MeshSmoother()
            :
            mNumVertices(0),
            mVertices(nullptr),
            mNumTriangles(0),
            mIndices(nullptr),
            mNormals{},
            mMeans{},
            mNeighborCounts{}
        {
        }

        virtual ~MeshSmoother() = default;

        // The input to operator() is a triangle mesh with the specified
        // vertex buffer and index buffer. The number of elements of
        // 'indices' must be a multiple of 3, each triple of indices
        // (3*t, 3*t+1, 3*t+2) representing the triangle with vertices
        // (vertices[3*t], vertices[3*t+1], vertices[3*t+2]).

        void operator()(size_t numVertices, Vector3<T>* vertices,
            size_t numTriangles, int32_t const* indices)
        {
            LogAssert(
                numVertices >= 3 && vertices != nullptr &&
                numTriangles >= 1 && indices != nullptr,
                "Invalid input.");

            mNumVertices = numVertices;
            mVertices = vertices;
            mNumTriangles = numTriangles;
            mIndices = indices;

            mNormals.resize(mNumVertices);
            mMeans.resize(mNumVertices);
            mNeighborCounts.resize(mNumVertices);

            // Count the number of vertex neighbors.
            std::fill(mNeighborCounts.begin(), mNeighborCounts.end(), 0);
            int32_t const* current = mIndices;
            for (size_t i = 0; i < mNumTriangles; ++i)
            {
                mNeighborCounts[*current++] += 2;
                mNeighborCounts[*current++] += 2;
                mNeighborCounts[*current++] += 2;
            }
        }

        void operator()(std::vector<Vector3<T>>& vertices,
            std::vector<int32_t> const& indices)
        {
            operator()(vertices.size(), vertices.data(),
                indices.size() / 3, indices.data());
        }

        inline size_t GetNumVertices() const
        {
            return mNumVertices;
        }

        inline Vector3<T> const* GetVertices() const
        {
            return mVertices;
        }

        inline size_t GetNumTriangles() const
        {
            return mNumTriangles;
        }

        inline int32_t const* GetIndices() const
        {
            return mIndices;
        }

        inline std::vector<Vector3<T>> const& GetNormals() const
        {
            return mNormals;
        }

        inline std::vector<Vector3<T>> const& GetMeans() const
        {
            return mMeans;
        }

        inline std::vector<size_t> const& GetNeighborCounts() const
        {
            return mNeighborCounts;
        }

        // Apply one iteration of the smoother. The input time is supported
        // for applications where the surface evolution is time-dependent.
        void Update(T t = static_cast<T>(0))
        {
            std::fill(mNormals.begin(), mNormals.end(), Vector3<T>::Zero());
            std::fill(mMeans.begin(), mMeans.end(), Vector3<T>::Zero());

            int32_t const* current = mIndices;
            for (size_t i = 0; i < mNumTriangles; ++i)
            {
                int32_t v0 = *current++;
                int32_t v1 = *current++;
                int32_t v2 = *current++;

                Vector3<T>& V0 = mVertices[v0];
                Vector3<T>& V1 = mVertices[v1];
                Vector3<T>& V2 = mVertices[v2];

                Vector3<T> edge1 = V1 - V0;
                Vector3<T> edge2 = V2 - V0;
                Vector3<T> normal = Cross(edge1, edge2);

                mNormals[v0] += normal;
                mNormals[v1] += normal;
                mNormals[v2] += normal;

                mMeans[v0] += V1 + V2;
                mMeans[v1] += V2 + V0;
                mMeans[v2] += V0 + V1;
            }

            for (size_t i = 0; i < mNumVertices; ++i)
            {
                Normalize(mNormals[i]);
                mMeans[i] /= static_cast<T>(mNeighborCounts[i]);
            }

            for (size_t i = 0; i < mNumVertices; ++i)
            {
                if (VertexInfluenced(i, t))
                {
                    Vector3<T> diff = mMeans[i] - mVertices[i];
                    T dotDifNor = Dot(diff, mNormals[i]);
                    Vector3<T> surfaceNormal = dotDifNor * mNormals[i];
                    Vector3<T> tangent = diff - surfaceNormal;

                    T tanWeight = GetTangentWeight(i, t);
                    T norWeight = GetNormalWeight(i, t);
                    mVertices[i] += tanWeight * tangent + norWeight * mNormals[i];
                }
            }
        }

    protected:
        // The input parameters are "size_t i, T t". They are unused by
        // default, so the names are hidden according to ANSI standards.
        virtual bool VertexInfluenced(size_t, T)
        {
            return true;
        }

        virtual T GetTangentWeight(size_t, T)
        {
            return static_cast<T>(0.5);
        }

        virtual T GetNormalWeight(size_t, T)
        {
            return static_cast<T>(0.0);
        }

        size_t mNumVertices;
        Vector3<T>* mVertices;
        size_t mNumTriangles;
        int32_t const* mIndices;

        std::vector<Vector3<T>> mNormals;
        std::vector<Vector3<T>> mMeans;
        std::vector<size_t> mNeighborCounts;
    };
}
