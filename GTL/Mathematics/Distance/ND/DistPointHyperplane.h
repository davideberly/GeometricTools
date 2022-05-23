// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a point and a line (N = 2), between a point
// and a plane (N = 3) or generally between a point and a hyperplane (N >= 2).
// 
// The plane is defined by Dot(N, X - P) = 0, where P is the plane origin and
// N is a unit-length normal for the plane.
//
// TODO: Modify to support non-unit-length N.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Hyperplane.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Vector<T, N>, Hyperplane<T, N>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                signedDistance(C_<T>(0)),
                closest{}
            {
                static_assert(
                    N >= 2,
                    "Invalid dimension.");
            }

            T distance, signedDistance;
            std::array<Vector<T, N>, 2> closest;
        };

        Output operator()(Vector<T, N> const& point, Hyperplane<T, N> const& hyperplane)
        {
            Output output{};
            output.signedDistance = Dot(hyperplane.normal, point) - hyperplane.constant;
            output.distance = std::fabs(output.signedDistance);
            output.closest[0] = point;
            output.closest[1] = point - output.signedDistance * hyperplane.normal;
            return output;
        }
    };
}
