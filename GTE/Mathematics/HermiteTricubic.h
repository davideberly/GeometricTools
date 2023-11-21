// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.11.20

#pragma once

// The Hermite tricubic polynomial is
//   G(x,y,z) = sum_{i=0}^3 sum_{j=0}^3 sum_{k=0}^3
//              c[i][j][k] * P(i,x) * P(j,y) * P(k,z)
// where P(i,t) = (1-t)^{3-i} * t^i. The domain is (x,y,z) in [0,1]^3.
// Interpolation using these polynomials is described in
// https://www.geometrictools.com/Documentation/SmoothLatticeInterpolation.pdf

#include <Mathematics/HermiteCubic.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class HermiteTricubic
    {
    public:
        // Create the identically zero polynomial.
        HermiteTricubic()
            :
            c{}
        {
            for (size_t i = 0; i < 4; ++i)
            {
                for (size_t j = 0; j < 4; ++j)
                {
                    for (size_t k = 0; k < 4; ++k)
                    {
                        c[i][j][k] = static_cast<T>(0);
                    }
                }
            }
        }

        // Evaluate the polynomial with the specified orders. The returned
        // value is zero if xOrder >= 4 or yOrder >= 4 or zOrder >= 4.
        // Otherwise, some examples are the following where 'hermite' is of
        // type HermiteTricubic.
        //   G(x, y, z) = hermite(0, 0, 0, x, y, z)
        //   Gx(x, y, z) = hermite(1, 0, 0, x, y, z)
        //   Gy(x, y, z) = hermite(0, 1, 0, x, y, z)
        //   Gz(x, y, z) = hermite(0, 0, 1, x, y, z)
        //   Gxx(x, y, z) = hermite(2, 0, 0, x, y, z)
        //   Gxy(x, y, z) = hermite(1, 1, 0, x, y, z)
        //   Gxz(x, y, z) = hermite(1, 0, 1, x, y, z)
        //   Gyy(x, y, z) = hermite(0, 2, 0, x, y, z)
        //   Gyz(x, y, z) = hermite(0, 1, 1, x, y, z)
        //   Gzz(x, y, z) = hermite(0, 0, 2, x, y, z)
        T operator()(size_t xOrder, size_t yOrder, size_t zOrder, T const& x, T const& y, T const& z) const
        {
            if (xOrder <= 3 && yOrder <= 3 && zOrder <= 3)
            {
                T result = static_cast<T>(0);
                for (size_t i = 0; i < 4; ++i)
                {
                    for (size_t j = 0; j < 4; ++j)
                    {
                        for (size_t k = 0; k < 4; ++k)
                        {
                            result += c[i][j][k] *
                                HermiteCubic<T>::P(i, xOrder, x) *
                                HermiteCubic<T>::P(j, yOrder, y) *
                                HermiteCubic<T>::P(k, zOrder, z);
                        }
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
                Fx(k0), Fy(k0), Fz(k0),
                Fxy(k0), Fxz(k0), Fyz(k0),
                Fxyz(k0)
            {
            }

            Sample(
                T const& f,
                T const& fx, T const& fy, T const& fz,
                T const& fxy, T const& fxz, T const& fyz,
                T const& fxyz)
                :
                F(f),
                Fx(fx), Fy(fy), Fz(fz),
                Fxy(fxy), Fxz(fxz), Fyz(fyz),
                Fxyz(fxyz)
            {
            }

            T F;
            T Fx, Fy, Fz;
            T Fxy, Fxz, Fyz;
            T Fxyz;
        };

        // Use this function for Hermite tricubic interpolation on a lattice.
        // Generate the 4x4x4 coefficients c[][] for a cell of the lattice
        // with pixels at (x,y,z), (x+1,y,z), (x,y+1,z), (x+1,y+1,z),
        // (x,y,z+1), (x+1,y,z+1), (x,y+1,z+1), and (x+1,y+1,z+1). The caller
        // is responsible for tracking the pixel (x,y) that is associated with
        // the coefficients.
        void Generate(std::array<std::array<std::array<Sample, 2>, 2>, 2> const& blocks)
        {
            for (int32_t b0 = 0; b0 <= 1; ++b0)
            {
                int32_t z0 = 3 * b0 + 0;
                int32_t p0 = 1 * b0 + 1;
                T s0 = static_cast<T>(1 - 2 * b0);

                for (int32_t b1 = 0; b1 <= 1; ++b1)
                {
                    int32_t z1 = 3 * b1 + 0;
                    int32_t p1 = 1 * b1 + 1;
                    T s1 = static_cast<T>(1 - 2 * b1);
                    T s0s1 = s0 * s1;

                    for (int32_t b2 = 0; b2 <= 1; ++b2)
                    {
                        int32_t z2 = 3 * b2 + 0;
                        int32_t p2 = 1 * b2 + 1;
                        T s2 = static_cast<T>(1 - 2 * b2);
                        T s0s2 = s0 * s2;
                        T s1s2 = s1 * s2;
                        T s0s1s2 = s0 * s1s2;

                        Sample const& b = blocks[b0][b1][b2];
                        Sample input(b.F, s0 * b.Fx, s1 * b.Fy, s2 * b.Fz,
                            s0s1 * b.Fxy, s0s2 * b.Fxz, s1s2 * b.Fyz, s0s1s2 * b.Fxyz);

                        Generate(input, c[z0][z1][z2], c[p0][z1][z2], c[z0][p1][z2],
                            c[z0][z1][p2], c[p0][p1][z2], c[p0][z1][p2],
                            c[z0][p1][p2], c[p0][p1][p2]);
                    }
                }
            }
        }

        // Set the coefficients manually as desired. For Hermite tricubic
        // interpolation on a lattice, use Generate(...). The lattice
        // interpolator is globally C1-continuous.
        std::array<std::array<std::array<T, 4>, 4>, 4> c;

    private:
        static T constexpr k3 = static_cast<T>(3);
        static T constexpr k9 = static_cast<T>(9);
        static T constexpr k27 = static_cast<T>(27);

        void Generate(Sample const& input, T& v000, T& v100, T& v010,
            T& v001, T& v110, T& v101, T& v011, T& v111)
        {
            v000 = input.F;
            v100 = k3 * v000 + input.Fx;
            v010 = k3 * v000 + input.Fy;
            v001 = k3 * v000 + input.Fz;
            v110 = -k9 * v000 + k3 * (v100 + v010) + input.Fxy;
            v101 = -k9 * v000 + k3 * (v100 + v001) + input.Fxz;
            v011 = -k9 * v000 + k3 * (v010 + v001) + input.Fyz;
            v111 = k27 * v000 - k9 * (v100 + v010 + v001) + k3 * (v110 + v101 + v011) + input.Fxyz;
        }
    };
}
