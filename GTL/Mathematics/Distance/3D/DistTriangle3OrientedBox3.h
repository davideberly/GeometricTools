// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a solid triangle and a solid oriented box
// in 3D.
// 
// The triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The oriented box has center C, unit-length axis directions U[i] and extents
// e[i] for all i. A box point is X = C + sum_i y[i] * U[i], where
// |y[i]| <= e[i] for all i.
// 
// The closest point on the triangle closest is stored in closest[0] with
// barycentric coordinates (b[0],b[1],b[2). The closest point on the box is
// stored in closest[1]. When there are infinitely many choices for the pair
// of closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/3D/DistTriangle3CanonicalBox3.h>
#include <GTL/Mathematics/Distance/3D/DistSegment3CanonicalBox3.h>
#include <GTL/Mathematics/Primitives/ND/OrientedBox.h>
#include <array>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Triangle3<T>, OrientedBox3<T>>
    {
    public:
        using TBQuery = DCPQuery<T, Triangle3<T>, CanonicalBox3<T>>;
        using Output = typename TBQuery::Output;

        Output operator()(Triangle3<T> const& triangle, OrientedBox3<T> const& box)
        {
            Output output{};

            // Rotate and translate the line and box so that the box is
            // aligned and has center at the origin.
            CanonicalBox3<T> cbox(box.extent);
            Triangle3<T> xfrmTriangle{};
            for (size_t j = 0; j < 3; ++j)
            {
                Vector3<T> delta = triangle.v[j] - box.center;
                for (size_t i = 0; i < 3; ++i)
                {
                    xfrmTriangle.v[j][i] = Dot(box.axis[i], delta);
                }
            }

            // The query computes 'output' relative to the box with center
            // at the origin.
            TBQuery tbQuery{};
            output = tbQuery(xfrmTriangle, cbox);

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
