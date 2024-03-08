// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Read the comments in Slerp.h about the slerp function. In particular, if
// you are using quaternions to represent rotations, read the comments about
// preprocessing the quaternions before calling slerp. The slerp functions in
// Slerp.h require angles in [0,pi). The first two slerp estimates implemented
// in this file require angles in [0,pi/2], because the estimates are based on
// Chebyshev ratio estimates that have the same angle requirement. The third
// estimate that uses the qh inputs allows for angles in [0,pi).

#include <Mathematics/ChebyshevRatioEstimate.h>
#include <Mathematics/Constants.h>
#include <array>
#include <cstddef>

namespace gte
{
    // The angle between q0 and q1 is in [0,pi/2].
    template <typename T, size_t N, size_t D>
    std::array<T, N> SlerpEstimate(T t,
        std::array<T, N> const& q0, std::array<T, N> const& q1)
    {
        static_assert(
            N >= 2,
            "Invalid dimension.");

        static_assert(
            1 <= D && D <= 16,
            "Invalid degree.");

        T const zero = static_cast<T>(0);
        T cosA = zero;
        for (size_t i = 0; i < N; ++i)
        {
            cosA += q0[i] * q1[i];
        }

        auto f = ChebyshevRatioEstimate<T, D>(t, cosA);

        std::array<T, N> result{};
        result.fill(zero);
        for (size_t i = 0; i < N; ++i)
        {
            result[i] += f[0] * q0[i] + f[1] * q1[i];
        }
        return result;
    }

    // The angle between q0 and q1 must be in [0,pi/2] and cosA = Dot(q0,q1).
    template <typename T, size_t N, size_t D>
    std::array<T, N> SlerpEstimate(T t,
        std::array<T, N> const& q0, std::array<T, N> const& q1, T cosA)
    {
        static_assert(
            N >= 2,
            "Invalid dimension.");

        static_assert(
            1 <= D && D <= 16,
            "Invalid degree.");

        auto f = ChebyshevRatioEstimate<T, D>(t, cosA);

        std::array<T, N> result{};
        result.fill(static_cast<T>(0));
        for (size_t i = 0; i < N; ++i)
        {
            result[i] += f[0] * q0[i] + f[1] * q1[i];
        }
        return result;
    }

    // The angle between q0 and q1 is in [0,pi). The input qh is halfway
    // between q0 and q1 along a hyperspherical arc. If cosA = Dot(q0,q1),
    // then cosAH = sqrt((1+cosA)/2) and qh = (q0+q1)/(2*cosAH).
    template <typename T, size_t N, size_t D>
    std::array<T, N> SlerpEstimate(T t,
        std::array<T, N> const& q0, std::array<T, N> const& q1,
        std::array<T, N> const& qh, T cosAH)
    {
        static_assert(
            N >= 2,
            "Invalid dimension.");

        static_assert(
            1 <= D && D <= 16,
            "Invalid degree.");

        std::array<T, N> result{};
        result.fill(static_cast<T>(0));

        T const one = static_cast<T>(1);
        T twoT = static_cast<T>(2) * t;
        if (twoT <= one)
        {
            auto f = ChebyshevRatioEstimate<T, D>(twoT, cosAH);
            for (size_t i = 0; i < N; ++i)
            {
                result[i] += f[0] * q0[i] + f[1] * qh[i];
            }
        }
        else
        {
            auto f = ChebyshevRatioEstimate<T, D>(twoT - one, cosAH);
            for (size_t i = 0; i < N; ++i)
            {
                result[i] += f[0] * qh[i] + f[1] * q1[i];
            }
        }
        return result;
    }
}
