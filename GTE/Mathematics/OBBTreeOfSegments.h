// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// Read the comments in OBBTree.h regarding tree construction.

#include <Mathematics/OBBTree.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace gte
{
    template <typename T>
    class OBBTreeOfSegments : public OBBTree<T>
    {
    public:
        OBBTreeOfSegments()
            :
            OBBTree<T>{},
            mVertices{},
            mSegments{}
        {
        }

        // The input height specifies the desired height of the tree and must
        // be no larger than 31. If std::numeric_limits<size_t>::max(), the
        // the entire tree is built and the actual height is computed from
        // centroids.size(). If larger than 31, the height is clamped to 31.
        void Create(
            std::vector<Vector3<T>> const& vertices,
            std::vector<std::array<size_t, 2>> const& segments,
            size_t height = std::numeric_limits<size_t>::max())
        {
            LogAssert(
                vertices.size() >= 2 && segments.size() > 0,
                "Invalid input.");

            mVertices = vertices;
            mSegments = segments;

            // Compute the triangle centroids.
            std::vector<Vector3<T>> centroids(mSegments.size());
            T const half = static_cast<T>(0.5);
            for (size_t i = 0; i < mSegments.size(); ++i)
            {
                auto const& seg = mSegments[i];
                centroids[i] = half * (mVertices[seg[0]] + mVertices[seg[1]]);
            }

            // Create the OBB tree for centroids.
            OBBTree<T>::Create(centroids, height);
        }

        // Member access.
        inline std::vector<Vector3<T>> const& GetVertices() const
        {
            return mVertices;
        }

        inline std::vector<std::array<size_t, 2>> const& GetSegments() const
        {
            return mSegments;
        }

    private:
        // Let C be the box center and let U0, U1 and U2 be the box axes.
        // Each input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.
        // The following code computes min(y0), max(y0), min(y1), max(y1),
        // min(y2), and max(y2). The box center is then adjusted to be
        //   C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1
        //        + 0.5*(min(y2)+max(y2))*U2
        virtual void ComputeInteriorBox(size_t i0, size_t i1, OrientedBox3<T>& box) override
        {
            OBBTree<T>::ComputeInteriorBox(i0, i1, box);

            Vector3<T> pmin = Vector3<T>::Zero(), pmax = pmin;
            for (size_t i = i0; i <= i1; ++i)
            {
                auto const& seg = mSegments[this->mPartition[i]];
                for (size_t k = 0; k < 2; ++k)
                {
                    Vector3<T> diff = mVertices[seg[k]] - box.center;
                    for (int32_t j = 0; j < 3; ++j)
                    {
                        T dot = Dot(diff, box.axis[j]);
                        if (dot < pmin[j])
                        {
                            pmin[j] = dot;
                        }
                        else if (dot > pmax[j])
                        {
                            pmax[j] = dot;
                        }
                    }
                }
            }

            T const half = static_cast<T>(0.5);
            for (int32_t j = 0; j < 3; ++j)
            {
                box.center += (half * (pmin[j] + pmax[j])) * box.axis[j];
                box.extent[j] = half * (pmax[j] - pmin[j]);
            }
        }

        virtual void ComputeLeafBox(size_t i, OrientedBox3<T>& box) override
        {
            // Create a degenerate box whose center is the midpoint of the
            // segment primitive, whose axis[0] is the segment direction and
            // whose extent[0] is half the length of the segment.
            auto const& seg = mSegments[this->mPartition[i]];
            box.center = this->mCentroids[this->mPartition[i]];
            box.axis[0] = mVertices[seg[1]] - mVertices[seg[0]];
            box.extent[0] = static_cast<T>(0.5) * Normalize(box.axis[0]);
            ComputeOrthogonalComplement(1, box.axis.data());
            box.extent[1] = static_cast<T>(0);
            box.extent[2] = static_cast<T>(0);
        }

        std::vector<Vector3<T>> mVertices;
        std::vector<std::array<size_t, 2>> mSegments;
    };
}
