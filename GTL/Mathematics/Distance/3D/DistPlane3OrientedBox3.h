// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a plane and a solid oriented box in 3D.
// 
// The plane is defined by Dot(N, X - P) = 0, where P is the plane origin and
// N is a unit-length normal for the plane.
// 
// The oriented box has center C, unit-length axis directions U[i] and extents
// e[i] for all i. A box point is X = C + sum_i y[i] * U[i], where
// |y[i]| <= e[i] for all i.
// 
// The closest point on the plane is stored in closest[0]. The closest point
// on the box is stored in closest[1]. When there are infinitely many choices
// for the pair of closest points, only one of them is returned.
//
// TODO: Modify to support non-unit-length N and non-unit-length U[].

#include <GTL/Mathematics/Distance/3D/DistPlane3AlignedBox3.h>
#include <GTL/Mathematics/Primitives/ND/OrientedBox.h>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Plane3<T>, OrientedBox3<T>>
    {
    public:
        using PCQuery = DCPQuery<T, Plane3<T>, CanonicalBox3<T>>;
        using Output = typename PCQuery::Output;

        Output operator()(Plane3<T> const& plane, OrientedBox3<T> const& box)
        {
            Output output{};

            // Rotate and translate the plane and box so that the box is
            // aligned and has center at the origin.
            CanonicalBox3<T> cbox(box.extent);
            Vector3<T> delta = plane.origin - box.center;
            Vector3<T> xfrmOrigin{}, xfrmNormal{};
            for (size_t i = 0; i < 3; ++i)
            {
                xfrmOrigin[i] = Dot(box.axis[i], delta);
                xfrmNormal[i] = Dot(box.axis[i], plane.normal);
            }

            // The query computes 'output' relative to the box with center
            // at the origin.
            Plane3<T> xfrmPlane(xfrmNormal, xfrmOrigin);
            PCQuery pcQuery{};
            output = pcQuery(xfrmPlane, cbox);

            // Rotate and translate the closest points to the original
            // coordinates.
            std::array<Vector3<T>, 2> closest{ box.center, box.center };
            for (size_t i = 0; i < 2; ++i)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    closest[i] += output.closest[i][j] * box.axis[j];
                }
            }
            output.closest = closest;

            return output;
        }
    };
}
