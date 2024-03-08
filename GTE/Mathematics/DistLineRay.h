// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a line and a ray in nD.
// 
// The line is P[0] + s[0] * D[0] and the ray is P[1] + s[1] * D[1] for
// s[1] >= 0. The D[i] are not required to be unit length.
// 
// The closest point on the line is stored in closest[0] with parameter[0]
// storing s[0]. The closest point on the ray is stored in closest[1] with
// parameter[1] storing s[1]. When there are infinitely many choices for the
// pair of closest points, only one of them is returned.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/Ray.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Line<N, T>, Ray<N, T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                parameter{ static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector<N, T>::Zero(), Vector<N, T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 2> parameter;
            std::array<Vector<N, T>, 2> closest;
        };

        Result operator()(Line<N, T> const& line, Ray<N, T> const& ray)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            Vector<N, T> diff = line.origin - ray.origin;
            T a00 = Dot(line.direction, line.direction);
            T a01 = -Dot(line.direction, ray.direction);
            T a11 = Dot(ray.direction, ray.direction);
            T b0 = Dot(line.direction, diff);
            T det = std::max(a00 * a11 - a01 * a01, zero);
            T s0{}, s1{};

            if (det > zero)
            {
                // The line and ray are not parallel.
                T b1 = -Dot(ray.direction, diff);
                s1 = a01 * b0 - a00 * b1;

                if (s1 >= zero)
                {
                    // Two interior points are closest, one on the line and
                    // one on the ray.
                    s0 = (a01 * b1 - a11 * b0) / det;
                    s1 /= det;
                }
                else
                {
                    // The origin of the ray is the closest ray point.
                    s0 = -b0 / a00;
                    s1 = zero;
                }
            }
            else
            {
                // The line and ray are parallel. Select the pair of closest
                // points where the closest ray point is the ray origin.
                s0 = -b0 / a00;
                s1 = zero;
            }

            result.parameter[0] = s0;
            result.parameter[1] = s1;
            result.closest[0] = line.origin + s0 * line.direction;
            result.closest[1] = ray.origin + s1 * ray.direction;
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPLineRay = DCPQuery<T, Line<N, T>, Ray<N, T>>;

    template <typename T>
    using DCPLine2Ray2 = DCPLineRay<2, T>;

    template <typename T>
    using DCPLine3Ray3 = DCPLineRay<3, T>;
}
