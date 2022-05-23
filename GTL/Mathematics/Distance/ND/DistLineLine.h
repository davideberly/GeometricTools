// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between two lines in nD.
// 
// The lines are P[i] + s[i] * D[i], where D[i] is not required to be unit
// length.
// 
// The closest point on line[i] is stored in closest[i] with parameter[i]
// storing s[i]. When there are infinitely many choices for the pair of
// closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Line.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Line<T, N>, Line<T, N>>
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

        Output operator()(Line<T, N> const& line0, Line<T, N> const& line1)
        {
            Output output{};

            Vector<T, N> diff = line0.origin - line1.origin;
            T a00 = Dot(line0.direction, line0.direction);
            T a01 = -Dot(line0.direction, line1.direction);
            T a11 = Dot(line1.direction, line1.direction);
            T b0 = Dot(line0.direction, diff);
            T det = std::max(a00 * a11 - a01 * a01, C_<T>(0));
            T s0{}, s1{};

            if (det > C_<T>(0))
            {
                // The lines are not parallel.
                T b1 = -Dot(line1.direction, diff);
                s0 = (a01 * b1 - a11 * b0) / det;
                s1 = (a01 * b0 - a00 * b1) / det;
            }
            else
            {
                // The lines are parallel. Select any pair of closest points.
                s0 = -b0 / a00;
                s1 = C_<T>(0);
            }

            output.parameter[0] = s0;
            output.parameter[1] = s1;
            output.closest[0] = line0.origin + s0 * line0.direction;
            output.closest[1] = line1.origin + s1 * line1.direction;
            diff = output.closest[0] - output.closest[1];
            output.sqrDistance = Dot(diff, diff);
            output.distance = std::sqrt(output.sqrDistance);

            return output;
        }
    };
}
