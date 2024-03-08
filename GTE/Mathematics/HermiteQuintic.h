// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.08.24

#pragma once

// The Hermite quintic polynomials are P(i,t) = (1-t)^{5-i} * t^i for
// 0 <= i <= 5. The domain is t in [0,1]. The class member function
// P(...) allows you to evaluate the polynomials and their derivatives for
// any order. If the order is 6 or larger, the corresponding derivative is
// zero. Note that P(0,t) = P(5,1-t), P(1,t) = P(4,1-t), and P(2,t) = P(3,1-t)
// which simplifies the implementations. Interpolation using these polynomials
// is described in
// https://www.geometrictools.com/Documentation/SmoothLatticeInterpolation.pdf

#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class HermiteQuintic
    {
    public:
        // The 'select' parameter is the i argument for the polynomial and the
        // 'order' parameter is the order of the derivative.
        static T P(size_t select, size_t order, T const& t)
        {
            static std::array<std::array<T(*)(T const&), 6>, 6> table =
            { {
                { P0D0, P0D1, P0D2, P0D3, P0D4, P0D5 },
                { P1D0, P1D1, P1D2, P1D3, P1D4, P1D5 },
                { P2D0, P2D1, P2D2, P2D3, P2D4, P2D5 },
                { P3D0, P3D1, P3D2, P3D3, P3D4, P3D5 },
                { P4D0, P4D1, P4D2, P4D3, P4D4, P4D5 },
                { P5D0, P5D1, P5D2, P5D3, P5D4, P5D5 }
            } };

            return (order <= 5 ? table[select][order](t) : static_cast<T>(0));
        }

    private:
        static T constexpr k1 = static_cast<T>(1);
        static T constexpr k3 = static_cast<T>(3);
        static T constexpr k4 = static_cast<T>(4);
        static T constexpr k5 = static_cast<T>(5);
        static T constexpr k6 = static_cast<T>(6);
        static T constexpr k12 = static_cast<T>(12);
        static T constexpr k20 = static_cast<T>(20);
        static T constexpr k24 = static_cast<T>(24);
        static T constexpr k48 = static_cast<T>(48);
        static T constexpr k60 = static_cast<T>(60);
        static T constexpr k120 = static_cast<T>(120);

        static T P0D0(T const& t) { return +P5D0(k1 - t); }
        static T P0D1(T const& t) { return -P5D1(k1 - t); }
        static T P0D2(T const& t) { return +P5D2(k1 - t); }
        static T P0D3(T const& t) { return -P5D3(k1 - t); }
        static T P0D4(T const& t) { return +P5D4(k1 - t); }
        static T P0D5(T const& t) { return -P5D5(k1 - t); }
        static T P1D0(T const& t) { return +P4D0(k1 - t); }
        static T P1D1(T const& t) { return -P4D1(k1 - t); }
        static T P1D2(T const& t) { return +P4D2(k1 - t); }
        static T P1D3(T const& t) { return -P4D3(k1 - t); }
        static T P1D4(T const& t) { return +P4D4(k1 - t); }
        static T P1D5(T const& t) { return -P4D5(k1 - t); }
        static T P2D0(T const& t) { return +P3D0(k1 - t); }
        static T P2D1(T const& t) { return -P3D1(k1 - t); }
        static T P2D2(T const& t) { return +P3D2(k1 - t); }
        static T P2D3(T const& t) { return -P3D3(k1 - t); }
        static T P2D4(T const& t) { return +P3D4(k1 - t); }
        static T P2D5(T const& t) { return -P3D4(k1 - t); }
        static T P3D0(T const& t) { return ((k1 - t) * t) * ((k1 - t) * t) * t; }
        static T P3D1(T const& t) { return (k1 - t) * t * t * (k3 - k5 * t); }
        static T P3D2(T const& t) { return t * (k6 + t * (-k24 + k20 * t)); }
        static T P3D3(T const& t) { return k6 + t * (-k48 + k60 * t); }
        static T P3D4(T const& t) { return -k48 + k120 * t; }
        static T P3D5(T const&  ) { return k120; }
        static T P4D0(T const& t) { return (k1 - t) * (t * t) * (t * t); }
        static T P4D1(T const& t) { return t * t * t * (k4 - k5 * t); }
        static T P4D2(T const& t) { return t * t * (k12 - k20 * t); }
        static T P4D3(T const& t) { return t * (k24 - k60 * t); }
        static T P4D4(T const& t) { return k24 - k120 * t; }
        static T P4D5(T const&  ) { return -k120; }
        static T P5D0(T const& t) { return t * (t * t) * (t * t); }
        static T P5D1(T const& t) { return k5 * (t * t) * (t * t); }
        static T P5D2(T const& t) { return k20 * t * (t * t); }
        static T P5D3(T const& t) { return k60 * t * t; }
        static T P5D4(T const& t) { return k120 * t; }
        static T P5D5(T const&  ) { return k120; }
    };
}
