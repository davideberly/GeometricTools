// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.3.2026.08.08

#pragma once

// Class AlignedBoxBV is a bounding volume that supports the queries based on
// BVTree and its derived classes.

#include <Mathematics/IntrLine3AlignedBox3.h>
#include <Mathematics/IntrRay3AlignedBox3.h>
#include <Mathematics/IntrSegment3AlignedBox3.h>
#include <cstdint>

namespace gte
{
    template <typename T>
    class AlignedBoxBV
    {
    public:
        AlignedBoxBV()
            :
            box{}
        {
        }

        void GetSplittingAxis(
            Vector3<T>& origin,
            Vector3<T>& direction) const
        {
            T const half = static_cast<T>(0.5);

            origin = half * (box.max + box.min);

            Vector3<T> extents = half * (box.max - box.min);
            T maxExtent = extents[0];
            std::int32_t maxIndex = 0;
            if (extents[1] > maxExtent)
            {
                maxExtent = extents[1];
                maxIndex = 1;
            }
            if (extents[2] > maxExtent)
            {
                maxExtent = extents[2];
                maxIndex = 2;
            }
            direction = Vector3<T>::Unit(maxIndex);
        }

        static bool IntersectLine(
            Vector3<T> const& P,
            Vector3<T> const& Q,
            AlignedBoxBV<T> const& boundingVolume)
        {
            TIQuery<T, Line3<T>, AlignedBox3<T>> query{};
            auto output = query(Line3<T>(P, Q), boundingVolume.box);
            return output.intersect;
        }

        static bool IntersectRay(
            Vector3<T> const& P,
            Vector3<T> const& Q,
            AlignedBoxBV<T> const& boundingVolume)
        {
            TIQuery<T, Ray3<T>, AlignedBox3<T>> query{};
            auto output = query(Ray3<T>(P, Q), boundingVolume.box);
            return output.intersect;
        }

        static bool IntersectSegment(
            Vector3<T> const& P,
            Vector3<T> const& Q,
            AlignedBoxBV<T> const& boundingVolume)
        {
            TIQuery<T, Segment3<T>, AlignedBox3<T>> query{};
            auto output = query(Segment3<T>(P, Q), boundingVolume.box);
            return output.intersect;
        }

        AlignedBox3<T> box;
    };
}
