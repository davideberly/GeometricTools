// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// Read the comments in OBBTree.h regarding tree construction. The IndexType
// can be any integral type that does not include bool, but the IndexType
// values must be nonnegative because they are indices into vertices[].

#include <Mathematics/OBBTree.h>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace gte
{
    template <typename T>
    class OBBTreeOfPoints : public OBBTree<T>
    {
    public:
        OBBTreeOfPoints()
            :
            OBBTree<T>{}
        {
        }

        // The input height specifies the desired height of the tree and must
        // be no larger than 31. If std::numeric_limits<size_t>::max(), the
        // the entire tree is built and the actual height is computed from
        // centroids.size(). If larger than 31, the height is clamped to 31.
        void Create(
            std::vector<Vector3<T>> const& points,
            size_t height = std::numeric_limits<size_t>::max())
        {
            // Create the OBB tree for centroids. The points are already the
            // centroids.
            OBBTree<T>::Create(points, height);
        }

        // Member access.
        inline std::vector<Vector3<T>> const& GetPoints() const
        {
            return this->mCentroids;
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
                Vector3<T> diff = this->mCentroids[this->mPartition[i]] - box.center;
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

            T const half = static_cast<T>(0.5);
            for (int32_t j = 0; j < 3; ++j)
            {
                box.center += (half * (pmin[j] + pmax[j])) * box.axis[j];
                box.extent[j] = half * (pmax[j] - pmin[j]);
            }
        }

        virtual void ComputeLeafBox(size_t i, OrientedBox3<T>& box) override
        {
            // Create a degenerate box whose center is the point primitive.
            box.center = this->mCentroids[this->mPartition[i]];
            box.axis[0] = Vector3<T>::Unit(0);
            box.axis[1] = Vector3<T>::Unit(1);
            box.axis[2] = Vector3<T>::Unit(2);
            box.extent = Vector3<T>::Zero();
        }
    };
}
