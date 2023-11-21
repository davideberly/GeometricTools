// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.11.20

#pragma once

// The Hermite biquintic polynomial is
//   G(x,y) = sum_{i=0}^5 sum_{j=0}^5 c[i][j] * P(i,x) * P(j,y)
// where P(i,t) = (1-t)^{5-i} * t^i. The domain is (x,y) in [0,1]^2.
// Interpolation using these polynomials is described in
// https://www.geometrictools.com/Documentation/SmoothLatticeInterpolation.pdf

#include <Mathematics/HermiteQuintic.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class HermiteBiquintic
    {
    public:
        // Create the identically zero polynomial.
        HermiteBiquintic()
            :
            c{}
        {
            for (size_t i = 0; i < 6; ++i)
            {
                for (size_t j = 0; j < 6; ++j)
                {
                    c[i][j] = static_cast<T>(0);
                }
            }
        }

        // Evaluate the polynomial with the specified orders. The returned
        // value is zero if xOrder >= 6 or yOrder >= 6. Otherwise, some
        // examples are the following where 'hermite' is of type
        // HermiteBiquintic.
        //   G(x, y) = hermite(0, 0, x, y)
        //   Gx(x, y) = hermite(1, 0, x, y)
        //   Gy(x, y) = hermite(0, 1, x, y)
        //   Gxx(x, y) = hermite(2, 0, x, y)
        //   Gxy(x, y) = hermite(1, 1, x, y)
        //   Gyy(x, y) = hermite(0, 2, x, y)
        T operator()(size_t xOrder, size_t yOrder, T const& x, T const& y) const
        {
            if (xOrder <= 5 && yOrder <= 5)
            {
                T result = static_cast<T>(0);
                for (size_t i = 0; i < 6; ++i)
                {
                    for (size_t j = 0; j < 6; ++j)
                    {
                        result += c[i][j] *
                            HermiteQuintic<T>::P(i, xOrder, x) *
                            HermiteQuintic<T>::P(j, yOrder, y);
                    }
                }
                return result;
            }
            else
            {
                return static_cast<T>(0);
            }
        }

        struct Sample
        {
            static T constexpr k0 = static_cast<T>(0);

            Sample()
                :
                F(k0),
                Fx(k0), Fy(k0),
                Fxx(k0), Fxy(k0), Fyy(k0),
                Fxxy(k0), Fxyy(k0),
                Fxxyy(k0)
            {
            }

            Sample(
                T const& f,
                T const& fx, T const& fy,
                T const& fxx, T const& fxy, T const& fyy,
                T const& fxxy, T const& fxyy, T const& fxxyy)
                :
                F(f),
                Fx(fx), Fy(fy),
                Fxx(fxx), Fxy(fxy), Fyy(fyy),
                Fxxy(fxxy), Fxyy(fxyy),
                Fxxyy(fxxyy)
            {
            }

            T F;
            T Fx, Fy;
            T Fxx, Fxy, Fyy;
            T Fxxy, Fxyy;
            T Fxxyy;
        };

        // Use this function for Hermite biquintic interpolation on a lattice.
        // Generate the 6x6 coefficients c[][] for a cell of the lattice with
        // pixels at (x,y), (x+1,y), (x,y+1), and (x+1,y+1). The caller is
        // responsible for tracking the pixel (x,y) that is associated with
        // the coefficients.
        void Generate(std::array<std::array<Sample, 2>, 2> const& blocks)
        {
            for (int32_t b0 = 0; b0 <= 1; ++b0)
            {
                int32_t z0 = 5 * b0 + 0;
                int32_t p0 = 3 * b0 + 1;
                int32_t q0 = 1 * b0 + 2;
                T s0 = static_cast<T>(1 - 2 * b0);

                for (int32_t b1 = 0; b1 <= 1; ++b1)
                {
                    int32_t z1 = 5 * b1 + 0;
                    int32_t p1 = 3 * b1 + 1;
                    int32_t q1 = 1 * b1 + 2;
                    T s1 = static_cast<T>(1 - 2 * b1);
                    T s0s1 = s0 * s1;

                    Sample const& b = blocks[b0][b1];
                    Sample input(b.F, s0 * b.Fx, s1 * b.Fy, b.Fxx,
                        s0s1 * b.Fxy, b.Fyy, s1 * b.Fxxy, s0 * b.Fxyy, b.Fxxyy);

                    Generate(input, c[z0][z1], c[p0][z1], c[z0][p1], c[q0][z1],
                        c[p0][p1], c[z0][q1], c[q0][p1], c[p0][q1], c[q0][q1]);
                }
            }
        }

        // Set the coefficients manually as desired. For Hermite biquintic
        // interpolation on a lattice, use Generate(...). The lattice
        // interpolator is globally C2-continuous.
        std::array<std::array<T, 6>, 6> c;

    private:
        static T constexpr k2 = static_cast<T>(2);
        static T constexpr k4 = static_cast<T>(4);
        static T constexpr k5 = static_cast<T>(5);
        static T constexpr k10 = static_cast<T>(10);
        static T constexpr k16 = static_cast<T>(16);
        static T constexpr k20 = static_cast<T>(20);
        static T constexpr k25 = static_cast<T>(25);
        static T constexpr k40 = static_cast<T>(40);
        static T constexpr k50 = static_cast<T>(50);
        static T constexpr k100 = static_cast<T>(100);

        void Generate(Sample const& input,T& v00, T& v10, T& v01,
            T& v20, T& v11, T& v02, T& v21, T& v12, T& v22)
        {
            v00 = input.F;
            v10 = k5 * v00 + input.Fx;
            v01 = k5 * v00 + input.Fy;
            v20 = -k10 * v00 + k4 * v10 + input.Fxx / k2;
            v11 = -k25 * v00 + k5 * (v10 + v01) + input.Fxy;
            v02 = -k10 * v00 + k4 * v01 + input.Fyy / k2;
            v21 = k50 * v00 - k20 * v10 - k10 * v01 + k5 * v20 + k4 * v11 + input.Fxxy / k2;
            v12 = k50 * v00 - k20 * v01 - k10 * v10 + k5 * v02 + k4 * v11 + input.Fxyy / k2;
            v22 = -k100 * v00 + k40 * (v10 + v01) - k10 * (v20 + v02) - k16 * v11 + k4 * (v21 + v12) + input.Fxxyy / k4;
        }
    };
}
