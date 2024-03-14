// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2024.03.13

#pragma once

// The Hermite cubic polynomial is
//   H(x) = sum_{i=0}^3 c[i] * P(i,x)
// where P(i,x) = (1-x)^{3-i} * x^i. The domain is x in [0,1].
// Interpolation using these polynomials is described in
// https://www.geometrictools.com/Documentation/SmoothLatticeInterpolation.pdf

#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class HermiteCubic
    {
    public:
        // Use function Generate(...) for Hermite cubic interpolation on a
        // lattice. Generate the 4x1 coefficients c[] for a cell of the
        // lattice with pixels at (x) and (x+1). The caller is responsible for
        // tracking the pixel (x) that is associated with the coefficients.
        struct Sample
        {
            Sample()
                :
                F(static_cast<T>(0)),
                Fx(static_cast<T>(0))
            {
            }

            Sample(T const& f, T const& fx)
                :
                F(f),
                Fx(fx)
            {
            }

            T F, Fx;
        };

        // Create the identically zero polynomial.
        HermiteCubic()
            :
            c{}
        {
            c.fill(static_cast<T>(0));
        }

        HermiteCubic(std::array<Sample, 2> const& blocks)
            :
            c{}
        {
            Generate(blocks);
        }

        // Evaluate the polynomial with the specified order. The returned
        // value is zero if xOrder >= 4. Otherwise, some examples are the
        // following where 'hermite' is of type Hermite<Bi,Tri>cubic:
        //   H(x) = hermite(0, x)
        //   Hx(x) = hermite(1, x)
        //   Hxx(x) = hermite(2, x)
        //   Hxxx(x) = hermite(3, x)
        T operator()(std::size_t xOrder, T const& x) const
        {
            T const zero = static_cast<T>(0);
            if (xOrder <= 3)
            {
                T result = zero;
                for (std::size_t i = 0; i < 4; ++i)
                {
                    result += c[i] * P(i, xOrder, x);
                }
                return result;
            }
            else
            {
                return zero;
            }
        }

        void Generate(std::array<Sample, 2> const& blocks)
        {
            for (std::int32_t b0 = 0; b0 <= 1; ++b0)
            {
                std::int32_t z0 = 3 * b0 + 0;
                std::int32_t p0 = 1 * b0 + 1;
                T s0 = static_cast<T>(1 - 2 * b0);
                Sample const& b = blocks[b0];
                Sample input(b.F, s0 * b.Fx);
                GenerateSingle(input, c[z0], c[p0]);
            }
        }

    private:
        void GenerateSingle(Sample const& input, T& v0, T& v1)
        {
            T const k3 = static_cast<T>(3);
            T const& F = input.F;
            T const& Fx = input.Fx;

            v0 = F;
            v1 = k3 * F + Fx;
        }

        // Set the coefficients manually as desired. For Hermite cubic
        // interpolation on a lattice, use Generate(...). The lattice
        // interpolator is globally C1-continuous.
        std::array<T, 4> c;

    public: // For internal use in HermiteCubic{2,3}.

        // The 'select' parameter is the i argument for the polynomial and the
        // 'order' parameter is the order of the derivative.
        static T P(std::size_t select, std::size_t order, T const& t)
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

        static T P0D0(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P3D0(k1 - t);
        }

        static T P0D1(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P3D1(k1 - t);
        }

        static T P0D2(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P3D2(k1 - t);
        }

        static T P0D3(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P3D3(k1 - t);
        }

        static T P1D0(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P2D0(k1 - t);
        }

        static T P1D1(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P2D1(k1 - t);
        }

        static T P1D2(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P2D2(k1 - t);
        }

        static T P1D3(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P2D3(k1 - t);
        }

        static T P2D0(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return (k1 - t) * t * t;
        }

        static T P2D1(T const& t)
        {
            T const k2 = static_cast<T>(2);
            T const k3 = static_cast<T>(3);
            return t * (k2 - k3 * t);
        }

        static T P2D2(T const& t)
        {
            T const k1 = static_cast<T>(1);
            T const k2 = static_cast<T>(2);
            T const k3 = static_cast<T>(3);
            return k2 * (k1 - k3 * t);
        }

        static T P2D3(T const&)
        {
            T const k6 = static_cast<T>(6);
            return -k6;
        }

        static T P3D0(T const& t)
        {
            return t * t * t;
        }

        static T P3D1(T const& t)
        {
            T const k3 = static_cast<T>(3);
            return k3 * t * t;
        }

        static T P3D2(T const& t)
        {
            T const k6 = static_cast<T>(6);
            return k6 * t;
        }

        static T P3D3(T const&)
        {
            T const k6 = static_cast<T>(6);
            return k6;
        }
    };
}
