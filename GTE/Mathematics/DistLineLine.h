// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between two lines in nD.
// 
// The lines are P[i] + s[i] * D[i], where D[i] is not required to be unit
// length.
// 
// The closest point on line[i] is stored in closest[i] with parameter[i]
// storing s[i]. When there are infinitely many choices for the pair of
// closest points, only one of them is returned.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Line.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Line<N, T>, Line<N, T>>
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

        Result operator()(Line<N, T> const& line0, Line<N, T> const& line1)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            Vector<N, T> diff = line0.origin - line1.origin;
            T a00 = Dot(line0.direction, line0.direction);
            T a01 = -Dot(line0.direction, line1.direction);
            T a11 = Dot(line1.direction, line1.direction);
            T b0 = Dot(line0.direction, diff);
            T det = std::max(a00 * a11 - a01 * a01, zero);
            T s0{}, s1{};

            if (det > zero)
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
                s1 = zero;
            }

            result.parameter[0] = s0;
            result.parameter[1] = s1;
            result.closest[0] = line0.origin + s0 * line0.direction;
            result.closest[1] = line1.origin + s1 * line1.direction;
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);

            return result;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPLineLine = DCPQuery<T, Line<N, T>, Line<N, T>>;

    template <typename T>
    using DCPLine2Line2 = DCPLineLine<2, T>;

    template <typename T>
    using DCPLine3Line3 = DCPLineLine<3, T>;
}
