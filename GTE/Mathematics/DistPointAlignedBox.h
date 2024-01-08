// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance from a point to a solid aligned box in nD.
// 
// The aligned box has minimum corner A and maximum corner B. A box point is X
// where A <= X <= B; the comparisons are componentwise.
// 
// The input point is stored in closest[0]. The closest point on the box is
// stored in closest[1]. When there are infinitely many choices for the pair
// of closest points, only one of them is returned.

#include <Mathematics/DistPointCanonicalBox.h>
#include <Mathematics/AlignedBox.h>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Vector<N, T>, AlignedBox<N, T>>
    {
    public:
        using PCQuery = DCPQuery<T, Vector<N, T>, CanonicalBox<N, T>>;
        using Result = typename PCQuery::Result;

        Result operator()(Vector<N, T> const& point, AlignedBox<N, T> const& box)
        {
            Result result{};

            // Translate the point and box so that the box has center at the
            // origin.
            Vector<N, T> boxCenter{};
            CanonicalBox<N, T> cbox{};
            box.GetCenteredForm(boxCenter, cbox.extent);
            Vector<N, T> xfrmPoint = point - boxCenter;

            // The query computes 'output' relative to the box with center
            // at the origin.
            PCQuery pcQuery{};
            result = pcQuery(xfrmPoint, cbox);

            // Store the input point.
            result.closest[0] = point;

            // Translate the closest box point to the original coordinates.
            result.closest[1] += boxCenter;

            return result;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPPointAlignedBox = DCPQuery<T, Vector<N, T>, AlignedBox<N, T>>;

    template <typename T>
    using DCPPoint2AlignedBox2 = DCPPointAlignedBox<2, T>;

    template <typename T>
    using DCPPoint3AlignedBox3 = DCPPointAlignedBox<3, T>;
}
