// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DistLine3CanonicalBox3.h>
#include <Mathematics/OrientedBox.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line3<T>, OrientedBox3<T>>
    {
    public:
        using LBQuery = DCPQuery<T, Line3<T>, CanonicalBox3<T>>;
        using Result = typename LBQuery::Result;

        Result operator()(Line3<T> const& line, OrientedBox3<T> const& box)
        {
            Result result{};

            // Rotate and translate the line and box so that the box is
            // aligned and has center at the origin.
            CanonicalBox3<T> cbox(box.extent);
            Vector3<T> delta = line.origin - box.center;
            Vector3<T> xfrmOrigin{}, xfrmDirection{};
            for (int32_t i = 0; i < 3; ++i)
            {
                xfrmOrigin[i] = Dot(box.axis[i], delta);
                xfrmDirection[i] = Dot(box.axis[i], line.direction);
            }

            // The query computes 'result' relative to the box with center
            // at the origin.
            Line3<T> xfrmLine(xfrmOrigin, xfrmDirection);
            LBQuery lbQuery{};
            result = lbQuery(xfrmLine, cbox);

            // Compute the closest point on the line.
            result.closest[0] = line.origin + result.parameter * line.direction;

            // Rotate and translate the closest points to the original
            // coordinates.
            std::array<Vector3<T>, 2> closest{ box.center, box.center };
            for (size_t i = 0; i < 2; ++i)
            {
                for (int32_t j = 0; j < 3; ++j)
                {
                    closest[i] += result.closest[i][j] * box.axis[j];
                }
            }
            result.closest = closest;

            return result;
        }
    };
}
