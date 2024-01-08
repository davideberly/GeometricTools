// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance from a point to a solid oriented box in nD.
// 
// The oriented box has center C, unit-length axis directions U[i] and extents
// e[i] for all i. A box point is X = C + sum_i y[i] * U[i], where
// |y[i]| <= e[i] for all i.
// 
// The input point is stored in closest[0]. The closest point on the box
// point is stored in closest[1].

#include <Mathematics/DistPointCanonicalBox.h>
#include <Mathematics/OrientedBox.h>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Vector<N, T>, OrientedBox<N, T>>
    {
    public:
        using PCQuery = DCPQuery<T, Vector<N, T>, CanonicalBox<N, T>>;
        using Result = typename PCQuery::Result;

        Result operator()(Vector<N, T> const& point, OrientedBox<N, T> const& box)
        {
            Result result{};

            // Rotate and translate the point and box so that the box is
            // aligned and has center at the origin.
            CanonicalBox<N, T> cbox(box.extent);
            Vector<N, T> delta = point - box.center;
            Vector<N, T> xfrmPoint{};
            for (int32_t i = 0; i < N; ++i)
            {
                xfrmPoint[i] = Dot(box.axis[i], delta);
            }

            // The query computes 'result' relative to the box with center
            // at the origin.
            PCQuery pcQuery{};
            result = pcQuery(xfrmPoint, cbox);

            // Store the input point.
            result.closest[0] = point;

            // Rotate and translate the closest box point to the original
            // coordinates.
            Vector<N, T> closest1 = box.center;
            for (int32_t i = 0; i < N; ++i)
            {
                closest1 += result.closest[1][i] * box.axis[i];
            }
            result.closest[1] = closest1;

            return result;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPPointOrientedBox = DCPQuery<T, Vector<N, T>, OrientedBox<N, T>>;

    template <typename T>
    using DCPPoint2OrientedBox2 = DCPPointOrientedBox<2, T>;

    template <typename T>
    using DCPPoint3OrientedBox3 = DCPPointOrientedBox<3, T>;
}
