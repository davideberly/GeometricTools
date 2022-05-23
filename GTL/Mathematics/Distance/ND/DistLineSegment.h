// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Line.h>
#include <GTL/Mathematics/Primitives/ND/Segment.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Line<T, N>, Segment<T, N>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                parameter{ C_<T>(0), C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<T, 2> parameter;
            std::array<Vector<T, N>, 2> closest;
        };

        Output operator()(Line<T, N> const& line, Segment<T, N> const& segment)
        {
            Output output{};

            Vector<T, N> segDirection = segment.p[1] - segment.p[0];
            Vector<T, N> diff = line.origin - segment.p[0];
            T a00 = Dot(line.direction, line.direction);
            T a01 = -Dot(line.direction, segDirection);
            T a11 = Dot(segDirection, segDirection);
            T b0 = Dot(line.direction, diff);
            T det = std::max(a00 * a11 - a01 * a01, C_<T>(0));
            T s0{}, s1{};

            if (det > C_<T>(0))
            {
                // The line and segment are not parallel.
                T b1 = -Dot(segDirection, diff);
                s1 = a01 * b0 - a00 * b1;

                if (s1 >= C_<T>(0))
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
                        s1 = C_<T>(1);
                    }
                }
                else
                {
                    // The endpoint Q0 of the segment and an interior point
                    // of the line are closest.
                    s0 = -b0 / a00;
                    s1 = C_<T>(0);
                }
            }
            else
            {
                // The line and segment are parallel. Select the pair of
                // closest points where the closest segment point is the
                // endpoint Q0.
                s0 = -b0 / a00;
                s1 = C_<T>(0);
            }

            output.parameter[0] = s0;
            output.parameter[1] = s1;
            output.closest[0] = line.origin + s0 * line.direction;
            output.closest[1] = segment.p[0] + s1 * segDirection;
            diff = output.closest[0] - output.closest[1];
            output.sqrDistance = Dot(diff, diff);
            output.distance = std::sqrt(output.sqrDistance);
            return output;
        }
    };
}
