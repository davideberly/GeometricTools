// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2024.03.13

#pragma once

// The Hermite triquintic polynomial is
//   G(x,y,z) = sum_{i=0}^5 sum_{j=0}^5 sum_{k=0}^5
//              c[i][j][k] * P(i,x) * P(j,y) * P(k,z)
// where P(i,t) = (1-t)^{5-i} * t^i. The domain is (x,y,z) in [0,1]^3.
// Interpolation using these polynomials is described in
// https://www.geometrictools.com/Documentation/SmoothLatticeInterpolation.pdf

#include <Mathematics/HermiteQuintic.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class HermiteTriquintic
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
                Fxx(static_cast<T>(0)), Fxy(static_cast<T>(0)), Fxz(static_cast<T>(0)),
                Fyy(static_cast<T>(0)), Fyz(static_cast<T>(0)), Fzz(static_cast<T>(0)),
                Fxxy(static_cast<T>(0)), Fxxz(static_cast<T>(0)), Fxyy(static_cast<T>(0)),
                Fxyz(static_cast<T>(0)), Fxzz(static_cast<T>(0)), Fyyz(static_cast<T>(0)),
                Fyzz(static_cast<T>(0)), Fxxyy(static_cast<T>(0)), Fxxyz(static_cast<T>(0)),
                Fxxzz(static_cast<T>(0)), Fxyyz(static_cast<T>(0)), Fxyzz(static_cast<T>(0)),
                Fyyzz(static_cast<T>(0)),
                Fxxyyz(static_cast<T>(0)), Fxxyzz(static_cast<T>(0)),
                Fxyyzz(static_cast<T>(0)), Fxxyyzz(static_cast<T>(0))
            {
            }

            Sample(
                T const& f,
                T const& fx, T const& fy, T const& fz,
                T const& fxx, T const& fxy, T const& fxz, T const& fyy, T const& fyz, T const& fzz,
                T const& fxxy, T const& fxxz, T const& fxyy, T const& fxyz, T const& fxzz, T const& fyyz, T const& fyzz,
                T const& fxxyy, T const& fxxyz, T const& fxxzz, T const& fxyyz, T const& fxyzz, T const& fyyzz,
                T const& fxxyyz, T const& fxxyzz, T const& fxyyzz, T const& fxxyyzz)
                :
                F(f),
                Fx(fx), Fy(fy), Fz(fz),
                Fxx(fxx), Fxy(fxy), Fxz(fxz), Fyy(fyy), Fyz(fyz), Fzz(fzz),
                Fxxy(fxxy), Fxxz(fxxz), Fxyy(fxyy), Fxyz(fxyz), Fxzz(fxzz), Fyyz(fyyz), Fyzz(fyzz),
                Fxxyy(fxxyy),
                Fxxyz(fxxyz),
                Fxxzz(fxxzz),
                Fxyyz(fxyyz),
                Fxyzz(fxyzz),
                Fyyzz(fyyzz),
                Fxxyyz(fxxyyz), Fxxyzz(fxxyzz), Fxyyzz(fxyyzz),
                Fxxyyzz(fxxyyzz)
            {
            }

            T F, Fx, Fy, Fz;
            T Fxx, Fxy, Fxz, Fyy, Fyz, Fzz;
            T Fxxy, Fxxz, Fxyy, Fxyz, Fxzz, Fyyz, Fyzz;
            T Fxxyy, Fxxyz, Fxxzz, Fxyyz, Fxyzz, Fyyzz;
            T Fxxyyz, Fxxyzz, Fxyyzz;
            T Fxxyyzz;
        };

        // Create the identically zero polynomial.
        HermiteTriquintic()
            :
            c{}
        {
            T const zero = static_cast<T>(0);
            for (std::size_t i = 0; i < 6; ++i)
            {
                for (std::size_t j = 0; j < 6; ++j)
                {
                    for (std::size_t k = 0; k < 6; ++k)
                    {
                        c[i][j][k] = zero;
                    }
                }
            }
        }

        HermiteTriquintic(std::array<std::array<std::array<Sample, 2>, 2>, 2> const& blocks)
            :
            c{}
        {
            Generate(blocks);
        }

        // Evaluate the polynomial with the specified orders. The returned
        // value is zero if xOrder >= 6 or yOrder >= 6 or zOrder >= 6.
        // Otherwise, some examples are the following where 'hermite' is of
        // type IntpHermiteTriquintic.
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
        T operator()(std::size_t xOrder, std::size_t yOrder, std::size_t zOrder,
            T const& x, T const& y, T const& z) const
        {
            T const zero = static_cast<T>(0);
            if (xOrder <= 5 && yOrder <= 5 && zOrder <= 5)
            {
                T result = zero;
                for (std::size_t i = 0; i < 6; ++i)
                {
                    T xValue = HermiteQuintic<T>::P(i, xOrder, x);
                    for (std::size_t j = 0; j < 6; ++j)
                    {
                        T yValue = HermiteQuintic<T>::P(j, yOrder, y);
                        T xyValue = xValue * yValue;
                        for (std::size_t k = 0; k < 6; ++k)
                        {
                            T zValue = HermiteQuintic<T>::P(k, zOrder, z);
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

                    for (std::int32_t b2 = 0; b2 <= 1; ++b2)
                    {
                        std::int32_t z2 = 5 * b2 + 0;
                        std::int32_t p2 = 3 * b2 + 1;
                        std::int32_t q2 = 1 * b2 + 2;
                        T s2 = static_cast<T>(1 - 2 * b2);
                        T s0s2 = s0 * s2;
                        T s1s2 = s1 * s2;
                        T s0s1s2 = s0 * s1s2;

                        Sample const& b = blocks[b0][b1][b2];
                        Sample input(
                            b.F,
                            s0 * b.Fx,
                            s1 * b.Fy,
                            s2 * b.Fz,
                            b.Fxx,
                            s0s1 * b.Fxy,
                            s0s2 * b.Fxz,
                            b.Fyy,
                            s1s2 * b.Fyz,
                            b.Fzz,
                            s1 * b.Fxxy,
                            s2 * b.Fxxz,
                            s0 * b.Fxyy,
                            s0s1s2 * b.Fxyz,
                            s0 * b.Fxzz,
                            s2 * b.Fyyz,
                            s1 * b.Fyzz,
                            b.Fxxyy,
                            s1s2 * b.Fxxyz,
                            b.Fxxzz,
                            s0s2 * b.Fxyyz,
                            s0s1 * b.Fxyzz,
                            b.Fyyzz,
                            s2 * b.Fxxyyz,
                            s1 * b.Fxxyzz,
                            s0 * b.Fxyyzz,
                            b.Fxxyyzz);

                        GenerateSingle(input,
                            c[z0][z1][z2], c[p0][z1][z2], c[z0][p1][z2], c[z0][z1][p2],
                            c[q0][z1][z2], c[p0][p1][z2], c[p0][z1][p2], c[z0][q1][z2],
                            c[z0][p1][p2], c[z0][z1][q2], c[q0][p1][z2], c[q0][z1][p2],
                            c[p0][q1][z2], c[p0][p1][p2], c[p0][z1][q2], c[z0][q1][p2],
                            c[z0][p1][q2], c[q0][q1][z2], c[q0][p1][p2], c[q0][z1][q2],
                            c[p0][q1][p2], c[p0][p1][q2], c[z0][q1][q2], c[q0][q1][p2],
                            c[q0][p1][q2], c[p0][q1][q2], c[q0][q1][q2]);
                    }
                }
            }
        }

        // Set the coefficients manually as desired. For Hermite quintic
        // interpolation on a lattice, use Generate(...). The lattice
        // interpolator is globally C2-continuous.
        std::array<std::array<std::array<T, 6>, 6>, 6> c;

    private:
        void GenerateSingle(Sample const& input, T& v000, T& v100, T& v010, T& v001, T& v200, T& v110,
            T& v101, T& v020, T& v011, T& v002, T& v210, T& v201, T& v120, T& v111, T& v102,
            T& v021, T& v012, T& v220, T& v211, T& v202, T& v121, T& v112, T& v022, T& v221,
            T& v212, T& v122, T& v222)
        {
            T const k2 = static_cast<T>(2);
            T const k4 = static_cast<T>(4);
            T const k5 = static_cast<T>(5);
            T const k8 = static_cast<T>(8);
            T const k10 = static_cast<T>(10);
            T const k16 = static_cast<T>(16);
            T const k20 = static_cast<T>(20);
            T const k25 = static_cast<T>(25);
            T const k40 = static_cast<T>(40);
            T const k50 = static_cast<T>(50);
            T const k64 = static_cast<T>(64);
            T const k80 = static_cast<T>(80);
            T const k100 = static_cast<T>(100);
            T const k125 = static_cast<T>(125);
            T const k160 = static_cast<T>(160);
            T const k200 = static_cast<T>(200);
            T const k250 = static_cast<T>(250);
            T const k500 = static_cast<T>(500);
            T const k400 = static_cast<T>(400);
            T const k1000 = static_cast<T>(1000);
            T const k1Div8 = static_cast<T>(0.125);
            T const k1Div4 = static_cast<T>(0.25);
            T const k1Div2 = static_cast<T>(0.5);
            T const k5Div4 = static_cast<T>(1.25);
            T const k5Div2 = static_cast<T>(2.5);
            T const k25Div2 = static_cast<T>(12.5);
            T const& F = input.F;
            T const& Fx = input.Fx;
            T const& Fy = input.Fy;
            T const& Fz = input.Fz;
            T const& Fxx = input.Fxx;
            T const& Fxy = input.Fxy;
            T const& Fxz = input.Fxz;
            T const& Fyy = input.Fyy;
            T const& Fyz = input.Fyz;
            T const& Fzz = input.Fzz;
            T const& Fxxy = input.Fxxy;
            T const& Fxxz = input.Fxxz;
            T const& Fxyy = input.Fxyy;
            T const& Fxyz = input.Fxyz;
            T const& Fxzz = input.Fxzz;
            T const& Fyyz = input.Fyyz;
            T const& Fyzz = input.Fyzz;
            T const& Fxxyy = input.Fxxyy;
            T const& Fxxyz = input.Fxxyz;
            T const& Fxxzz = input.Fxxzz;
            T const& Fxyyz = input.Fxyyz;
            T const& Fxyzz = input.Fxyzz;
            T const& Fyyzz = input.Fyyzz;
            T const& Fxxyyz = input.Fxxyyz;
            T const& Fxxyzz = input.Fxxyzz;
            T const& Fxyyzz = input.Fxyyzz;
            T const& Fxxyyzz = input.Fxxyyzz;

            v000 = F;

            v100 = k5 * F + Fx;
            v010 = k5 * F + Fy;
            v001 = k5 * F + Fz;

            v200 = k10 * F + k4 * Fx + k1Div2 * Fxx;
            v110 = k25 * F + k5 * Fx + k5 * Fy + Fxy;
            v101 = k25 * F + k5 * Fx + k5 * Fz + Fxz;
            v020 = k10 * F + k4 * Fy + k1Div2 * Fyy;
            v011 = k25 * F + k5 * Fy + k5 * Fz + Fyz;
            v002 = k10 * F + k4 * Fz + k1Div2 * Fzz;

            v210 = k50 * F + k20 * Fx + k10 * Fy + k5Div2 * Fxx + k4 * Fxy + k1Div2 * Fxxy;
            v201 = k50 * F + k20 * Fx + k10 * Fz + k5Div2 * Fxx + k4 * Fxz + k1Div2 * Fxxz;
            v120 = k50 * F + k10 * Fx + k20 * Fy + k4 * Fxy + k5Div2 * Fyy + k1Div2 * Fxyy;
            v111 = k125 * F + k25 * Fx + k25 * Fy + k25 * Fz + k5 * Fxy + k5 * Fxz + k5 * Fyz + Fxyz;
            v102 = k50 * F + k10 * Fx + k20 * Fz + k4 * Fxz + k5Div2 * Fzz + k1Div2 * Fxzz;
            v021 = k50 * F + k20 * Fy + k10 * Fz + k5Div2 * Fyy + k4 * Fyz + k1Div2 * Fyyz;
            v012 = k50 * F + k10 * Fy + k20 * Fz + k4 * Fyz + k5Div2 * Fzz + k1Div2 * Fyzz;

            v220 = k100 * F + k40 * Fx + k40 * Fy + k5 * Fxx + k16 * Fxy + k5 * Fyy + k2 * Fxxy + k2 * Fxyy + k1Div4 * Fxxyy;
            v211 = k250 * F + k100 * Fx + k50 * Fy + k50 * Fz + k25Div2 * Fxx + k20 * Fxy + k20 * Fxz + k10 * Fyz + k5Div2 * Fxxy + k5Div2 * Fxxz + k4 * Fxyz + k1Div2 * Fxxyz;
            v202 = k100 * F + k40 * Fx + k40 * Fz + k5 * Fxx + k16 * Fxz + k5 * Fzz + k2 * Fxxz + k2 * Fxzz + k1Div4 * Fxxzz;
            v121 = k250 * F + k50 * Fx + k100 * Fy + k50 * Fz + k20 * Fxy + k10 * Fxz + k25Div2 * Fyy + k20 * Fyz + k5Div2 * Fxyy + k4 * Fxyz + k5Div2 * Fyyz + k1Div2 * Fxyyz;
            v112 = k250 * F + k50 * Fx + k50 * Fy + k100 * Fz + k10 * Fxy + k20 * Fxz + k20 * Fyz + k25Div2 * Fzz + k4 * Fxyz + k5Div2 * Fxzz + k5Div2 * Fyzz + k1Div2 * Fxyzz;
            v022 = k100 * F + k40 * Fy + k40 * Fz + k5 * Fyy + k16 * Fyz + k5 * Fzz + k2 * Fyyz + k2 * Fyzz + k1Div4 * Fyyzz;

            v221 = k500 * F + k200 * Fx + k200 * Fy + k100 * Fz + k25 * Fxx + k80 * Fxy + k40 * Fxz + k25 * Fyy + k40 * Fyz + k10 * Fxxy + k5 * Fxxz + k10 * Fxyy + k16 * Fxyz + k5 * Fyyz + k5Div4 * Fxxyy + k2 * Fxxyz + k2 * Fxyyz + k1Div4 * Fxxyyz;
            v212 = k500 * F + k200 * Fx + k100 * Fy + k200 * Fz + k25 * Fxx + k40 * Fxy + k80 * Fxz + k40 * Fyz + k25 * Fzz + k5 * Fxxy + k10 * Fxxz + k16 * Fxyz + k10 * Fxzz + k5 * Fyzz + k2 * Fxxyz + k5Div4 * Fxxzz + k2 * Fxyzz + k1Div4 * Fxxyzz;
            v122 = k500 * F + k100 * Fx + k200 * Fy + k200 * Fz + k40 * Fxy + k40 * Fxz + k25 * Fyy + k80 * Fyz + k25 * Fzz + k5 * Fxyy + k16 * Fxyz + k5 * Fxzz + k10 * Fyyz + k10 * Fyzz + k2 * Fxyyz + k2 * Fxyzz + k5Div4 * Fyyzz + k1Div4 * Fxyyzz;

            v222 = k1000 * F + k400 * Fx + k400 * Fy + k400 * Fz + k50 * Fxx + k160 * Fxy + k160 * Fxz + k50 * Fyy + k160 * Fyz + k50 * Fzz + k20 * Fxxy + k20 * Fxxz + k20 * Fxyy + k64 * Fxyz + k20 * Fxzz + k20 * Fyyz + k20 * Fyzz + k5Div2 * Fxxyy + k8 * Fxxyz + k5Div2 * Fxxzz + k8 * Fxyyz + k8 * Fxyzz + k5Div2 * Fyyzz + Fxxyyz + Fxxyzz + Fxyyzz + k1Div8 * Fxxyyzz;
        }
    };
}
