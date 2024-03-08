// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the six
// faces of the box.  The find-intersection queries use Liang-Barsky
// clipping.  The queries consider the box to be a solid.  The algorithms
// are described in
// https://www.geometrictools.com/Documentation/IntersectionLineBox.pdf

#include <Mathematics/IntrRay3AlignedBox3.h>
#include <Mathematics/OrientedBox.h>
#include <cstddef>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray3<T>, OrientedBox3<T>>
        :
        public TIQuery<T, Ray3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
            :
            public TIQuery<T, Ray3<T>, AlignedBox3<T>>::Result
        {
            Result()
                :
                TIQuery<T, Ray3<T>, AlignedBox3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray3<T> const& ray, OrientedBox3<T> const& box)
        {
            // Transform the ray to the oriented-box coordinate system.
            Vector3<T> diff = ray.origin - box.center;
            Vector3<T> rayOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1]),
                Dot(diff, box.axis[2])
            };
            Vector3<T> rayDirection = Vector3<T>
            {
                Dot(ray.direction, box.axis[0]),
                Dot(ray.direction, box.axis[1]),
                Dot(ray.direction, box.axis[2])
            };

            Result result{};
            this->DoQuery(rayOrigin, rayDirection, box.extent, result);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Ray3<T>, OrientedBox3<T>>
        :
        public FIQuery<T, Ray3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Ray3<T>, AlignedBox3<T>>::Result
        {
            Result()
                :
                FIQuery<T, Ray3<T>, AlignedBox3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray3<T> const& ray, OrientedBox3<T> const& box)
        {
            // Transform the ray to the oriented-box coordinate system.
            Vector3<T> diff = ray.origin - box.center;
            Vector3<T> rayOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1]),
                Dot(diff, box.axis[2])
            };
            Vector3<T> rayDirection = Vector3<T>
            {
                Dot(ray.direction, box.axis[0]),
                Dot(ray.direction, box.axis[1]),
                Dot(ray.direction, box.axis[2])
            };

            Result result{};
            this->DoQuery(rayOrigin, rayDirection, box.extent, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = ray.origin + result.parameter[i] * ray.direction;
                }
            }
            return result;
        }
    };
}
