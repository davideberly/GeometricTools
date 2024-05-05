// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2024.03.13

#pragma once

// The Hermite bicubic polynomial is
//   H(x,y) = sum_{i=0}^3 sum_{j=0}^3 c[i][j] * P(i,x) * P(j,y)
// where P(i,t) = (1-t)^{3-i} * t^i. The domain is (x,y) in [0,1]^2.
// Interpolation using these polynomials is described in
// https://www.geometrictools.com/Documentation/SmoothLatticeInterpolation.pdf

#include <Mathematics/HermiteCubic.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class HermiteBicubic
    {
    public:
        // Use function Generate(...) for Hermite cubic interpolation on a
        // lattice. Generate the 4x4 coefficients c[][] for a cell of the
        // lattice with pixels at (x,y), (x+1,y), (x,y+1), and (x+1,y+1). The
        // caller is responsible for tracking the pixel (x,y) that is
        // associated with the coefficients.
        struct Sample
        {
            Sample()
                :
                F(static_cast<T>(0)),
                Fx(static_cast<T>(0)), Fy(static_cast<T>(0)),
                Fxy(static_cast<T>(0))
            {
            }

            Sample(
                T const& f,
                T const& fx, T const& fy,
                T const& fxy)
                :
                F(f),
                Fx(fx), Fy(fy),
                Fxy(fxy)
            {
            }

            T F;
            T Fx, Fy;
            T Fxy;
        };

        // Create the identically zero polynomial.
        HermiteBicubic()
            :
            c{}
        {
            T const zero = static_cast<T>(0);
            for (std::size_t i = 0; i < 4; ++i)
            {
                for (std::size_t j = 0; j < 4; ++j)
                {
                    c[i][j] = zero;
                }
            }
        }

        HermiteBicubic(std::array<std::array<Sample, 2>, 2> const& blocks)
            :
            c{}
        {
            Generate(blocks);
        }

        // Evaluate the polynomial with the specified orders. The returned
        // value is zero if xOrder >= 4 or yOrder >= 4. Otherwise, some
        // examples are the following where 'hermite' is of type
        // IntpHermiteCubic2<T>:
        //   H(x, y) = hermite(0, 0, x, y)
        //   Hx(x, y) = hermite(1, 0, x, y)
        //   Hy(x, y) = hermite(0, 1, x, y)
        //   Hxx(x, y) = hermite(2, 0, x, y)
        //   Hxy(x, y) = hermite(1, 1, x, y)
        //   Hyy(x, y) = hermite(0, 2, x, y)
        T operator()(std::size_t xOrder, std::size_t yOrder, T const& x, T const& y) const
        {
            T const zero = static_cast<T>(0);
            if (xOrder <= 3 && yOrder <= 3)
            {
                T result = zero;
                for (std::size_t i = 0; i < 4; ++i)
                {
                    T xValue = HermiteCubic<T>::P(i, xOrder, x);
                    for (std::size_t j = 0; j < 4; ++j)
                    {
                        T yValue = HermiteCubic<T>::P(j, yOrder, y);
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
                std::int32_t z0 = 3 * b0 + 0;
                std::int32_t p0 = 1 * b0 + 1;
                T s0 = static_cast<T>(1 - 2 * b0);

                for (std::int32_t b1 = 0; b1 <= 1; ++b1)
                {
                    std::int32_t z1 = 3 * b1 + 0;
                    std::int32_t p1 = 1 * b1 + 1;
                    T s1 = static_cast<T>(1 - 2 * b1);
                    T s0s1 = s0 * s1;

                    Sample const& b = blocks[b0][b1];
                    Sample input(b.F, s0 * b.Fx, s1 * b.Fy, s0s1 * b.Fxy);

                    GenerateSingle(input, c[z0][z1], c[p0][z1], c[z0][p1], c[p0][p1]);
                }
            }
        }

        // Set the coefficients manually as desired. For Hermite cubic
        // interpolation on a lattice, use Generate(...). The lattice
        // interpolator is globally C1-continuous.
        std::array<std::array<T, 4>, 4> c;

    private:
        void GenerateSingle(Sample const& input, T& v00, T& v10, T& v01, T& v11)
        {
            T const k3 = static_cast<T>(3);
            T const k9 = static_cast<T>(9);
            T const& F = input.F;
            T const& Fx = input.Fx;
            T const& Fy = input.Fy;
            T const& Fxy = input.Fxy;

            v00 = F;
            v10 = k3 * F + Fx;
            v01 = k3 * F + Fy;
            v11 = k9 * F + k3 * (Fx + Fy) + Fxy;
        }
    };
}
