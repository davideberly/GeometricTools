// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between solid aligned and oriented boxes in 3D.
// 
// The aligned box has minimum corner A and maximum corner B. A box point is X
// where A <= X <= B; the comparisons are componentwise.
// 
// The oriented box has center C, unit-length axis directions U[i] and extents
// e[i] for all i. A box point is X = C + sum_i y[i] * U[i], where
// |y[i]| <= e[i] for all i.
//
// The closest point of the aligned box is stored in closest[0]. The closest
// point of the oriented box is stored in closest[1].

#include <Mathematics/DistOrientedBox3OrientedBox3.h>
#include <Mathematics/AlignedBox.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, AlignedBox3<T>, OrientedBox3<T>>
    {
    public:
        using BBQuery = DCPQuery<T, OrientedBox3<T>, OrientedBox3<T>>;
        using Result = typename BBQuery::Result;

        Result operator()(AlignedBox3<T> const& box0, OrientedBox3<T> const& box1)
        {
            Result result{};

            // Convert the aligned box to an oriented box.
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const half = static_cast<T>(0.5);
            OrientedBox3<T> obox0{};
            obox0.center = half * (box0.max + box0.min);
            obox0.extent = half * (box0.max - box0.min);
            obox0.axis[0] = { one, zero, zero };
            obox0.axis[1] = { zero, one, zero };
            obox0.axis[2] = { zero, zero, one };

            // Execute the query for two oriented boxes.
            BBQuery bbQuery{};
            result = bbQuery(obox0, box1);
            return result;
        }
    };
}
