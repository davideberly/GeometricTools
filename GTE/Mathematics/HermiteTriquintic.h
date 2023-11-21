// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.11.20

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
        // Create the identically zero polynomial.
        HermiteTriquintic()
            :
            c{}
        {
            for (size_t i = 0; i < 6; ++i)
            {
                for (size_t j = 0; j < 6; ++j)
                {
                    for (size_t k = 0; k < 6; ++k)
                    {
                        c[i][j][k] = static_cast<T>(0);
                    }
                }
            }
        }

        // Evaluate the polynomial with the specified orders. The returned
        // value is zero if xOrder >= 6 or yOrder >= 6 or zOrder >= 6.
        // Otherwise, some examples are the following where 'hermite' is of
        // type HermiteTriquintic.
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
            if (xOrder <= 5 && yOrder <= 5)
            {
                T result = static_cast<T>(0);
                for (size_t i = 0; i < 6; ++i)
                {
                    for (size_t j = 0; j < 6; ++j)
                    {
                        for (size_t k = 0; k < 4; ++k)
                        {
                            result += c[i][j][k] *
                                HermiteQuintic<T>::P(i, xOrder, x) *
                                HermiteQuintic<T>::P(j, yOrder, y) *
                                HermiteQuintic<T>::P(k, zOrder, z);
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
                Fxx(k0), Fyy(k0), Fzz(k0), Fxy(k0), Fxz(k0), Fyz(k0),
                Fxxy(k0), Fxxz(k0), Fyyz(k0), Fxyy(k0), Fxzz(k0), Fyzz(k0), Fxyz(k0),
                Fxxyy(k0), Fxxzz(k0), Fyyzz(k0), Fxxyz(k0), Fxyyz(k0), Fxyzz(k0),
                Fxxyyz(k0), Fxxyzz(k0), Fxyyzz(k0),
                Fxxyyzz(k0)
            {
            }

            Sample(
                T const& f,
                T const& fx, T const& fy, T const& fz,
                T const& fxx, T const& fyy, T const& fzz, T const& fxy, T const& fxz, T const& fyz,
                T const& fxxy, T const& fxxz, T const& fyyz, T const& fxyy, T const& fxzz, T const& fyzz, T const& fxyz,
                T const& fxxyy, T const& fxxzz, T const& fyyzz, T const& fxxyz, T const& fxyyz, T const& fxyzz,
                T const& fxxyyz, T const& fxxyzz, T const& fxyyzz, T const& fxxyyzz)
                :
                F(f),
                Fx(fx), Fy(fy), Fz(fz),
                Fxx(fxx), Fyy(fyy), Fzz(fzz), Fxy(fxy), Fxz(fxz), Fyz(fyz),
                Fxxy(fxxy), Fxxz(fxxz), Fyyz(fyyz), Fxyy(fxyy), Fxzz(fxzz), Fyzz(fyzz), Fxyz(fxyz),
                Fxxyy(fxxyy), Fxxzz(fxxzz), Fyyzz(fyyzz), Fxxyz(fxxyz), Fxyyz(fxyyz), Fxyzz(fxyzz),
                Fxxyyz(fxxyyz), Fxxyzz(fxxyzz), Fxyyzz(fxyyzz),
                Fxxyyzz(fxxyyzz)
            {
            }

            T F, Fx, Fy, Fz;
            T Fxx, Fyy, Fzz, Fxy, Fxz, Fyz;
            T Fxxy, Fxxz, Fyyz, Fxyy, Fxzz, Fyzz, Fxyz;
            T Fxxyy, Fxxzz, Fyyzz, Fxxyz, Fxyyz, Fxyzz;
            T Fxxyyz, Fxxyzz, Fxyyzz;
            T Fxxyyzz;
        };

        // Use this function for Hermite triquintic interpolation on a lattice.
        // Generate the 4x4x4 coefficients c[][] for a cell of the lattice
        // with pixels at (x,y,z), (x+1,y,z), (x,y+1,z), (x+1,y+1,z),
        // (x,y,z+1), (x+1,y,z+1), (x,y+1,z+1), and (x+1,y+1,z+1). The caller
        // is responsible for tracking the pixel (x,y) that is associated with
        // the coefficients.
        void Generate(std::array<std::array<std::array<Sample, 2>, 2>, 2> const& blocks)
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

                    for (int32_t b2 = 0; b2 <= 1; ++b2)
                    {
                        int32_t z2 = 5 * b2 + 0;
                        int32_t p2 = 3 * b2 + 1;
                        int32_t q2 = 1 * b2 + 2;
                        T s2 = static_cast<T>(1 - 2 * b2);
                        T s0s2 = s0 * s2;
                        T s1s2 = s1 * s2;
                        T s0s1s2 = s0 * s1s2;

                        Sample const& b = blocks[b0][b1][b2];
                        Sample input(b.F, s0 * b.Fx, s1 * b.Fy, s2 * b.Fz, b.Fxx, b.Fyy, b.Fzz,
                            s0s1 * b.Fxy, s0s2 * b.Fxz, s1s2 * b.Fyz, s1 * b.Fxxy,
                            s2 * b.Fxxz, s2 * b.Fyyz, s0 * b.Fxyy, s0 * b.Fxzz, s1 * b.Fyzz,
                            s0s1s2 * b.Fxyz, b.Fxxyy, b.Fxxzz, b.Fyyzz, s1s2 * b.Fxxyz,
                            s0s2 * b.Fxyyz, s0s1 * b.Fxyzz, s2 * b.Fxxyyz, s1 * b.Fxxyzz,
                            s0 * b.Fxyyzz, b.Fxxyyzz);

                        Generate(input, c[z0][z1][z2], c[p0][z1][z2], c[z0][p1][z2],
                            c[z0][z1][p2], c[q0][z1][z2], c[z0][q1][z2], c[z0][z1][q2],
                            c[p0][p1][z2], c[p0][z1][p2], c[z0][p1][p2], c[q0][p1][z2],
                            c[q0][z1][p2], c[z0][q1][p2], c[p0][q1][z2], c[p0][z1][q2],
                            c[z0][p1][q2], c[p0][p1][p2], c[q0][q1][z2], c[q0][z1][q2],
                            c[z0][q1][q2], c[q0][p1][p2], c[p0][q1][p2], c[p0][p1][q2],
                            c[q0][q1][p2], c[q0][p1][q2], c[p0][q1][q2], c[q0][q1][q2]);
                    }
                }
            }
        }

        // Set the coefficients manually as desired. For Hermite triquintic
        // interpolation on a lattice, use Generate(...). The lattice
        // interpolator is globally C2-continuous.
        std::array<std::array<std::array<T, 6>, 6>, 6> c;

    private:
        static T constexpr k2 = static_cast<T>(2);
        static T constexpr k4 = static_cast<T>(4);
        static T constexpr k5 = static_cast<T>(5);
        static T constexpr k8 = static_cast<T>(8);
        static T constexpr k10 = static_cast<T>(10);
        static T constexpr k16 = static_cast<T>(16);
        static T constexpr k20 = static_cast<T>(20);
        static T constexpr k25 = static_cast<T>(25);
        static T constexpr k40 = static_cast<T>(40);
        static T constexpr k50 = static_cast<T>(50);
        static T constexpr k64 = static_cast<T>(64);
        static T constexpr k100 = static_cast<T>(100);
        static T constexpr k125 = static_cast<T>(125);
        static T constexpr k160 = static_cast<T>(160);
        static T constexpr k250 = static_cast<T>(250);
        static T constexpr k500 = static_cast<T>(500);
        static T constexpr k400 = static_cast<T>(400);
        static T constexpr k1000 = static_cast<T>(1000);

        void Generate(Sample const& input, T& v000, T& v100, T& v010, T& v001, T& v200, T& v020,
            T& v002, T& v110, T& v101, T& v011, T& v210, T& v201, T& v021, T& v120, T& v102,
            T& v012, T& v111, T& v220, T& v202, T& v022, T& v211, T& v121, T& v112, T& v221,
            T& v212, T& v122, T& v222)
        {
            v000 = input.F;
            v100 = k5 * v000 + input.Fx;
            v010 = k5 * v000 + input.Fy;
            v001 = k5 * v000 + input.Fz;
            v200 = -k10 * v000 + k4 * v100 + input.Fxx / k2;
            v020 = -k10 * v000 + k4 * v010 + input.Fyy / k2;
            v002 = -k10 * v000 + k4 * v001 + input.Fzz / k2;
            v110 = -k25 * v000 + k5 * (v100 + v010) + input.Fxy;
            v101 = -k25 * v000 + k5 * (v100 + v001) + input.Fxz;
            v011 = -k25 * v000 + k5 * (v010 + v001) + input.Fyz;
            v210 = k50 * v000 - k10 * (k2 * v100 + v010) + k5 * v200 + k4 * v110 + input.Fxxy / k2;
            v201 = k50 * v000 - k10 * (k2 * v100 + v001) + k5 * v200 + k4 * v101 + input.Fxxz / k2;
            v021 = k50 * v000 - k10 * (k2 * v010 + v001) + k5 * v020 + k4 * v011 + input.Fyyz / k2;
            v120 = k50 * v000 - k10 * (v100 + k2 * v010) + k5 * v020 + k4 * v110 + input.Fxyy / k2;
            v102 = k50 * v000 - k10 * (v100 + k2 * v001) + k5 * v002 + k4 * v101 + input.Fxzz / k2;
            v012 = k50 * v000 - k10 * (v010 + k2 * v001) + k5 * v002 + k4 * v011 + input.Fyzz / k2;
            v111 = k125 * v000 - k25 * (v001 + v010 + v100) + k5 * (v011 + v101 + v110) + input.Fxyz;
            v220 = -k100 * v000 + k40 * (v100 + v010) - k10 * (v200 + v020) - k16 * v110 + k4 * (v210 + v120) + input.Fxxyy / k4;
            v202 = -k100 * v000 + k40 * (v100 + v001) - k10 * (v002 + v200) - k16 * v101 + k4 * (v201 + v102) + input.Fxxzz / k4;
            v022 = -k100 * v000 + k40 * (v010 + v001) - k10 * (v002 + v020) - k16 * v011 + k4 * (v021 + v012) + input.Fyyzz / k4;
            v211 = -k250 * v000 + k50 * (k2 * v100 + v010 + v001) - k10 * (k2 * (v110 + v101) + v011) - k25 * v200
                + k5 * (v210 + v201) + k4 * v111 + input.Fxxyz / k2;
            v121 = -k250 * v000 + k50 * (v100 + k2 * v010 + v001) - k10 * (k2 * (v110 + v011) + v101) - k25 * v020
                + k5 * (v120 + v021) + k4 * v111 + input.Fxyyz / k2;
            v112 = -k250 * v000 + k50 * (v100 + v010 + k2 * v001) - k10 * (k2 * (v011 + v101) + v110) - k25 * v002
                + k5 * (v012 + v102) + k4 * v111 + input.Fxyzz / k2;
            v221 = k500 * v000 - k100 * (v001 + k2 * (v010 + v100)) + k50 * (v020 + v200) + k40 * (v011 + k2 * v110 + v101)
                - k10 * (v021 + v201) - k20 * (v120 + v210) - k16 * v111 + k5 * v220 + k4 * (v121 + v211) + input.Fxxyyz / k4;
            v212 = k500 * v000 - k100 * (v010 + k2 * (v100 + v001)) + k50 * (v002 + v200) + k40 * (v011 + k2 * v101 + v110)
                - k10 * (v012 + v210) - k20 * (v102 + v201) - k16 * v111 + k5 * v202 + k4 * (v112 + v211) + input.Fxxyzz / k4;
            v122 = k500 * v000 - k100 * (v100 + k2 * (v001 + v010)) + k50 * (v002 + v020) + k40 * (v101 + k2 * v011 + v110)
                - k10 * (v102 + v120) - k20 * (v012 + v021) - k16 * v111 + k5 * v022 + k4 * (v112 + v121) + input.Fxyyzz / k4;
            v222 = -k1000 * v000 + k400 * (v001 + v100 + v010) - k100 * (v002 + v020 + v200) - k160 * (v011 + v101 + v110)
                + k40 * (v012 + v021 + v102 + v120 + v201 + v210) + k64 * v111 - k10 * (v022 + v202 + v220)
                - k16 * (v112 + v121 + v211) + k4 * (v122 + v212 + v221) + input.Fxxyyzz / k8;
        }
    };
}
