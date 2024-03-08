// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DistPlane3CanonicalBox3.h>
#include <Mathematics/AlignedBox.h>
#include <cstddef>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Plane3<T>, AlignedBox3<T>>
    {
    public:
        using PCQuery = DCPQuery<T, Plane3<T>, CanonicalBox3<T>>;
        using Result = typename PCQuery::Result;

        Result operator()(Plane3<T> const& plane, AlignedBox3<T> const& box)
        {
            Result result{};

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
            result = pcQuery(xfrmPlane, cbox);

            // Translate the closest points to the original coordinates.
            for (size_t i = 0; i < 2; ++i)
            {
                result.closest[i] += boxCenter;
            }

            return result;
        }
    };
}
