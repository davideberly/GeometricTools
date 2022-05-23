// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance from a point to a solid oriented box in nD.
// 
// The oriented box has center C, unit-length axis directions U[i] and extents
// e[i] for all i. A box point is X = C + sum_i y[i] * U[i], where
// |y[i]| <= e[i] for all i.
// 
// The input point is stored in closest[0]. The closest point on the box
// point is stored in closest[1].

#include <GTL/Mathematics/Distance/ND/DistPointCanonicalBox.h>
#include <GTL/Mathematics/Primitives/ND/OrientedBox.h>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Vector<T, N>, OrientedBox<T, N>>
    {
    public:
        using PCQuery = DCPQuery<T, Vector<T, N>, CanonicalBox<T, N>>;
        using Output = typename PCQuery::Output;

        Output operator()(Vector<T, N> const& point, OrientedBox<T, N> const& box)
        {
            Output output{};

            // Rotate and translate the point and box so that the box is
            // aligned and has center at the origin.
            CanonicalBox<T, N> cbox(box.extent);
            Vector<T, N> delta = point - box.center;
            Vector<T, N> xfrmPoint{};
            for (size_t i = 0; i < N; ++i)
            {
                xfrmPoint[i] = Dot(box.axis[i], delta);
            }

            // The query computes 'result' relative to the box with center
            // at the origin.
            PCQuery pcQuery{};
            output = pcQuery(xfrmPoint, cbox);

            // Store the input point.
            output.closest[0] = point;

            // Rotate and translate the closest box point to the original
            // coordinates.
            Vector<T, N> closest1 = box.center;
            for (size_t i = 0; i < N; ++i)
            {
                closest1 += output.closest[1][i] * box.axis[i];
            }
            output.closest[1] = closest1;

            return output;
        }
    };
}
