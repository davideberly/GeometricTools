// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the sphere to be a solid.
//
// The sphere is (X-C)^T*(X-C)-r^2 = 0 and the ray is X = P+t*D for t >= 0.
// Substitute the ray equation into the sphere equation to obtain a quadratic
// equation Q(t) = t^2 + 2*a1*t + a0 = 0, where a1 = D^T*(P-C) and
// a0 = (P-C)^T*(P-C)-r^2. The algorithm involves an analysis of the
// real-valued roots of Q(t) for t >= 0.

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3Sphere3.h>
#include <Mathematics/Ray.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray3<T>, Sphere3<T>>
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

        Result operator()(Ray3<T> const& ray, Sphere3<T> const& sphere)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            Vector3<T> diff = ray.origin - sphere.center;
            T a0 = Dot(diff, diff) - sphere.radius * sphere.radius;
            if (a0 <= zero)
            {
                // P is inside the sphere.
                result.intersect = true;
                return result;
            }
            // else: P is outside the sphere

            T a1 = Dot(ray.direction, diff);
            if (a1 >= zero)
            {
                result.intersect = false;
                return result;
            }

            // Intersection occurs when Q(t) has real roots.
            T discr = a1 * a1 - a0;
            result.intersect = (discr >= zero);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Ray3<T>, Sphere3<T>>
        :
        public FIQuery<T, Line3<T>, Sphere3<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line3<T>, Sphere3<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line3<T>, Sphere3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray3<T> const& ray, Sphere3<T> const& sphere)
        {
            Result result{};
            DoQuery(ray.origin, ray.direction, sphere, result);
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
            Vector3<T> const& rayDirection, Sphere3<T> const& sphere,
            Result& result)
        {
            FIQuery<T, Line3<T>, Sphere3<T>>::DoQuery(
                rayOrigin, rayDirection, sphere, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the sphere; the
                // t-interval is [t0,t1]. The ray intersects the sphere as
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
                    // sphere.
                    result = Result{};
                }
            }
        }
    };
}
