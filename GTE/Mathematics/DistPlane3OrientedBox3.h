// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DistPlane3CanonicalBox3.h>
#include <Mathematics/OrientedBox.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Plane3<T>, OrientedBox3<T>>
    {
    public:
        using PCQuery = DCPQuery<T, Plane3<T>, CanonicalBox3<T>>;
        using Result = typename PCQuery::Result;

        Result operator()(Plane3<T> const& plane, OrientedBox3<T> const& box)
        {
            Result result{};

            // Rotate and translate the plane and box so that the box is
            // aligned and has center at the origin.
            CanonicalBox3<T> cbox(box.extent);
            Vector3<T> delta = plane.origin - box.center;
            Vector3<T> xfrmOrigin{}, xfrmNormal{};
            for (int32_t i = 0; i < 3; ++i)
            {
                xfrmOrigin[i] = Dot(box.axis[i], delta);
                xfrmNormal[i] = Dot(box.axis[i], plane.normal);
            }

            // The query computes 'output' relative to the box with center
            // at the origin.
            Plane3<T> xfrmPlane(xfrmNormal, xfrmOrigin);
            PCQuery pcQuery{};
            result = pcQuery(xfrmPlane, cbox);

            // Rotate and translate the closest points to the original
            // coordinates.
            std::array<Vector3<T>, 2> closest{ box.center, box.center };
            for (size_t i = 0; i < 2; ++i)
            {
                for (int32_t j = 0; j < 3; ++j)
                {
                    closest[i] += result.closest[i][j] * box.axis[j];
                }
            }
            result.closest = closest;

            return result;
        }
    };
}
