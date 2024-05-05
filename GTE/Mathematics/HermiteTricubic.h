// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2024.03.13

#pragma once

// The Hermite tricubic polynomial is
//   H(x,y,z) = sum_{i=0}^3 sum_{j=0}^3 sum_{k=0}^3
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
        // Use function Generate(...) for Hermite cubic interpolation on a
        // lattice. Generate the 4x4x4 coefficients c[][][] for a cell of the
        // lattice with pixels at (x,y,z), (x+1,y,z), (x,y+1,z), (x+1,y+1,z),
        // (x,y,z+1), (x+1,y,z+1), (x,y+1,z+1), and (x+1,y+1,z+1). The caller
        // is responsible for tracking the pixel (x,y,z) that is associated
        // with the coefficients.
        struct Sample
        {
            Sample()
                :
                F(static_cast<T>(0)),
                Fx(static_cast<T>(0)), Fy(static_cast<T>(0)), Fz(static_cast<T>(0)),
                Fxy(static_cast<T>(0)), Fxz(static_cast<T>(0)), Fyz(static_cast<T>(0)),
                Fxyz(static_cast<T>(0))
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

        // Create the identically zero polynomial.
        HermiteTricubic()
            :
            c{}
        {
            T const zero = static_cast<T>(0);
            for (std::size_t i = 0; i < 4; ++i)
            {
                for (std::size_t j = 0; j < 4; ++j)
                {
                    for (std::size_t k = 0; k < 4; ++k)
                    {
                        c[i][j][k] = zero;
                    }
                }
            }
        }

        HermiteTricubic(std::array<std::array<std::array<Sample, 2>, 2>, 2> const& blocks)
            :
            c{}
        {
            Generate(blocks);
        }

        // Evaluate the polynomial with the specified orders. The returned
        // value is zero if xOrder >= 4 or yOrder >= 4 or zOrder >= 4.
        // Otherwise, some examples are the following where 'hermite' is of
        // type IntpHermiteCubic3<T>:
        //   H(x, y, z) = hermite(0, 0, 0, x, y, z)
        //   Hx(x, y, z) = hermite(1, 0, 0, x, y, z)
        //   Hy(x, y, z) = hermite(0, 1, 0, x, y, z)
        //   Hz(x, y, z) = hermite(0, 0, 1, x, y, z)
        //   Hxx(x, y, z) = hermite(2, 0, 0, x, y, z)
        //   Hxy(x, y, z) = hermite(1, 1, 0, x, y, z)
        //   Hxz(x, y, z) = hermite(1, 0, 1, x, y, z)
        //   Hyy(x, y, z) = hermite(0, 2, 0, x, y, z)
        //   Hyz(x, y, z) = hermite(0, 1, 1, x, y, z)
        //   Hzz(x, y, z) = hermite(0, 0, 2, x, y, z)
        T operator()(std::size_t xOrder, std::size_t yOrder, std::size_t zOrder,
            T const& x, T const& y, T const& z) const
        {
            T const zero = static_cast<T>(0);
            if (xOrder <= 3 && yOrder <= 3 && zOrder <= 3)
            {
                T result = zero;
                for (std::size_t i = 0; i < 4; ++i)
                {
                    T xValue = HermiteCubic<T>::P(i, xOrder, x);
                    for (std::size_t j = 0; j < 4; ++j)
                    {
                        T yValue = HermiteCubic<T>::P(j, yOrder, y);
                        T xyValue = xValue * yValue;
                        for (std::size_t k = 0; k < 4; ++k)
                        {
                            T zValue = HermiteCubic<T>::P(k, zOrder, z);
                            result += c[i][j][k] * xyValue * zValue;
                        }
                    }
                }
                return result;
            }
            else
            {
                return zero;
            }
        }

        void Generate(std::array<std::array<std::array<Sample, 2>, 2>, 2> const& blocks)
        {
            for (std::int32_t b0 = 0; b0 <= 1; ++b0)
            {
                std::int32_t z0 = 3 * b0 + 0;
                std::int32_t p0 = 1 * b0 + 1;
                T s0 = static_cast<T>(1 - 2 * b0);

                for (std::int32_t b1 = 0; b1 <= 1; ++b1)
                {
                    std::int32_t z1 = 3 * b1 + 0;
                    std::int32_t p1 = 1 * b1 + 1;
                    T s1 = static_cast<T>(1 - 2 * b1);
                    T s0s1 = s0 * s1;

                    for (std::int32_t b2 = 0; b2 <= 1; ++b2)
                    {
                        std::int32_t z2 = 3 * b2 + 0;
                        std::int32_t p2 = 1 * b2 + 1;
                        T s2 = static_cast<T>(1 - 2 * b2);
                        T s0s2 = s0 * s2;
                        T s1s2 = s1 * s2;
                        T s0s1s2 = s0 * s1s2;

                        Sample const& b = blocks[b0][b1][b2];
                        Sample input(b.F, s0 * b.Fx, s1 * b.Fy, s2 * b.Fz,
                            s0s1 * b.Fxy, s0s2 * b.Fxz, s1s2 * b.Fyz, s0s1s2 * b.Fxyz);

                        GenerateSingle(input, c[z0][z1][z2], c[p0][z1][z2], c[z0][p1][z2],
                            c[z0][z1][p2], c[p0][p1][z2], c[p0][z1][p2],
                            c[z0][p1][p2], c[p0][p1][p2]);
                    }
                }
            }
        }

        // Set the coefficients manually as desired. For Hermite cubic
        // interpolation on a lattice, use Generate(...). The lattice
        // interpolator is globally C1-continuous.
        std::array<std::array<std::array<T, 4>, 4>, 4> c;

    private:
        void GenerateSingle(Sample const& input, T& v000, T& v100, T& v010,
            T& v001, T& v110, T& v101, T& v011, T& v111)
        {
            T const k3 = static_cast<T>(3);
            T const k9 = static_cast<T>(9);
            T const k27 = static_cast<T>(27);
            T const& F = input.F;
            T const& Fx = input.Fx;
            T const& Fy = input.Fy;
            T const& Fz = input.Fz;
            T const& Fxy = input.Fxy;
            T const& Fxz = input.Fxz;
            T const& Fyz = input.Fyz;
            T const& Fxyz = input.Fxyz;

            v000 = F;
            v100 = k3 * F + Fx;
            v010 = k3 * F + Fy;
            v001 = k3 * F + Fz;
            v110 = k9 * F + k3 * (Fx + Fy) + Fxy;
            v101 = k9 * F + k3 * (Fx + Fz) + Fxz;
            v011 = k9 * F + k3 * (Fy + Fz) + Fyz;
            v111 = k27 * F + k9 * (Fx + Fy + Fz) + k3 * (Fxy + Fxz + Fyz) + Fxyz;
        }
    };
}
