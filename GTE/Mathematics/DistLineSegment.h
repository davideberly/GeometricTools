// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a line and a segment in nD.
// 
// The segment is Q[0] + s[0] * (Q[1] - Q[0]) for 0 <= s[0] <= 1. The
// direction D[0] = Q[1] - Q[0] is generally not unit length.
// 
// The line is P[1] + s[1] * D[1], where D[i] is not required to be unit
// length.
// 
// The closest point on the segment is stored in closest[0] with parameter[0]
// storing s[0]. The closest point on the line is stoed in closest[1] with
// parameter[1] storing s[1]. When there are infinitely many choices for the
// pair of closest points, only one of them is returned.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/Segment.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Line<N, T>, Segment<N, T>>
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

        Result operator()(Line<N, T> const& line, Segment<N, T> const& segment)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            Vector<N, T> segDirection = segment.p[1] - segment.p[0];
            Vector<N, T> diff = line.origin - segment.p[0];
            T a00 = Dot(line.direction, line.direction);
            T a01 = -Dot(line.direction, segDirection);
            T a11 = Dot(segDirection, segDirection);
            T b0 = Dot(line.direction, diff);
            T det = std::max(a00 * a11 - a01 * a01, zero);
            T s0{}, s1{};

            if (det > zero)
            {
                // The line and segment are not parallel.
                T b1 = -Dot(segDirection, diff);
                s1 = a01 * b0 - a00 * b1;

                if (s1 >= zero)
                {
                    if (s1 <= det)
                    {
                        // Two interior points are closest, one on the line
                        // and one on the segment.
                        s0 = (a01 * b1 - a11 * b0) / det;
                        s1 /= det;
                    }
                    else
                    {
                        // The endpoint Q1 of the segment and an interior
                        // point of the line are closest.
                        s0 = -(a01 + b0) / a00;
                        s1 = one;
                    }
                }
                else
                {
                    // The endpoint Q0 of the segment and an interior point
                    // of the line are closest.
                    s0 = -b0 / a00;
                    s1 = zero;
                }
            }
            else
            {
                // The line and segment are parallel. Select the pair of
                // closest points where the closest segment point is the
                // endpoint Q0.
                s0 = -b0 / a00;
                s1 = zero;
            }

            result.parameter[0] = s0;
            result.parameter[1] = s1;
            result.closest[0] = line.origin + s0 * line.direction;
            result.closest[1] = segment.p[0] + s1 * segDirection;
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPLineSegment = DCPQuery<T, Line<N, T>, Segment<N, T>>;

    template <typename T>
    using DCPLine2Segment2 = DCPLineSegment<2, T>;

    template <typename T>
    using DCPLine3Segment3 = DCPLineSegment<3, T>;
}
