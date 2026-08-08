// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.3.2026.08.08

#pragma once

#include <Mathematics/ContOrientedBox3.h>
#include <Mathematics/OrientedBoxBV.h>
#include <Mathematics/BVTreeOfPoints.h>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <typename T>
    class OrientedBoxTreeOfPoints : public BVTreeOfPoints<T, OrientedBoxBV<T>>
    {
    public:
        OrientedBoxTreeOfPoints()
            :
            BVTreeOfPoints<T, OrientedBoxBV<T>>{}
        {
        }

    protected:
        // Let C be the box center and let U0, U1 and U2 be the box axes.
        // Each input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.
        // The following code computes min(y0), max(y0), min(y1), max(y1),
        // min(y2), and max(y2). The box center is then adjusted to be
        //   C' = C + 0.5 * (min(y0) + max(y0)) * U0
        //          + 0.5 * (min(y1) + max(y1)) * U1
        //          + 0.5 * (min(y2) + max(y2)) * U2
        virtual void ComputeInteriorBoundingVolume(
            std::size_t i0,
            std::size_t i1,
            OrientedBoxBV<T>& boundingVolume) override
        {
            auto& box = boundingVolume.box;

            std::vector<Vector3<T>> localCentroids{};
            localCentroids.reserve(i1 - i0 + 1);
            for (std::size_t i = i0; i <= i1; ++i)
            {
                localCentroids.push_back(this->mCentroids[this->mPartition[i]]);
            }

            GetContainer(localCentroids, box);
        }

        // The bounding volume for a single primitive's vertices depends on
        // the type of primitive. A derived class representing a primitive
        // tree must implement this.
        virtual void ComputeLeafBoundingVolume(
            std::size_t i,
            OrientedBoxBV<T>& boundingVolume) override
        {
            auto& box = boundingVolume.box;

            box.center = this->mCentroids[this->mPartition[i]];
            for (std::int32_t j = 0; j < 3; ++j)
            {
                box.axis[j] = Vector3<T>::Unit(j);
                box.extent[j] = static_cast<T>(0);
            }
        }

    private:
        friend class UnitTestOrientedBoxTreeOfPoints;
    };
}
