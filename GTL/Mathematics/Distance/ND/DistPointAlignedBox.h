// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance from a point to a solid aligned box in nD.
// 
// The aligned box has minimum corner A and maximum corner B. A box point is X
// where A <= X <= B; the comparisons are componentwise.
// 
// The input point is stored in closest[0]. The closest point on the box is
// stored in closest[1]. When there are infinitely many choices for the pair
// of closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/ND/DistPointCanonicalBox.h>
#include <GTL/Mathematics/Primitives/ND/AlignedBox.h>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Vector<T, N>, AlignedBox<T, N>>
    {
    public:
        using PCQuery = DCPQuery<T, Vector<T, N>, CanonicalBox<T, N>>;
        using Output = typename PCQuery::Output;
            
        Output operator()(Vector<T, N> const& point, AlignedBox<T, N> const& box)
        {
            Output output{};

            // Translate the point and box so that the box has center at the
            // origin.
            Vector<T, N> boxCenter{};
            CanonicalBox<T, N> cbox{};
            box.GetCenteredForm(boxCenter, cbox.extent);
            Vector<T, N> xfrmPoint = point - boxCenter;

            // The query computes 'output' relative to the box with center
            // at the origin.
            PCQuery pcQuery{};
            output = pcQuery(xfrmPoint, cbox);

            // Store the input point.
            output.closest[0] = point;

            // Translate the closest box point to the original coordinates.
            output.closest[1] += boxCenter;

            return output;
        }
    };
}
