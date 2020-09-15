// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2Triangle2.h>
#include <Mathematics/Ray.h>

// The queries consider the triangle to be a solid.

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Ray2<Real>, Triangle2<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Ray2<Real> const& ray, Triangle2<Real> const& triangle)
        {
            Result result;
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
            // No additional information to compute.
        };

        Result operator()(Ray2<Real> const& ray, Triangle2<Real> const& triangle)
        {
            Result result;
            DoQuery(ray.origin, ray.direction, triangle, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = ray.origin + result.parameter[i] * ray.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector2<Real> const& rayOrigin,
            Vector2<Real> const& rayDirection, Triangle2<Real> const& triangle,
            Result& result)
        {
            FIQuery<Real, Line2<Real>, Triangle2<Real>>::DoQuery(rayOrigin,
                rayDirection, triangle, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the disk; the
                // t-interval is [t0,t1].  The ray intersects the disk as long
                // as [t0,t1] overlaps the ray t-interval [0,+infinity).
                std::array<Real, 2> rayInterval =
                    { (Real)0, std::numeric_limits<Real>::max() };
                FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> iiQuery;
                result.parameter = iiQuery(result.parameter, rayInterval).overlap;
            }
        }
    };
}
