// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between two rays in nD.
//
// The rays are P[i] + s[i] * D[i] for s[i] >= 0, where D[i] is not required
// to be unit length.
// 
// The closest point on ray[i] is stored in closest[i] with parameter[i]
// storing s[i]. When there are infinitely many choices for the pair of
// closest points, only one of them is returned.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Ray.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Ray<N, T>, Ray<N, T>>
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

        Result operator()(Ray<N, T> const& ray0, Ray<N, T> const& ray1)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            Vector<N, T> diff = ray0.origin - ray1.origin;
            T a00 = Dot(ray0.direction, ray0.direction);
            T a01 = -Dot(ray0.direction, ray1.direction);
            T a11 = Dot(ray1.direction, ray1.direction);
            T b0 = Dot(ray0.direction, diff);
            T det = std::max(a00 * a11 - a01 * a01, zero);
            T s0{}, s1{};

            if (det > zero)
            {
                // The rays are not parallel.
                T b1 = -Dot(ray1.direction, diff);
                s0 = a01 * b1 - a11 * b0;
                s1 = a01 * b0 - a00 * b1;

                if (s0 >= zero)
                {
                    if (s1 >= zero)  // region 0 (interior)
                    {
                        // The minimum occurs at two interior points of
                        // the rays.
                        s0 /= det;
                        s1 /= det;
                    }
                    else  // region 3 (side)
                    {
                        if (b0 >= zero)
                        {
                            s0 = zero;
                        }
                        else
                        {
                            s0 = -b0 / a00;
                        }
                        s1 = zero;
                    }
                }
                else
                {
                    if (s1 >= zero)  // region 1 (side)
                    {
                        s0 = zero;
                        if (b1 >= zero)
                        {
                            s1 = zero;
                        }
                        else
                        {
                            s1 = -b1 / a11;
                        }
                    }
                    else  // region 2 (corner)
                    {
                        if (b0 < zero)
                        {
                            s0 = -b0 / a00;
                            s1 = zero;
                        }
                        else
                        {
                            s0 = zero;
                            if (b1 >= zero)
                            {
                                s1 = zero;
                            }
                            else
                            {
                                s1 = -b1 / a11;
                            }
                        }
                    }
                }
            }
            else
            {
                // The rays are parallel.
                if (a01 > zero)
                {
                    // Opposite direction vectors.
                    s1 = zero;
                    if (b0 >= zero)
                    {
                        s0 = zero;
                    }
                    else
                    {
                        s0 = -b0 / a00;
                    }
                }
                else
                {
                    // Same direction vectors.
                    if (b0 >= zero)
                    {
                        T b1 = -Dot(ray1.direction, diff);
                        s0 = zero;
                        s1 = -b1 / a11;
                    }
                    else
                    {
                        s0 = -b0 / a00;
                        s1 = zero;
                    }
                }
            }

            result.parameter[0] = s0;
            result.parameter[1] = s1;
            result.closest[0] = ray0.origin + s0 * ray0.direction;
            result.closest[1] = ray1.origin + s1 * ray1.direction;
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);

            return result;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPRayRay = DCPQuery<T, Ray<N, T>, Ray<N, T>>;

    template <typename T>
    using DCPRay2Ray2 = DCPRayRay<2, T>;

    template <typename T>
    using DCPRay3Ray3 = DCPRayRay<3, T>;
}
