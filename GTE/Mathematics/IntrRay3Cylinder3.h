// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the cylinder to be a solid.

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3Cylinder3.h>
#include <Mathematics/Ray.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class FIQuery<T, Ray3<T>, Cylinder3<T>>
        :
        public FIQuery<T, Line3<T>, Cylinder3<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line3<T>, Cylinder3<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line3<T>, Cylinder3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray3<T> const& ray, Cylinder3<T> const& cylinder)
        {
            Result result{};
            DoQuery(ray.origin, ray.direction, cylinder, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = ray.origin + result.parameter[i] * ray.direction;
                }
            }
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector3<T> const& rayOrigin,
            Vector3<T> const& rayDirection, Cylinder3<T> const& cylinder,
            Result& result)
        {
            FIQuery<T, Line3<T>, Cylinder3<T>>::DoQuery(
                rayOrigin, rayDirection, cylinder, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the cylinder; the
                // t-interval is [t0,t1].  The ray intersects the cylinder as
                // long as [t0,t1] overlaps the ray t-interval [0,+infinity).
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery;
                auto iiResult = iiQuery(result.parameter, static_cast<T>(0), true);
                if (iiResult.intersect)
                {
                    result.numIntersections = iiResult.numIntersections;
                    result.parameter = iiResult.overlap;
                }
                else
                {
                    // The line containing the ray does not intersect the
                    // cylinder.
                    result = Result{};
                }
            }
        }
    };
}
