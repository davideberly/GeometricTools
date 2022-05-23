// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a point and a segment in nD.
// 
// The segment is P0 + t * (P1 - P0) for 0 <= t <= 1. The direction D = P1-P0
// is generally not unit length.
// 
// The input point is stored in closest[0]. The closest point on the segment
// is stored in closest[1]. When there are infinitely many choices for the
// pair of closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Segment.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Vector<T, N>, Segment<T, N>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                parameter(C_<T>(0)),
                closest{}
            {
            }

            T distance, sqrDistance;
            T parameter;
            std::array<Vector<T, N>, 2> closest;
        };

        Output operator()(Vector<T, N> const& point, Segment<T, N> const& segment)
        {
            Output output{};

            // The direction vector is not unit length. The normalization is
            // deferred until it is needed.
            Vector<T, N> direction = segment.p[1] - segment.p[0];
            Vector<T, N> diff = point - segment.p[1];
            T t = Dot(direction, diff);
            if (t >= C_<T>(0))
            {
                output.parameter = C_<T>(1);
                output.closest[1] = segment.p[1];
            }
            else
            {
                diff = point - segment.p[0];
                t = Dot(direction, diff);
                if (t <= C_<T>(0))
                {
                    output.parameter = C_<T>(0);
                    output.closest[1] = segment.p[0];
                }
                else
                {
                    T sqrLength = Dot(direction, direction);
                    if (sqrLength > C_<T>(0))
                    {
                        t /= sqrLength;
                        output.parameter = t;
                        output.closest[1] = segment.p[0] + t * direction;
                    }
                    else
                    {
                        output.parameter = C_<T>(0);
                        output.closest[1] = segment.p[0];
                    }
                }
            }
            output.closest[0] = point;

            diff = output.closest[0] - output.closest[1];
            output.sqrDistance = Dot(diff, diff);
            output.distance = std::sqrt(output.sqrDistance);

            return output;
        }
    };
}
