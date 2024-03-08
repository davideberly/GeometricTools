// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a ray and a segment in nD.
// 
// The ray is P[0] + s[0] * D[0] for s[0] >= 0. D[0] is not required to be
// unit length. 
// 
// The segment is Q[0] + s[1] * (Q[1] - Q[0]) for 0 <= s[1 <= 1. The
// direction D = Q[1] - Q[0] is generally not unit length.
// 
// The closest point on the ray is stored in closest[0] with parameter[0]
// storing s[0]. The closest point on the segment is stored in closest[1] with
// parameter[1] storing s[1]. When there are infinitely many choices for the
// pair of closest points, only one of them is returned.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Ray.h>
#include <Mathematics/Segment.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Ray<N, T>, Segment<N, T>>
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

        Result operator()(Ray<N, T> const& ray, Segment<N, T> const& segment)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            Vector<N, T> segDirection = segment.p[1] - segment.p[0];
            Vector<N, T> diff = ray.origin - segment.p[0];
            T a00 = Dot(ray.direction, ray.direction);
            T a01 = -Dot(ray.direction, segDirection);
            T a11 = Dot(segDirection, segDirection);
            T b0 = Dot(ray.direction, diff);
            T det = std::max(a00 * a11 - a01 * a01, zero);
            T s0{}, s1{};

            if (det > zero)
            {
                // The ray and segment are not parallel.
                T b1 = -Dot(segDirection, diff);
                s0 = a01 * b1 - a11 * b0;
                s1 = a01 * b0 - a00 * b1;

                if (s0 >= zero)
                {
                    if (s1 >= zero)
                    {
                        if (s1 <= det)  // region 0
                        {
                            // The minimum occurs at interior points of the
                            // ray and the segment.
                            s0 /= det;
                            s1 /= det;
                        }
                        else  // region 1
                        {
                            // The endpoint Q1 of the segment and an interior
                            // point of the line are closest.
                            s0 = -(a01 + b0) / a00;
                            s1 = one;
                        }
                    }
                    else  // region 5
                    {
                        // The endpoint Q0 of the segment and an interior
                        // point of the line are closest.
                        s0 = -b0 / a00;
                        s1 = zero;
                    }
                }
                else  // s0 < 0
                {
                    if (s1 <= zero)  // region 4
                    {
                        s0 = -b0;
                        if (s0 > zero)
                        {
                            s0 /= a00;
                            s1 = zero;
                        }
                        else
                        {
                            s0 = zero;
                            s1 = -b1;
                            if (s1 < zero)
                            {
                                s1 = zero;
                            }
                            else if (s1 > a11)
                            {
                                s1 = one;
                            }
                            else
                            {
                                s1 /= a11;
                            }
                        }
                    }
                    else if (s1 <= det)  // region 3
                    {
                        s0 = zero;
                        s1 = -b1;
                        if (s1 < zero)
                        {
                            s1 = zero;
                        }
                        else if (s1 > a11)
                        {
                            s1 = one;
                        }
                        else
                        {
                            s1 /= a11;
                        }
                    }
                    else  // region 2
                    {
                        s0 = -(a01 + b0);
                        if (s0 > zero)
                        {
                            s0 /= a00;
                            s1 = one;
                        }
                        else
                        {
                            s0 = zero;
                            s1 = -b1;
                            if (s1 < zero)
                            {
                                s1 = zero;
                            }
                            else if (s1 > a11)
                            {
                                s1 = one;
                            }
                            else
                            {
                                s1 /= a11;
                            }
                        }
                    }
                }
            }
            else
            {
                // The ray and segment are parallel.
                if (a01 > zero)
                {
                    // Opposite direction vectors.
                    s0 = -b0 / a00;
                    s1 = zero;
                }
                else
                {
                    // Same direction vectors.
                    s0 = -(a01 + b0) / a00;
                    s1 = one;
                }
            }

            result.parameter[0] = s0;
            result.parameter[1] = s1;
            result.closest[0] = ray.origin + s0 * ray.direction;
            result.closest[1] = segment.p[0] + s1 * segDirection;
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPRaySegment = DCPQuery<T, Ray<N, T>, Segment<N, T>>;

    template <typename T>
    using DCPRay2Segment2 = DCPRaySegment<2, T>;

    template <typename T>
    using DCPRay3Segment3 = DCPRaySegment<3, T>;
}
