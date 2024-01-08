// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.27

#pragma once

// Compute the distance between a point and a line in nD.
// 
// The line is P + t * D, where D is not required to be unit length.
// 
// The input point is stored in closest[0]. The closest point on the line is
// stored in closest[1].

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Line.h>
#include <array>
#include <cmath>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Vector<N, T>, Line<N, T>>
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

        Result operator()(Vector<N, T> const& point, Line<N, T> const& line)
        {
            Result result{};

            Vector<N, T> diff = point - line.origin;
            result.parameter = Dot(line.direction, diff) / Dot(line.direction, line.direction);
            result.closest[0] = point;
            result.closest[1] = line.origin + result.parameter * line.direction;
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);

            return result;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPPointLine = DCPQuery<T, Vector<N, T>, Line<N, T>>;

    template <typename T>
    using DCPPoint2Line2 = DCPPointLine<2, T>;

    template <typename T>
    using DCPPoint3Line3 = DCPPointLine<3, T>;
}
