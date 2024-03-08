// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a line and a solid oriented box in 2D.
// 
// The line is P + t * D, where D is not required to be unit length.
// 
// The oriented box has center C, unit-length axis directions U[i] and extents
// e[i] for all i. A box point is X = C + sum_i y[i] * U[i], where
// |y[i]| <= e[i] for all i.
// 
// The closest point on the line is stored in closest[0] with parameter t. The
// closest point on the box is stored in closest[1]. When there are infinitely
// many choices for the pair of closest points, only one of them is returned.

#include <Mathematics/DistLine2AlignedBox2.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line2<T>, OrientedBox2<T>>
    {
    public:
        using AlignedQuery = DCPQuery<T, Line2<T>, AlignedBox2<T>>;
        using Result = typename AlignedQuery::Result;

        Result operator()(Line2<T> const& line, OrientedBox2<T> const& box)
        {
            Result result{};

            // Rotate and translate the line and box so that the box is
            // aligned and has center at the origin.
            Vector2<T> delta = line.origin - box.center;
            Vector2<T> origin{}, direction{};
            for (int32_t i = 0; i < 2; ++i)
            {
                origin[i] = Dot(box.axis[i], delta);
                direction[i] = Dot(box.axis[i], line.direction);
            }

            // The query computes 'result' relative to the box with center
            // at the origin.
            AlignedQuery::DoQuery(origin, direction, box.extent, result);

            // Rotate and translate the closest points to the original
            // coordinates.
            std::array<Vector2<T>, 2> temp{ result.closest[0], result.closest[1] };
            for (size_t i = 0; i < 2; ++i)
            {
                result.closest[i] = box.center + temp[i][0] * box.axis[0]
                    + temp[i][1] * box.axis[1];
            }

            // Compute the distance and squared distance.
            Vector2<T> diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };
}
