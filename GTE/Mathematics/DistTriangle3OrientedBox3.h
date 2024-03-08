// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DistTriangle3CanonicalBox3.h>
#include <Mathematics/DistSegment3CanonicalBox3.h>
#include <Mathematics/OrientedBox.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Triangle3<T>, OrientedBox3<T>>
    {
    public:
        using TBQuery = DCPQuery<T, Triangle3<T>, CanonicalBox3<T>>;
        using Result = typename TBQuery::Result;

        Result operator()(Triangle3<T> const& triangle, OrientedBox3<T> const& box)
        {
            Result result{};

            // Rotate and translate the line and box so that the box is
            // aligned and has center at the origin.
            CanonicalBox3<T> cbox(box.extent);
            Triangle3<T> xfrmTriangle{};
            for (size_t j = 0; j < 3; ++j)
            {
                Vector3<T> delta = triangle.v[j] - box.center;
                for (int32_t i = 0; i < 3; ++i)
                {
                    xfrmTriangle.v[j][i] = Dot(box.axis[i], delta);
                }
            }

            // The query computes 'result' relative to the box with center
            // at the origin.
            TBQuery tbQuery{};
            result = tbQuery(xfrmTriangle, cbox);

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
