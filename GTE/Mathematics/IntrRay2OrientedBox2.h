// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the box to be a solid.
//
// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the four
// edges of the box.

#include <Mathematics/IntrRay2AlignedBox2.h>
#include <Mathematics/OrientedBox.h>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray2<T>, OrientedBox2<T>>
        :
        public TIQuery<T, Ray2<T>, AlignedBox2<T>>
    {
    public:
        struct Result
            :
            public TIQuery<T, Ray2<T>, AlignedBox2<T>>::Result
        {
            Result()
                :
                TIQuery<T, Ray2<T>, AlignedBox2<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray2<T> const& ray, OrientedBox2<T> const& box)
        {
            // Transform the ray to the oriented-box coordinate system.
            Vector2<T> diff = ray.origin - box.center;
            Vector2<T> rayOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1])
            };
            Vector2<T> rayDirection = Vector2<T>
            {
                Dot(ray.direction, box.axis[0]),
                Dot(ray.direction, box.axis[1])
            };

            Result result{};
            this->DoQuery(rayOrigin, rayDirection, box.extent, result);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Ray2<T>, OrientedBox2<T>>
        :
        public FIQuery<T, Ray2<T>, AlignedBox2<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Ray2<T>, AlignedBox2<T>>::Result
        {
            Result()
                :
                FIQuery<T, Ray2<T>, AlignedBox2<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray2<T> const& ray, OrientedBox2<T> const& box)
        {
            // Transform the ray to the oriented-box coordinate system.
            Vector2<T> diff = ray.origin - box.center;
            Vector2<T> rayOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1])
            };
            Vector2<T> rayDirection = Vector2<T>
            {
                Dot(ray.direction, box.axis[0]),
                Dot(ray.direction, box.axis[1])
            };

            Result result{};
            this->DoQuery(rayOrigin, rayDirection, box.extent, result);
            for (int32_t i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = ray.origin + result.parameter[i] * ray.direction;
            }
            return result;
        }
    };
}
