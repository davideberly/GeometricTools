// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/3D/DistRectangle3CanonicalBox3.h>
#include <GTL/Mathematics/Distance/3D/DistSegment3CanonicalBox3.h>
#include <GTL/Mathematics/Primitives/ND/AlignedBox.h>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Rectangle3<T>, AlignedBox3<T>>
    {
    public:
        using RBQuery = DCPQuery<T, Rectangle3<T>, CanonicalBox3<T>>;
        using Output = typename RBQuery::Output;

        Output operator()(Rectangle3<T> const& rectangle, AlignedBox3<T> const& box)
        {
            Output output{};

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
            output = rbQuery(xfrmRectangle, cbox);

            // Translate the closest points to the original coordinates.
            output.closest[0] += boxCenter;
            output.closest[1] += boxCenter;

            return output;
        }
    };
}
