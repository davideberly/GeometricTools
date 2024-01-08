// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the circle to be a solid (disk).

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2Circle2.h>
#include <Mathematics/Ray.h>
#include <array>
#include <cstdint>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray2<T>, Circle2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Ray2<T> const& ray, Circle2<T> const& circle)
        {
            Result result{};
            FIQuery<T, Ray2<T>, Circle2<T>> rcQuery{};
            result.intersect = rcQuery(ray, circle).intersect;
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Ray2<T>, Circle2<T>>
        :
        public FIQuery<T, Line2<T>, Circle2<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line2<T>, Circle2<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line2<T>, Circle2<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray2<T> const& ray, Circle2<T> const& circle)
        {
            Result result{};
            DoQuery(ray.origin, ray.direction, circle, result);
            for (int32_t i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = ray.origin + result.parameter[i] * ray.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector2<T> const& rayOrigin,
            Vector2<T> const& rayDirection, Circle2<T> const& circle,
            Result& result)
        {
            FIQuery<T, Line2<T>, Circle2<T>>::DoQuery(rayOrigin,
                rayDirection, circle, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the disk; the
                // t-interval is [t0,t1].  The ray intersects the disk as long
                // as [t0,t1] overlaps the ray t-interval [0,+infinity).
                std::array<T, 2> rayInterval = { (T)0, std::numeric_limits<T>::max() };
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                auto iiResult = iiQuery(result.parameter, rayInterval);
                result.intersect = iiResult.intersect;
                result.numIntersections = iiResult.numIntersections;
                result.parameter = iiResult.overlap;
            }
        }
    };
}
