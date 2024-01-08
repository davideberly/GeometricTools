// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a line and a solid aligned box in 3D.
//
// The line is P + t * D, where D is not required to be unit length.
// 
// The aligned box has minimum corner A and maximum corner B. A box point is X
// where A <= X <= B; the comparisons are componentwise.
// 
// The closest point on the line is stored in closest[0] with parameter t. The
// closest point on the box is stored in closest[1]. When there are infinitely
// many choices for the pair of closest points, only one of them is returned.
//
// The DoQueryND functions are described in Section 10.9.4 Linear Component
// to Oriented Bounding Box of
//    Geometric Tools for Computer Graphics,
//    Philip J. Schneider and David H. Eberly,
//    Morgan Kaufmnn, San Francisco CA, 2002

#include <Mathematics/DistLine3CanonicalBox3.h>
#include <Mathematics/AlignedBox.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line3<T>, AlignedBox3<T>>
    {
    public:
        using LBQuery = DCPQuery<T, Line3<T>, CanonicalBox3<T>>;
        using Result = typename LBQuery::Result;

        Result operator()(Line3<T> const& line, AlignedBox3<T> const& box)
        {
            Result result{};

            // Translate the line and box so that the box has center at the
            // origin.
            Vector3<T> boxCenter{};
            CanonicalBox3<T> cbox{};
            box.GetCenteredForm(boxCenter, cbox.extent);
            Vector3<T> xfrmOrigin = line.origin - boxCenter;

            // The query computes 'output' relative to the box with center
            // at the origin.
            Line3<T> xfrmLine(xfrmOrigin, line.direction);
            LBQuery lbQuery{};
            result = lbQuery(xfrmLine, cbox);

            // Compute the closest point on the line.
            result.closest[0] = line.origin + result.parameter * line.direction;

            // Translate the closest box point to the original coordinates.
            result.closest[1] += boxCenter;
            return result;
        }
    };
}
