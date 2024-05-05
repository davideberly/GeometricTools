// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2024.03.13

#pragma once

// The Hermite quintic polynomial is
//   H(x) = sum_{i=0}^5 c[i] * P(i,x)
// where are P(i,x) = (1-x)^{5-i} * t^x. The domain is x in [0,1].
// Interpolation using these polynomials is described in
// https://www.geometrictools.com/Documentation/SmoothLatticeInterpolation.pdf

#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class HermiteQuintic
    {
    public:
        // Use function Generate(...) for Hermite quintic interpolation on a
        // lattice. Generate the 6x1 coefficients c[] for a cell of the
        // lattice with pixels at (x) and (x+1). The caller is responsible for
        // tracking the pixel (x) that is associated with the coefficients.
        struct Sample
        {
            Sample()
                :
                F(static_cast<T>(0)),
                Fx(static_cast<T>(0)),
                Fxx(static_cast<T>(0))
            {
            }

            Sample(T const& f, T const& fx, T const& fxx)
                :
                F(f),
                Fx(fx),
                Fxx(fxx)
            {
            }

            T F, Fx, Fxx;
        };

        // Create the identically zero polynomial.
        HermiteQuintic()
            :
            c{}
        {
            c.fill(static_cast<T>(0));
        }

        HermiteQuintic(std::array<Sample, 2> const& blocks)
            :
            c{}
        {
            Generate(blocks);
        }

        // Evaluate the polynomial with the specified order. The returned
        // value is zero if xOrder >= 6. Otherwise, some examples are the
        // following where 'hermite' is of type HermiteQuintic<T>:
        //   H(x) = hermite(0, x)
        //   Hx(x) = hermite(1, x)
        //   Hxx(x) = hermite(2, x)
        //   Hxxx(x) = hermite(3, x)
        //   Hxxxx(x) = hermite(4, x)
        //   Hxxxx(x) = hermite(5, x)
        T operator()(std::size_t xOrder, T const& x) const
        {
            T const zero = static_cast<T>(0);
            if (xOrder <= 5)
            {
                T result = zero;
                for (std::size_t i = 0; i < 6; ++i)
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
                std::int32_t z0 = 5 * b0 + 0;
                std::int32_t p0 = 3 * b0 + 1;
                std::int32_t q0 = 1 * b0 + 2;
                T s0 = static_cast<T>(1 - 2 * b0);
                Sample const& b = blocks[b0];
                Sample input(b.F, s0 * b.Fx, b.Fxx);
                GenerateSingle(input, c[z0], c[p0], c[q0]);
            }
        }

    private:
        void GenerateSingle(Sample const& input, T& v0, T& v1, T& v2)
        {
            T const k4 = static_cast<T>(4);
            T const k5 = static_cast<T>(5);
            T const k10 = static_cast<T>(10);
            T const k1Div2 = static_cast<T>(0.5);
            T const& F = input.F;
            T const& Fx = input.Fx;
            T const& Fxx = input.Fxx;

            v0 = F;
            v1 = k5 * F + Fx;
            v2 = k10 * F + k4 * Fx + k1Div2 * Fxx;
        }

        // Set the coefficients manually as desired. For Hermite cubic
        // interpolation on a lattice, use Generate(...). The lattice
        // interpolator is globally C1-continuous.
        std::array<T, 6> c;

    public: // For internal use in Hermite<Bi,Tri>quintic.

        // The 'select' parameter is the i argument for the polynomial and the
        // 'order' parameter is the order of the derivative.
        static T P(std::size_t select, std::size_t order, T const& t)
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

        static T P0D0(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P5D0(k1 - t);
        }

        static T P0D1(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P5D1(k1 - t);
        }

        static T P0D2(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P5D2(k1 - t);
        }

        static T P0D3(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P5D3(k1 - t);
        }

        static T P0D4(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P5D4(k1 - t);
        }

        static T P0D5(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P5D5(k1 - t);
        }

        static T P1D0(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P4D0(k1 - t);
        }

        static T P1D1(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P4D1(k1 - t);
        }

        static T P1D2(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P4D2(k1 - t);
        }

        static T P1D3(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P4D3(k1 - t);
        }

        static T P1D4(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P4D4(k1 - t);
        }

        static T P1D5(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P4D5(k1 - t);
        }

        static T P2D0(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P3D0(k1 - t);
        }

        static T P2D1(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P3D1(k1 - t);
        }

        static T P2D2(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P3D2(k1 - t);
        }

        static T P2D3(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P3D3(k1 - t);
        }

        static T P2D4(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return +P3D4(k1 - t);
        }

        static T P2D5(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return -P3D4(k1 - t);
        }

        static T P3D0(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return ((k1 - t) * t) * ((k1 - t) * t) * t;
        }

        static T P3D1(T const& t)
        {
            T const k1 = static_cast<T>(1);
            T const k3 = static_cast<T>(3);
            T const k5 = static_cast<T>(5);
            return (k1 - t) * t * t * (k3 - k5 * t);
        }

        static T P3D2(T const& t)
        {
            T const k6 = static_cast<T>(6);
            T const k20 = static_cast<T>(20);
            T const k24 = static_cast<T>(24);
            return t * (k6 + t * (-k24 + k20 * t));
        }

        static T P3D3(T const& t)
        {
            T const k6 = static_cast<T>(6);
            T const k48 = static_cast<T>(48);
            T const k60 = static_cast<T>(60);
            return k6 + t * (-k48 + k60 * t);
        }

        static T P3D4(T const& t)
        {
            T const k48 = static_cast<T>(48);
            T const k120 = static_cast<T>(120);
            return -k48 + k120 * t;
        }

        static T P3D5(T const&)
        {
            T const k120 = static_cast<T>(120);
            return k120;
        }

        static T P4D0(T const& t)
        {
            T const k1 = static_cast<T>(1);
            return (k1 - t) * (t * t) * (t * t);
        }

        static T P4D1(T const& t)
        {
            T const k4 = static_cast<T>(4);
            T const k5 = static_cast<T>(5);
            return t * t * t * (k4 - k5 * t);
        }

        static T P4D2(T const& t)
        {
            T const k12 = static_cast<T>(12);
            T const k20 = static_cast<T>(20);
            return t * t * (k12 - k20 * t);
        }

        static T P4D3(T const& t)
        {
            T const k24 = static_cast<T>(24);
            T const k60 = static_cast<T>(60);
            return t * (k24 - k60 * t);
        }

        static T P4D4(T const& t)
        {
            T const k24 = static_cast<T>(24);
            T const k120 = static_cast<T>(120);
            return k24 - k120 * t;
        }

        static T P4D5(T const&)
        {
            T const k120 = static_cast<T>(120);
            return -k120;
        }

        static T P5D0(T const& t)
        {
            return t * (t * t) * (t * t);
        }

        static T P5D1(T const& t)
        {
            T const k5 = static_cast<T>(5);
            return k5 * (t * t) * (t * t);
        }

        static T P5D2(T const& t)
        {
            T const k20 = static_cast<T>(20);
            return k20 * t * (t * t);
        }

        static T P5D3(T const& t)
        {
            T const k60 = static_cast<T>(60);
            return k60 * t * t;
        }

        static T P5D4(T const& t)
        {
            T const k120 = static_cast<T>(120);
            return k120 * t;
        }

        static T P5D5(T const&)
        {
            T const k120 = static_cast<T>(120);
            return k120;
        }
    };
}
