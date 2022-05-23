// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a plane and a solid aligned box in 3D.
// 
// The plane is defined by Dot(N, X - P) = 0, where P is the plane origin and
// N is a unit-length normal for the plane.
// 
// The aligned box has minimum corner A and maximum corner B. A box point is X
// where A <= X <= B; the comparisons are componentwise.
// 
// The closest point on the plane is stored in closest[0]. The closest point
// on the box is stored in closest[1]. When there are infinitely many choices
// for the pair of closest points, only one of them is returned.
//
// TODO: Modify to support non-unit-length N.

#include <GTL/Mathematics/Distance/3D/DistPlane3CanonicalBox3.h>
#include <GTL/Mathematics/Primitives/ND/AlignedBox.h>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Plane3<T>, AlignedBox3<T>>
    {
    public:
        using PCQuery = DCPQuery<T, Plane3<T>, CanonicalBox3<T>>;
        using Output = typename PCQuery::Output;

        Output operator()(Plane3<T> const& plane, AlignedBox3<T> const& box)
        {
            Output output{};

            // Translate the plane and box so that the box has center at the
            // origin.
            Vector3<T> boxCenter{};
            CanonicalBox3<T> cbox{};
            box.GetCenteredForm(boxCenter, cbox.extent);
            Vector3<T> xfrmOrigin = plane.origin - boxCenter;

            // The query computes 'output' relative to the box with center
            // at the origin.
            Plane3<T> xfrmPlane(plane.normal, xfrmOrigin);
            PCQuery pcQuery{};
            output = pcQuery(xfrmPlane, cbox);

            // Translate the closest points to the original coordinates.
            for (size_t i = 0; i < 2; ++i)
            {
                output.closest[i] += boxCenter;
            }

            return output;
        }
    };
}
