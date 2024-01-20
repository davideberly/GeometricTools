// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.1.2024.01.20

#pragma once

#include <Mathematics/IntrLine3AlignedBox3.h>
#include <Mathematics/IntrRay3AlignedBox3.h>
#include <Mathematics/IntrSegment3AlignedBox3.h>
#include <Mathematics/BVTreeOfTriangles.h>

namespace gte
{
    template <typename T>
    struct AABBBoundingVolume
    {
        AABBBoundingVolume()
            :
            box{}
        {
        }

        void GetSplittingAxis(Vector3<T>& origin, Vector3<T>& direction) const
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const half = static_cast<T>(0.5);

            origin = half * (box.max + box.min);
            Vector3<T> extents = half * (box.max - box.min);
            T projectionExtent = extents[0];
            direction = { one, zero, zero };
            if (extents[1] > projectionExtent)
            {
                projectionExtent = extents[1];
                direction = { zero, one, zero };
            }
            if (extents[2] > projectionExtent)
            {
                projectionExtent = extents[2];
                direction = { zero, zero, one };
            }
        }

        static bool IntersectLine(Vector3<T> const& P, Vector3<T> const& Q,
            AABBBoundingVolume<T> const& boundingVolume)
        {
            TIQuery<T, Line3<T>, AlignedBox3<T>> query{};
            auto result = query(Line3<T>(P, Q), boundingVolume.box);
            return result.intersect;
        }

        static bool IntersectRay(Vector3<T> const& P, Vector3<T> const& Q,
            AABBBoundingVolume<T> const& boundingVolume)
        {
            TIQuery<T, Ray3<T>, AlignedBox3<T>> query{};
            auto result = query(Ray3<T>(P, Q), boundingVolume.box);
            return result.intersect;
        }

        static bool IntersectSegment(Vector3<T> const& P, Vector3<T> const& Q,
            AABBBoundingVolume<T> const& boundingVolume)
        {
            TIQuery<T, Segment3<T>, AlignedBox3<T>> query{};
            auto result = query(Segment3<T>(P, Q), boundingVolume.box);
            return result.intersect;
        }

        AlignedBox3<T> box;
    };

    template <typename T>
    class AABBBVTreeOfTriangles
        : public BVTreeOfTriangles<T, AABBBoundingVolume<T>>
    {
    public:
        AABBBVTreeOfTriangles()
            :
            BVTreeOfTriangles<T, AABBBoundingVolume<T>>{}
        {
        }

    protected:
        // The bounding volume for the primitives' vertices depends on the
        // type of primitive. A derived class representing a primitive tree
        // must implement this.
        virtual void ComputeInteriorBoundingVolume(size_t i0, size_t i1,
            AABBBoundingVolume<T>& boundingVolume) override
        {
            auto& box = boundingVolume.box;
            auto const& initialTri = this->mTriangles[this->mPartition[i0]];
            box.min = this->mVertices[initialTri[0]];
            box.max = box.min;

            for (size_t i = i0; i <= i1; ++i)
            {
                auto const& tri = this->mTriangles[this->mPartition[i]];
                for (size_t j = 0; j < 3; ++j)
                {
                    Vector3<T> const& vertex = this->mVertices[tri[j]];
                    for (int32_t k = 0; k < 3; ++k)
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
        virtual void ComputeLeafBoundingVolume(size_t i,
            AABBBoundingVolume<T>& boundingVolume) override
        {
            auto& box = boundingVolume.box;
            auto const& tri = this->mTriangles[this->mPartition[i]];
            box.min = this->mVertices[tri[0]];
            box.max = box.min;
            for (size_t j = 1; j < 3; ++j)
            {
                Vector3<T> const& vertex = this->mVertices[tri[j]];
                for (int32_t k = 0; k < 3; ++k)
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
    };
}
