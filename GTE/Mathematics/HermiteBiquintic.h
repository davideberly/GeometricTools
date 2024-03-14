// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2024.03.13

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
        // Use function Generate(...) for Hermite cubic interpolation on a
        // lattice. Generate the 6x6x6 coefficients c[][] for a cell of the
        // lattice with voxels at (x,y,z), (x+1,y,z), (x,y+1,z), (x+1,y+1,z),
        // (x,y,z+1), (x+1,y,z+1), (x,y+1,z+1), and (x+1,y+1,z+1). The caller
        // is responsible for tracking the voxel (x,y,z) that is associated
        // with the coefficients.
        struct Sample
        {
            Sample()
                :
                F(static_cast<T>(0)),
                Fx(static_cast<T>(0)), Fy(static_cast<T>(0)),
                Fxx(static_cast<T>(0)), Fxy(static_cast<T>(0)), Fyy(static_cast<T>(0)),
                Fxxy(static_cast<T>(0)), Fxyy(static_cast<T>(0)),
                Fxxyy(static_cast<T>(0))
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

        // Create the identically zero polynomial.
        HermiteBiquintic()
            :
            c{}
        {
            T const zero = static_cast<T>(0);
            for (std::size_t i = 0; i < 6; ++i)
            {
                for (std::size_t j = 0; j < 6; ++j)
                {
                    c[i][j] = zero;
                }
            }
        }

        HermiteBiquintic(std::array<std::array<Sample, 2>, 2> const& blocks)
            :
            c{}
        {
            Generate(blocks);
        }

        // Evaluate the polynomial with the specified orders. The returned
        // value is zero if xOrder >= 6 or yOrder >= 6. Otherwise, some
        // examples are the following where 'hermite' is of type
        // IntpHermiteBiquintic.
        //   G(x, y) = hermite(0, 0, x, y)
        //   Gx(x, y) = hermite(1, 0, x, y)
        //   Gy(x, y) = hermite(0, 1, x, y)
        //   Gxx(x, y) = hermite(2, 0, x, y)
        //   Gxy(x, y) = hermite(1, 1, x, y)
        //   Gyy(x, y) = hermite(0, 2, x, y)
        T operator()(std::size_t xOrder, std::size_t yOrder, T const& x, T const& y) const
        {
            T const zero = static_cast<T>(0);
            if (xOrder <= 5 && yOrder <= 5)
            {
                T result = zero;
                for (std::size_t i = 0; i < 6; ++i)
                {
                    T xValue = HermiteQuintic<T>::P(i, xOrder, x);
                    for (std::size_t j = 0; j < 6; ++j)
                    {
                        T yValue = HermiteQuintic<T>::P(j, yOrder, y);
                        result += c[i][j] * xValue * yValue;
                    }
                }
                return result;
            }
            else
            {
                return zero;
            }
        }

        void Generate(std::array<std::array<Sample, 2>, 2> const& blocks)
        {
            for (std::int32_t b0 = 0; b0 <= 1; ++b0)
            {
                std::int32_t z0 = 5 * b0 + 0;
                std::int32_t p0 = 3 * b0 + 1;
                std::int32_t q0 = 1 * b0 + 2;
                T s0 = static_cast<T>(1 - 2 * b0);

                for (std::int32_t b1 = 0; b1 <= 1; ++b1)
                {
                    std::int32_t z1 = 5 * b1 + 0;
                    std::int32_t p1 = 3 * b1 + 1;
                    std::int32_t q1 = 1 * b1 + 2;
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
        void Generate(Sample const& input, T& v00, T& v10, T& v01,
            T& v20, T& v11, T& v02, T& v21, T& v12, T& v22)
        {
            T const k2 = static_cast<T>(2);
            T const k4 = static_cast<T>(4);
            T const k5 = static_cast<T>(5);
            T const k10 = static_cast<T>(10);
            T const k16 = static_cast<T>(16);
            T const k20 = static_cast<T>(20);
            T const k25 = static_cast<T>(25);
            T const k40 = static_cast<T>(40);
            T const k50 = static_cast<T>(50);
            T const k1Div4 = static_cast<T>(0.25);
            T const k100 = static_cast<T>(100);
            T const k1Div2 = static_cast<T>(0.5);
            T const k5Div2 = static_cast<T>(2.5);
            T const& F = input.F;
            T const& Fx = input.Fx;
            T const& Fy = input.Fy;
            T const& Fxx = input.Fxx;
            T const& Fxy = input.Fxy;
            T const& Fyy = input.Fyy;
            T const& Fxxy = input.Fxxy;
            T const& Fxyy = input.Fxyy;
            T const& Fxxyy = input.Fxxyy;

            v00 = F;
            v10 = k5 * F + Fx;
            v01 = k5 * F + Fy;
            v20 = k10 * F + k4 * Fx + k1Div2 * Fxx;
            v11 = k25 * F + k5 * (Fx + Fy) + Fxy;
            v02 = k10 * F + k4 * Fy + k1Div2 * Fyy;
            v21 = k50 * F + k20 * Fx + k10 * Fy + k5Div2 * Fxx + k4 * Fxy + k1Div2 * Fxxy;
            v12 = k50 * F + k10 * Fx + k20 * Fy + k4 * Fxy + k5Div2 * Fyy + k1Div2 * Fxyy;
            v22 = k100 * F + k40 * (Fx + Fy) + k5 * (Fxx + Fyy) + k16 * Fxy + k2 * (Fxxy + Fxyy) + k1Div4 * Fxxyy;
        }
    };
}
