// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a line and a solid oriented box in 3D.
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

#include <GTL/Mathematics/Distance/3D/DistLine3CanonicalBox3.h>
#include <GTL/Mathematics/Primitives/ND/OrientedBox.h>
#include <array>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Line3<T>, OrientedBox3<T>>
    {
    public:
        using LBQuery = DCPQuery<T, Line3<T>, CanonicalBox3<T>>;
        using Output = typename LBQuery::Output;

        Output operator()(Line3<T> const& line, OrientedBox3<T> const& box)
        {
            Output output{};

            // Rotate and translate the line and box so that the box is
            // aligned and has center at the origin.
            CanonicalBox3<T> cbox(box.extent);
            Vector3<T> delta = line.origin - box.center;
            Vector3<T> xfrmOrigin{}, xfrmDirection{};
            for (size_t i = 0; i < 3; ++i)
            {
                xfrmOrigin[i] = Dot(box.axis[i], delta);
                xfrmDirection[i] = Dot(box.axis[i], line.direction);
            }

            // The query computes 'output' relative to the box with center
            // at the origin.
            Line3<T> xfrmLine(xfrmOrigin, xfrmDirection);
            LBQuery lbQuery{};
            output = lbQuery(xfrmLine, cbox);

            // Compute the closest point on the line.
            output.closest[0] = line.origin + output.parameter * line.direction;

            // Rotate and translate the closest points to the original
            // coordinates.
            std::array<Vector3<T>, 2> closest{ box.center, box.center };
            for (size_t i = 0; i < 2; ++i)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    closest[i] += output.closest[i][j] * box.axis[j];
                }
            }
            output.closest = closest;

            return output;
        }
    };
}
