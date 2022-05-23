// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/3D/DistOrientedBox3OrientedBox3.h>
#include <GTL/Mathematics/Primitives/ND/AlignedBox.h>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, AlignedBox3<T>, OrientedBox3<T>>
    {
    public:
        using BBQuery = DCPQuery<T, OrientedBox3<T>, OrientedBox3<T>>;
        using Output = typename BBQuery::Output;

        Output operator()(AlignedBox3<T> const& box0, OrientedBox3<T> const& box1)
        {
            Output output{};

            // Convert the aligned box to an oriented box.
            OrientedBox3<T> obox0{};
            obox0.center = C_<T>(1, 2) * (box0.max + box0.min);
            obox0.extent = C_<T>(1, 2) * (box0.max - box0.min);
            obox0.axis[0] = { C_<T>(1), C_<T>(0), C_<T>(0) };
            obox0.axis[1] = { C_<T>(0), C_<T>(1), C_<T>(0) };
            obox0.axis[2] = { C_<T>(0), C_<T>(0), C_<T>(1) };

            // Execute the query for two oriented boxes.
            BBQuery bbQuery{};
            output = bbQuery(obox0, box1);
            return output;
        }
    };
}
