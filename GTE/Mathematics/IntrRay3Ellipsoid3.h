// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the ellipsoid to be a solid.
//
// The ellipsoid is (X-C)^T*M*(X-C)-1 = 0 and the ray is X = P+t*D for t >= 0.
// Substitute the ray equation into the ellipsoid equation to obtain a
// quadratic equation Q(t) = a2*t^2 + 2*a1*t + a0 = 0, where a2 = D^T*M*D,
// a1 = D^T*M*(P-C) and a0 = (P-C)^T*M*(P-C)-1. The algorithm involves an
// analysis of the real-valued roots of Q(t) for t >= 0.

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3Ellipsoid3.h>
#include <Mathematics/Ray.h>
#include <Mathematics/Matrix3x3.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray3<T>, Ellipsoid3<T>>
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

        Result operator()(Ray3<T> const& ray, Ellipsoid3<T> const& ellipsoid)
        {
            Result result{};

            Matrix3x3<T> M{};
            ellipsoid.GetM(M);
            T const zero = static_cast<T>(0);
            Vector3<T> diff = ray.origin - ellipsoid.center;
            Vector3<T> matDir = M * ray.direction;
            Vector3<T> matDiff = M * diff;
            T a0 = Dot(diff, matDiff) - static_cast<T>(1);
            if (a0 <= zero)
            {
                // P is inside the ellipsoid.
                result.intersect = true;
                return result;
            }
            // else: P is outside the ellipsoid

            T a1 = Dot(ray.direction, matDiff);
            if (a1 >= zero)
            {
                // Q(t) >= a0 > 0 for t >= 0, so Q(t) cannot be zero for
                // t in [0,+infinity) and the ray does not intersect the
                // ellipsoid.
                result.intersect = false;
                return result;
            }

            // The minimum of Q(t) occurs for some t in (0,+infinity). An
            // intersection occurs when Q(t) has real roots.
            T a2 = Dot(ray.direction, matDir);
            T discr = a1 * a1 - a0 * a2;
            result.intersect = (discr >= zero);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Ray3<T>, Ellipsoid3<T>>
        :
        public FIQuery<T, Line3<T>, Ellipsoid3<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line3<T>, Ellipsoid3<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line3<T>, Ellipsoid3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray3<T> const& ray, Ellipsoid3<T> const& ellipsoid)
        {
            Result result{};
            DoQuery(ray.origin, ray.direction, ellipsoid, result);
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
            Vector3<T> const& rayDirection, Ellipsoid3<T> const& ellipsoid,
            Result& result)
        {
            FIQuery<T, Line3<T>, Ellipsoid3<T>>::DoQuery(rayOrigin,
                rayDirection, ellipsoid, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the ellipsoid; the
                // t-interval is [t0,t1]. The ray intersects the capsule as
                // long as [t0,t1] overlaps the ray t-interval [0,+infinity).
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                auto iiResult = iiQuery(result.parameter, static_cast<T>(0), true);
                if (iiResult.intersect)
                {
                    result.numIntersections = iiResult.numIntersections;
                    result.parameter = iiResult.overlap;
                }
                else
                {
                    // The line containing the ray does not intersect the
                    // ellipsoid.
                    result = Result{};
                }
            }
        }
    };
}
