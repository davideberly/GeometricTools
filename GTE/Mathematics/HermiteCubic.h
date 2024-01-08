// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.08.24

#pragma once

// The Hermite cubic polynomials are P(i,t) = (1-t)^{3-i} * t^i for
// 0 <= i <= 3. The domain is t in [0,1]. The class member function
// P(...) allows you to evaluate the polynomials and their derivatives for
// any order. If the order is 4 or larger, the corresponding derivative is
// zero. Note that P(0,t) = P(3,1-t) and P(1,t) = P(2,1-t) which simplifies
// the implementations. Interpolation using these polynomials is described in
// https://www.geometrictools.com/Documentation/SmoothLatticeInterpolation.pdf

#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class HermiteCubic
    {
    public:
        // The 'select' parameter is the i argument for the polynomial and the
        // 'order' parameter is the order of the derivative.
        static T P(size_t select, size_t order, T const& t)
        {
            static std::array<std::array<T(*)(T const&), 4>, 4> table =
            { {
                { P0D0, P0D1, P0D2, P0D3 },
                { P1D0, P1D1, P1D2, P1D3 },
                { P2D0, P2D1, P2D2, P2D3 },
                { P3D0, P3D1, P3D2, P3D3 }
            } };

            return (order <= 3 ? table[select][order](t) : static_cast<T>(0));
        }

    private:
        static T constexpr k1 = static_cast<T>(1);
        static T constexpr k2 = static_cast<T>(2);
        static T constexpr k3 = static_cast<T>(3);
        static T constexpr k6 = static_cast<T>(6);

        static T P0D0(T const& t) { return +P3D0(k1 - t); }
        static T P0D1(T const& t) { return -P3D1(k1 - t); }
        static T P0D2(T const& t) { return +P3D2(k1 - t); }
        static T P0D3(T const& t) { return -P3D3(k1 - t); }
        static T P1D0(T const& t) { return +P2D0(k1 - t); }
        static T P1D1(T const& t) { return -P2D1(k1 - t); }
        static T P1D2(T const& t) { return +P2D2(k1 - t); }
        static T P1D3(T const& t) { return -P2D3(k1 - t); }
        static T P2D0(T const& t) { return (k1 - t) * t * t; }
        static T P2D1(T const& t) { return t * (k2 - k3 * t); }
        static T P2D2(T const& t) { return k2 * (k1 - k3 * t); }
        static T P2D3(T const&  ) { return -k6; }
        static T P3D0(T const& t) { return t * t * t; }
        static T P3D1(T const& t) { return k3 * t * t; }
        static T P3D2(T const& t) { return k6 * t; }
        static T P3D3(T const& )  { return k6; }
    };
}
