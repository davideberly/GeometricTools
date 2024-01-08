// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a rectangle and a solid aligned box in 3D.
// 
// The rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
//
// The aligned box has minimum corner A and maximum corner B. A box point is X
// where A <= X <= B; the comparisons are componentwise.
// 
// The closest point on the rectangle is stored in closest[0] with
// W-coordinates (s[0],s[1]). The closest point on the box is stored in
// closest[1]. When there are infinitely many choices for the pair of closest
// points, only one of them is returned.
//
// TODO: Modify to support non-unit-length W[].

#include <Mathematics/DistRectangle3CanonicalBox3.h>
#include <Mathematics/DistSegment3CanonicalBox3.h>
#include <Mathematics/AlignedBox.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Rectangle3<T>, AlignedBox3<T>>
    {
    public:
        using RBQuery = DCPQuery<T, Rectangle3<T>, CanonicalBox3<T>>;
        using Result = typename RBQuery::Result;

        Result operator()(Rectangle3<T> const& rectangle, AlignedBox3<T> const& box)
        {
            Result result{};

            // Translate the rectangle and box so that the box has center at
            // the origin.
            Vector3<T> boxCenter{};
            CanonicalBox3<T> cbox{};
            box.GetCenteredForm(boxCenter, cbox.extent);
            Vector3<T> xfrmCenter = rectangle.center - boxCenter;

            // The query computes 'output' relative to the box with center
            // at the origin.
            Rectangle3<T> xfrmRectangle(xfrmCenter, rectangle.axis, rectangle.extent);
            RBQuery rbQuery{};
            result = rbQuery(xfrmRectangle, cbox);

            // Translate the closest points to the original coordinates.
            result.closest[0] += boxCenter;
            result.closest[1] += boxCenter;

            return result;
        }
    };
}
