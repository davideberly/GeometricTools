// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a rectangle and a solid oriented box in 3D.
//
// The rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
//
// The oriented box has center C, unit-length axis directions U[i] and extents
// e[i] for all i. A box point is X = C + sum_i y[i] * U[i], where
// |y[i]| <= e[i] for all i.
// 
// The closest point on the rectangle is stored in closest[0] with
// W-coordinates (s[0],s[1]). The closest point on the box is stored in
// closest[1]. When there are infinitely many choices for the pair of closest
// points, only one of them is returned.
//
// TODO: Modify to support non-unit-length W[].

#include <Mathematics/DistRectangle3CanonicalBox3.h>
#include <Mathematics/OrientedBox.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Rectangle3<T>, OrientedBox3<T>>
    {
    public:
        using RBQuery = DCPQuery<T, Rectangle3<T>, CanonicalBox3<T>>;
        using Result = typename RBQuery::Result;

        Result operator()(Rectangle3<T> const& rectangle, OrientedBox3<T> const& box)
        {
            Result result{};

            // Rotate and translate the rectangle and box so that the box is
            // aligned and has center at the origin.
            CanonicalBox3<T> cbox(box.extent);
            Vector3<T> delta = rectangle.center - box.center;
            Vector3<T> xfrmCenter{};
            std::array<Vector3<T>, 2> xfrmAxis{};
            for (int32_t i = 0; i < 3; ++i)
            {
                xfrmCenter[i] = Dot(box.axis[i], delta);
                for (size_t j = 0; j < 2; ++j)
                {
                    xfrmAxis[j][i] = Dot(box.axis[i], rectangle.axis[j]);
                }
            }

            // The query computes 'output' relative to the box with center
            // at the origin.
            Rectangle3<T> xfrmRectangle(xfrmCenter, xfrmAxis, rectangle.extent);
            RBQuery rbQuery{};
            result = rbQuery(xfrmRectangle, cbox);

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
