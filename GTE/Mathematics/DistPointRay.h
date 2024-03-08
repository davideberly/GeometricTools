// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.27

#pragma once

// Compute the distance between a point and a ray in nD.
// 
// The ray is P + t * D for t >= 0, where D is not required to be unit length.
// 
// The input point is stored in closest[0]. The closest point on the ray is
// stored in closest[1].

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Ray.h>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Vector<N, T>, Ray<N, T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                parameter(static_cast<T>(0)),
                closest{ Vector<N, T>::Zero(), Vector<N, T>::Zero() }
            {
            }

            T distance, sqrDistance;
            T parameter;
            std::array<Vector<N, T>, 2> closest;
        };

        Result operator()(Vector<N, T> const& point, Ray<N, T> const& ray)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            Vector<N, T> diff = point - ray.origin;
            result.parameter = Dot(ray.direction, diff) / Dot(ray.direction, ray.direction);
            result.closest[0] = point;
            if (result.parameter > zero)
            {
                result.closest[1] = ray.origin + result.parameter * ray.direction;
            }
            else
            {
                result.parameter = zero;
                result.closest[1] = ray.origin;
            }
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);

            return result;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPPointRay = DCPQuery<T, Vector<N, T>, Ray<N, T>>;

    template <typename T>
    using DCPPoint2Ray2 = DCPPointRay<2, T>;

    template <typename T>
    using DCPPoint3Ray3 = DCPPointRay<3, T>;
}
