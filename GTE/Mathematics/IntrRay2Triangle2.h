// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the triangle to be a solid.

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2Triangle2.h>
#include <Mathematics/Ray.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Ray2<Real>, Triangle2<Real>>
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

        // The ray is P + t * D, where P is a point on the line and D is a
        // direction vector that does not have to be unit length. This is
        // useful when using a 2-point representation P0 + t * (P1 - P0). The
        // t-parameter is constrained by t >= 0.
        Result operator()(Ray2<Real> const& ray, Triangle2<Real> const& triangle)
        {
            Result result{};
            FIQuery<Real, Ray2<Real>, Triangle2<Real>> rtQuery;
            result.intersect = rtQuery(ray, triangle).intersect;
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Ray2<Real>, Triangle2<Real>>
        :
        public FIQuery<Real, Line2<Real>, Triangle2<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line2<Real>, Triangle2<Real>>::Result
        {
            Result()
                :
                FIQuery<Real, Line2<Real>, Triangle2<Real>>::Result{}
            {
            }

            // No additional information to compute.
        };

        // The line is P + t * D, where P is a point on the line and D is a
        // direction vector that does not have to be unit length. This is
        // useful when using a 2-point representation P0 + t * (P1 - P0).
        Result operator()(Ray2<Real> const& ray, Triangle2<Real> const& triangle)
        {
            Result result{};
            DoQuery(ray.origin, ray.direction, triangle, result);
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
        void DoQuery(Vector2<Real> const& origin, Vector2<Real> const& direction,
            Triangle2<Real> const& triangle, Result& result)
        {
            FIQuery<Real, Line2<Real>, Triangle2<Real>>::DoQuery(
                origin, direction, triangle, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the triangle; the
                // t-interval is [t0,t1]. The ray intersects the triangle as
                // long as [t0,t1] overlaps the ray t-interval [0,+infinity).
                FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> iiQuery{};
                auto iiResult = iiQuery(result.parameter, static_cast<Real>(0), true);
                if (iiResult.intersect)
                {
                    result.numIntersections = iiResult.numIntersections;
                    result.parameter = iiResult.overlap;
                }
                else
                {
                    // The line containing the ray does not intersect the
                    // triangle.
                    result = Result{};
                }
            }
        }
    };
}
