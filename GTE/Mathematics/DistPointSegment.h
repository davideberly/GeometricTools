// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a point and a segment in nD.
// 
// The segment is P0 + t * (P1 - P0) for 0 <= t <= 1. The direction D = P1-P0
// is generally not unit length.
// 
// The input point is stored in closest[0]. The closest point on the segment
// is stored in closest[1]. When there are infinitely many choices for the
// pair of closest points, only one of them is returned.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Segment.h>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Vector<N, T>, Segment<N, T>>
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

        Result operator()(Vector<N, T> const& point, Segment<N, T> const& segment)
        {
            Result result{};

            // The direction vector is not unit length. The normalization is
            // deferred until it is needed.
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            Vector<N, T> direction = segment.p[1] - segment.p[0];
            Vector<N, T> diff = point - segment.p[1];
            T t = Dot(direction, diff);
            if (t >= zero)
            {
                result.parameter = one;
                result.closest[1] = segment.p[1];
            }
            else
            {
                diff = point - segment.p[0];
                t = Dot(direction, diff);
                if (t <= zero)
                {
                    result.parameter = zero;
                    result.closest[1] = segment.p[0];
                }
                else
                {
                    T sqrLength = Dot(direction, direction);
                    if (sqrLength > zero)
                    {
                        t /= sqrLength;
                        result.parameter = t;
                        result.closest[1] = segment.p[0] + t * direction;
                    }
                    else
                    {
                        result.parameter = zero;
                        result.closest[1] = segment.p[0];
                    }
                }
            }
            result.closest[0] = point;

            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);

            return result;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPPointSegment = DCPQuery<T, Vector<N, T>, Segment<N, T>>;

    template <typename T>
    using DCPPoint2Segment2 = DCPPointSegment<2, T>;

    template <typename T>
    using DCPPoint3Segment3 = DCPPointSegment<3, T>;
}
