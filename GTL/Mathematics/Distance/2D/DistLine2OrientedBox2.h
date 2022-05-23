// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/2D/DistLine2AlignedBox2.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Line2<T>, OrientedBox2<T>>
    {
    public:
        // The comments in DistLine2AlignedBox2.h describe the members of
        // Output.
        using AlignedQuery = DCPQuery<T, Line2<T>, AlignedBox2<T>>;
        using Output = typename AlignedQuery::Output;

        Output operator()(Line2<T> const& line, OrientedBox2<T> const& box)
        {
            Output output{};

            // Rotate and translate the line and box so that the box is
            // aligned and has center at the origin.
            Vector2<T> delta = line.origin - box.center;
            Vector2<T> origin{}, direction{};
            for (int32_t i = 0; i < 2; ++i)
            {
                origin[i] = Dot(box.axis[i], delta);
                direction[i] = Dot(box.axis[i], line.direction);
            }

            // The query computes 'output' relative to the box with center
            // at the origin.
            AlignedQuery::DoQuery(origin, direction, box.extent, output);

            // Rotate and translate the closest points to the original
            // coordinates.
            std::array<Vector2<T>, 2> temp{ output.closest[0], output.closest[1] };
            for (size_t i = 0; i < 2; ++i)
            {
                output.closest[i] = box.center + temp[i][0] * box.axis[0]
                    + temp[i][1] * box.axis[1];
            }

            // Compute the distance and squared distance.
            Vector2<T> diff = output.closest[0] - output.closest[1];
            output.sqrDistance = Dot(diff, diff);
            output.distance = std::sqrt(output.sqrDistance);
            return output;
        }
    };
}
