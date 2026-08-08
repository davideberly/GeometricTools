// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.3.2026.08.08

#pragma once

#include <Mathematics/ContOrientedBox3.h>
#include <Mathematics/OrientedBoxBV.h>
#include <Mathematics/BVTreeOfTriangles.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <typename T>
    class OrientedBoxTreeOfTriangles : public BVTreeOfTriangles<T, OrientedBoxBV<T>>
    {
    public:
        OrientedBoxTreeOfTriangles()
            :
            BVTreeOfTriangles<T, OrientedBoxBV<T>>{}
        {
        }

    protected:
        // The bounding volume for the primitives' vertices depends on the
        // type of primitive. A derived class representing a primitive tree
        // must implement this.
        virtual void ComputeInteriorBoundingVolume(std::size_t i0, std::size_t i1,
            OrientedBoxBV<T>& boundingVolume) override
        {
            auto& box = boundingVolume.box;

            std::vector<Vector3<T>> localVertices{};
            localVertices.reserve(3 * (i1 - i0 + 1));
            for (std::size_t i = i0; i <= i1; ++i)
            {
                auto const& tri = this->mTriangles[this->mPartition[i]];
                localVertices.push_back(this->mVertices[tri[0]]);
                localVertices.push_back(this->mVertices[tri[1]]);
                localVertices.push_back(this->mVertices[tri[2]]);
            }
            GetContainer(localVertices, box);
        }

        // The bounding volume for a single primitive's vertices depends on
        // the type of primitive. A derived class representing a primitive
        // tree must implement this.
        virtual void ComputeLeafBoundingVolume(
            std::size_t i,
            OrientedBoxBV<T>& boundingVolume) override
        {
            auto& box = boundingVolume.box;

            auto const& tri = this->mTriangles[this->mPartition[i]];
            std::vector<Vector3<T>> localVertices =
            {
                this->mVertices[tri[0]],
                this->mVertices[tri[1]],
                this->mVertices[tri[2]]
            };
            GetContainer(localVertices, box);

            // Numerical rounding errors in the Gaussian fit of GetContainer
            // will lead to 1 extent nearly zero. Locate it and set it to
            // zero.
            T minAbsExtent = std::fabs(box.extent[0]);
            std::int32_t minIndex = 0;
            T absExtent = std::fabs(box.extent[1]);
            if (absExtent < minAbsExtent)
            {
                minAbsExtent = absExtent;
                minIndex = 1;
            }
            absExtent = std::fabs(box.extent[2]);
            if (absExtent > minAbsExtent)
            {
                minAbsExtent = absExtent;
                minIndex = 2;
            }

            box.extent[minIndex] = static_cast<T>(0);
        }

    private:
        friend class UnitTestOrientedBoxTreeOfTriangles;
    };
}
