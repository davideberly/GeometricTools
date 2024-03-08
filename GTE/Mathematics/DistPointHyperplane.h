// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a point and a line (N = 2), between a point
// and a plane (N = 3) or generally between a point and a hyperplane (N >= 2).
// 
// The plane is defined by Dot(N, X - P) = 0, where P is the plane origin and
// N is a unit-length normal for the plane.
//
// TODO: Modify to support non-unit-length N.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Hyperplane.h>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Vector<N, T>, Hyperplane<N, T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                signedDistance(static_cast<T>(0)),
                closest{ Vector<N, T>::Zero(), Vector<N, T>::Zero() }
            {
                static_assert(
                    N >= 2,
                    "Invalid dimension.");
            }

            T distance, signedDistance;
            std::array<Vector<N, T>, 2> closest;
        };

        Result operator()(Vector<N, T> const& point, Hyperplane<N, T> const& plane)
        {
            Result result{};
            result.signedDistance = Dot(plane.normal, point) - plane.constant;
            result.distance = std::fabs(result.signedDistance);
            result.closest[0] = point;
            result.closest[1] = point - result.signedDistance * plane.normal;
            return result;
        }
    };
}
