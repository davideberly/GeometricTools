// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.3.2026.08.08

#pragma once

#include <Mathematics/AlignedBoxBV.h>
#include <Mathematics/BVTreeOfSegments.h>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class AlignedBoxTreeOfSegments : public BVTreeOfSegments<T, AlignedBoxBV<T>>
    {
    public:
        AlignedBoxTreeOfSegments()
            :
            BVTreeOfSegments<T, AlignedBoxBV<T>>{}
        {
        }

    protected:
        // The bounding volume for the primitives' vertices depends on the
        // type of primitive. A derived class representing a primitive tree
        // must implement this.
        virtual void ComputeInteriorBoundingVolume(
            std::size_t i0,
            std::size_t i1,
            AlignedBoxBV<T>& boundingVolume) override
        {
            auto& box = boundingVolume.box;
            auto const& initialSeg = this->mSegments[this->mPartition[i0]];
            box.min = this->mVertices[initialSeg[0]];
            box.max = box.min;

            for (std::size_t i = i0; i <= i1; ++i)
            {
                auto const& seg = this->mSegments[this->mPartition[i]];
                for (std::size_t j = 0; j < 2; ++j)
                {
                    Vector3<T> const& vertex = this->mVertices[seg[j]];
                    for (std::int32_t k = 0; k < 3; ++k)
                    {
                        if (vertex[k] < box.min[k])
                        {
                            box.min[k] = vertex[k];
                        }
                        else if (vertex[k] > box.max[k])
                        {
                            box.max[k] = vertex[k];
                        }
                    }
                }
            }
        }

        // The bounding volume for a single primitive's vertices depends on
        // the type of primitive. A derived class representing a primitive
        // tree must implement this.
        virtual void ComputeLeafBoundingVolume(
            std::size_t i,
            AlignedBoxBV<T>& boundingVolume) override
        {
            auto& box = boundingVolume.box;
            auto const& seg = this->mSegments[this->mPartition[i]];
            box.min = this->mVertices[seg[0]];
            box.max = box.min;

            Vector3<T> const& vertex = this->mVertices[seg[1]];
            for (std::int32_t k = 0; k < 3; ++k)
            {
                if (vertex[k] < box.min[k])
                {
                    box.min[k] = vertex[k];
                }
                else if (vertex[k] > box.max[k])
                {
                    box.max[k] = vertex[k];
                }
            }
        }

    private:
        friend class UnitTestAlignedBoxTreeOfSegments;
    };
}
