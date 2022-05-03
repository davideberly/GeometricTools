// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Read the comments in Slerp.h about the slerp function. In particular, if
// you are using quaternions to represent rotations, read the comments about
// preprocessing the quaternions before calling slerp. The slerp functions in
// Slerp.h require angles in [0,pi). The first two slerp estimates implemented
// in this file require angles in [0,pi/2], because the estimates are based on
// Chebyshev ratio estimates that have the same angle requirement. The third
// estimate that uses the qh inputs allows for angles in [0,pi).

#include <GTL/Mathematics/Functions/ChebyshevRatioEstimate.h>
#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <array>

namespace gtl
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

        T cosA = C_<T>(0);
        for (size_t i = 0; i < N; ++i)
        {
            cosA += q0[i] * q1[i];
        }

        auto f = ChebyshevRatioEstimate<T, D>(t, cosA);

        std::array<T, N> result{};
        result.fill(C_<T>(0));
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
        result.fill(C_<T>(0));
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
        result.fill(C_<T>(0));

        T twoT = C_<T>(2) * t;
        if (twoT <= C_<T>(1))
        {
            auto f = ChebyshevRatioEstimate<T, D>(twoT, cosAH);
            for (size_t i = 0; i < N; ++i)
            {
                result[i] += f[0] * q0[i] + f[1] * qh[i];
            }
        }
        else
        {
            auto f = ChebyshevRatioEstimate<T, D>(twoT - C_<T>(1), cosAH);
            for (size_t i = 0; i < N; ++i)
            {
                result[i] += f[0] * qh[i] + f[1] * q1[i];
            }
        }
        return result;
    }
}
