// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// The algorithm for representing a circle as a NURBS curve or a sphere as a
// NURBS surface is described in
//   https://www.geometrictools.com/Documentation/NURBSCircleSphere.pdf
// The implementations are related to the documents as shown next.
//   NURBSEighthSphereDegree4 implements Section 3.1.2 (triangular domain)
//   NURBSHalfSphereDegree3 implements Section 3.2 (rectangular domain)
//   NURBSFullSphereDegree3 implements Section 2.3 (rectangular domain)
// TODO: The class NURBSSurface currently assumes a rectangular domain.
// Once support is added for triangular domains, make that new class a
// base class of the sphere-representing NURBS. This will allow sharing
// of the NURBS basis functions and evaluation framework.

#include <GTL/Mathematics/Surfaces/NURBSSurface.h>
#include <functional>

namespace gtl
{
    // TODO: Why is this not derived from NURBSSurface<T, 3>?
    template <typename T>
    class NURBSEighthSphereDegree4
    {
    public:
        // Construction. The eigth sphere is x^2 + y^2 + z^2 = 1 for x >= 0,
        // y >= 0 and z >= 0.
        NURBSEighthSphereDegree4()
        {
            T const a0 = (C_SQRT_3<T> - C_<T>(1)) / C_SQRT_3<T>;
            T const a1 = (C_SQRT_3<T> + C_<T>(1)) / (C_<T>(2) * C_SQRT_3<T>);
            T const a2 = C_<T>(1) - (C_<T>(5) - C_SQRT_2<T>) * (C_<T>(7) - C_SQRT_3<T>) / C_<T>(46);
            T const b0 = C_<T>(4) * C_SQRT_3<T> * (C_SQRT_3<T> - C_<T>(1));
            T const b1 = C_<T>(3) * C_SQRT_2<T>;
            T const b2 = C_<T>(4);
            T const b3 = C_SQRT_2<T> * (C_<T>(3) + C_<T>(2) * C_SQRT_2<T> -C_SQRT_3<T>) / C_SQRT_3<T>;

            mControls[0][0] = { C_<T>(0), C_<T>(0), C_<T>(1) }; // P004
            mControls[0][1] = { C_<T>(0), a0,   C_<T>(1) };     // P013
            mControls[0][2] = { C_<T>(0), a1,   a1 };           // P022
            mControls[0][3] = { C_<T>(0), C_<T>(1), a0 };       // P031
            mControls[0][4] = { C_<T>(0), C_<T>(1), C_<T>(0) }; // P040

            mControls[1][0] = { a0,   C_<T>(0), C_<T>(1) };     // P103
            mControls[1][1] = { a2,   a2,   C_<T>(1) };         // P112
            mControls[1][2] = { a2,   C_<T>(1), a2 };           // P121
            mControls[1][3] = { a0,   C_<T>(1), C_<T>(0) };     // P130
            mControls[1][4] = { C_<T>(0), C_<T>(0), C_<T>(0) }; // unused

            mControls[2][0] = { a1,   C_<T>(0), a1 };           // P202
            mControls[2][1] = { C_<T>(1), a2,   a2 };           // P211
            mControls[2][2] = { a1,   a1,   C_<T>(0) };         // P220
            mControls[2][3] = { C_<T>(0), C_<T>(0), C_<T>(0) }; // unused
            mControls[2][4] = { C_<T>(0), C_<T>(0), C_<T>(0) }; // unused

            mControls[3][0] = { C_<T>(1), C_<T>(0), a0 };       // P301
            mControls[3][1] = { C_<T>(1), a0,   C_<T>(0) };     // P310
            mControls[3][2] = { C_<T>(0), C_<T>(0), C_<T>(0) }; // unused
            mControls[3][3] = { C_<T>(0), C_<T>(0), C_<T>(0) }; // unused
            mControls[3][4] = { C_<T>(0), C_<T>(0), C_<T>(0) }; // unused

            mControls[4][0] = { C_<T>(1), C_<T>(0), C_<T>(0) }; // P400
            mControls[4][1] = { C_<T>(0), C_<T>(0), C_<T>(0) }; // unused
            mControls[4][2] = { C_<T>(0), C_<T>(0), C_<T>(0) }; // unused
            mControls[4][3] = { C_<T>(0), C_<T>(0), C_<T>(0) }; // unused
            mControls[4][4] = { C_<T>(0), C_<T>(0), C_<T>(0) }; // unused

            mWeights[0][0] = b0;        // w004
            mWeights[0][1] = b1;        // w013
            mWeights[0][2] = b2;        // w022
            mWeights[0][3] = b1;        // w031
            mWeights[0][4] = b0;        // w040

            mWeights[1][0] = b1;        // w103
            mWeights[1][1] = b3;        // w112
            mWeights[1][2] = b3;        // w121
            mWeights[1][3] = b1;        // w130
            mWeights[1][4] = C_<T>(0);  // unused

            mWeights[2][0] = b2;        // w202
            mWeights[2][1] = b3;        // w211
            mWeights[2][2] = b2;        // w220
            mWeights[2][3] = C_<T>(0);  // unused
            mWeights[2][4] = C_<T>(0);  // unused

            mWeights[3][0] = b1;        // w301
            mWeights[3][1] = b1;        // w310
            mWeights[3][2] = C_<T>(0);  // unused
            mWeights[3][3] = C_<T>(0);  // unused
            mWeights[3][4] = C_<T>(0);  // unused

            mWeights[4][0] = b0;        // w400
            mWeights[4][1] = C_<T>(0);  // unused
            mWeights[4][2] = C_<T>(0);  // unused
            mWeights[4][3] = C_<T>(0);  // unused
            mWeights[4][4] = C_<T>(0);  // unused
        }

        // Evaluation of the surface. It is required that order <= 2, which
        // allows computing derivatives through order 2. If you want only the
        // position, pass in order of 0. If you want the position and first
        // derivatives, pass in order of 1, and so on. The output array 'jet'
        // must have enough storage to support the specified order. The values
        // are ordered as:
        //   jet[0] contains position X
        //   jet[1] contains first-order derivative dX/du
        //   jet[2] contains first-order derivative dX/dv
        //   jet[3] contains second-order derivative d2X/du2
        //   jet[4] contains second-order derivative d2X/dudv
        //   jet[5] contains second-order derivative d2X/dv2
        void Evaluate(T const& u, T const& v, size_t order, Vector3<T>* jet) const
        {
            // TODO: Some of the polynomials are used in other polynomials.
            // Optimize the code by eliminating the redundant computations.

            T w = C_<T>(1) - u - v;
            T uu = u * u, uv = u * v, uw = u * w, vv = v * v, vw = v * w, ww = w * w;

            // Compute the order-0 polynomials. Only the elements to be used
            // are filled in. The other terms are uninitialized but never
            // used.
            std::array<std::array<T, 5>, 5> B;
            B[0][0] = ww * ww;
            B[0][1] = C_<T>(4) * vw * ww;
            B[0][2] = C_<T>(6) * vv * ww;
            B[0][3] = C_<T>(4) * vv * vw;
            B[0][4] = vv * vv;
            B[1][0] = C_<T>(4) * uw * ww;
            B[1][1] = C_<T>(12) * uv * ww;
            B[1][2] = C_<T>(12) * uv * vw;
            B[1][3] = C_<T>(4) * uv * vv;
            B[2][0] = C_<T>(6) * uu * ww;
            B[2][1] = C_<T>(12) * uu * vw;
            B[2][2] = C_<T>(6) * uu * vv;
            B[3][0] = C_<T>(4) * uu * uw;
            B[3][1] = C_<T>(4) * uu * uv;
            B[4][0] = uu * uu;

            // Compute the NURBS position.
            Vector3<T> N{};
            T D = C_<T>(0);
            for (size_t j1 = 0; j1 <= 4; ++j1)
            {
                for (size_t j0 = 0; j0 <= 4 - j1; ++j0)
                {
                    T product = mWeights[j1][j0] * B[j1][j0];
                    N += product * mControls[j1][j0];
                    D += product;
                }
            }
            jet[0] = N / D;

            if (order >= 1)
            {
                // Compute the order-1 polynomials. Only the elements to be
                // used are filled in. The other terms are uninitialized but
                // never used.
                T WmU = w - u;
                T WmTwoU = WmU - u;
                T WmThreeU = WmTwoU - u;
                T TwoWmU = w + WmU;
                T ThreeWmU = w + TwoWmU;
                T WmV = w - v;
                T WmTwoV = WmV - v;
                T WmThreeV = WmTwoV - v;
                T TwoWmV = w + WmV;
                T ThreeWmV = w + TwoWmV;
                T Dsqr = D * D;

                std::array<std::array<T, 5>, 5> Bu;
                Bu[0][0] = -C_<T>(4) * ww * w;
                Bu[0][1] = -C_<T>(12) * v * ww;
                Bu[0][2] = -C_<T>(12) * vv * w;
                Bu[0][3] = -C_<T>(4) * v * vv;
                Bu[0][4] = C_<T>(0);
                Bu[1][0] = C_<T>(4) * ww * WmThreeU;
                Bu[1][1] = C_<T>(12) * vw * WmTwoU;
                Bu[1][2] = C_<T>(12) * vv * WmU;
                Bu[1][3] = C_<T>(4) * vv;
                Bu[2][0] = C_<T>(12) * uw * WmU;
                Bu[2][1] = C_<T>(12) * uv * TwoWmU;
                Bu[2][2] = C_<T>(12) * u * vv;
                Bu[3][0] = C_<T>(4) * uu * ThreeWmU;
                Bu[3][1] = C_<T>(12) * uu * v;
                Bu[4][0] = C_<T>(4) * uu * u;

                std::array<std::array<T, 5>, 5> Bv;
                Bv[0][0] = -C_<T>(4) * ww * w;
                Bv[0][1] = C_<T>(4) * ww * WmThreeV;
                Bv[0][2] = C_<T>(12) * vw * WmV;
                Bv[0][3] = C_<T>(4) * vv * ThreeWmV;
                Bv[0][4] = C_<T>(4) * vv * v;
                Bv[1][0] = -C_<T>(12) * u * ww;
                Bv[1][1] = C_<T>(12) * uw * WmTwoV;
                Bv[1][2] = C_<T>(12) * uv * TwoWmV;
                Bv[1][3] = C_<T>(12) * u * vv;
                Bv[2][0] = -C_<T>(12) * uu * w;
                Bv[2][1] = C_<T>(12) * uu * WmV;
                Bv[2][2] = C_<T>(12) * uu * v;
                Bv[3][0] = -C_<T>(4) * uu * u;
                Bv[3][1] = C_<T>(4) * uu * u;
                Bv[4][0] = C_<T>(0);

                Vector3<T> Nu{}, Nv{};
                T Du = C_<T>(0), Dv = C_<T>(0);
                for (size_t j1 = 0; j1 <= 4; ++j1)
                {
                    for (size_t j0 = 0; j0 <= 4 - j1; ++j0)
                    {
                        T product = mWeights[j1][j0] * Bu[j1][j0];
                        Nu += product * mControls[j1][j0];
                        Du += product;
                        product = mWeights[j1][j0] * Bv[j1][j0];
                        Nv += product * mControls[j1][j0];
                        Dv += product;
                    }
                }
                Vector3<T> numerDU = D * Nu - Du * N;
                Vector3<T> numerDV = D * Nv - Dv * N;
                jet[1] = numerDU / Dsqr;
                jet[2] = numerDV / Dsqr;

                if (order >= 2)
                {
                    // Compute the order-2 polynomials. Only the elements to
                    // be used are filled in. The other terms are
                    // uninitialized but never used.
                    T Dcub = Dsqr * D;

                    std::array<std::array<T, 5>, 5> Buu;
                    Buu[0][0] = C_<T>(12) * ww;
                    Buu[0][1] = C_<T>(24) * vw;
                    Buu[0][2] = C_<T>(12) * vv;
                    Buu[0][3] = C_<T>(0);
                    Buu[0][4] = C_<T>(0);
                    Buu[1][0] = -C_<T>(24) * w * WmU;
                    Buu[1][1] = -C_<T>(24) * v * TwoWmU;
                    Buu[1][2] = -C_<T>(24) * vv;
                    Buu[1][3] = C_<T>(0);
                    Buu[2][0] = C_<T>(12) * (ww - C_<T>(4) * uw + uu);
                    Buu[2][1] = C_<T>(24) * v * WmTwoU;
                    Buu[2][2] = C_<T>(12) * vv;
                    Buu[3][0] = C_<T>(24) * u * WmU;
                    Buu[3][1] = C_<T>(24) * uv;
                    Buu[4][0] = C_<T>(12) * uu;

                    std::array<std::array<T, 5>, 5> Buv;
                    Buv[0][0] = C_<T>(12) * ww;
                    Buv[0][1] = -C_<T>(12) * w * WmTwoV;
                    Buv[0][2] = -C_<T>(12) * v * TwoWmV;
                    Buv[0][3] = -C_<T>(12) * vv;
                    Buv[0][4] = C_<T>(0);
                    Buv[1][0] = -C_<T>(12) * w * WmTwoU;
                    Buv[1][1] = C_<T>(12) * (ww + C_<T>(2) * (uv - uw - vw));
                    Buv[1][2] = C_<T>(12) * v * (C_<T>(2) * WmU - v);
                    Buv[1][3] = C_<T>(12) * vv;
                    Buv[2][0] = -C_<T>(12) * u * TwoWmU;
                    Buv[2][1] = C_<T>(12) * u * (C_<T>(2) * WmV - u);
                    Buv[2][2] = C_<T>(24) * uv;
                    Buv[3][0] = -C_<T>(12) * uu;
                    Buv[3][1] = C_<T>(12) * uu;
                    Buv[4][0] = C_<T>(0);

                    std::array<std::array<T, 5>, 5> Bvv;
                    Bvv[0][0] = C_<T>(12) * ww;
                    Bvv[0][1] = -C_<T>(24) * w * WmV;
                    Bvv[0][2] = C_<T>(12) * (ww - C_<T>(4) * vw + vv);
                    Bvv[0][3] = C_<T>(24) * v * WmV;
                    Bvv[0][4] = C_<T>(12) * vv;
                    Bvv[1][0] = C_<T>(24) * uw;
                    Bvv[1][1] = -C_<T>(24) * u * TwoWmV;
                    Bvv[1][2] = C_<T>(24) * u * WmTwoV;
                    Bvv[1][3] = C_<T>(24) * uv;
                    Bvv[2][0] = C_<T>(12) * uu;
                    Bvv[2][1] = -C_<T>(24) * uu;
                    Bvv[2][2] = C_<T>(12) * uu;
                    Bvv[3][0] = C_<T>(0);
                    Bvv[3][1] = C_<T>(0);
                    Bvv[4][0] = C_<T>(0);

                    Vector3<T> Nuu{}, Nuv{}, Nvv{};
                    T Duu = C_<T>(0), Duv = C_<T>(0), Dvv = C_<T>(0);
                    for (size_t j1 = 0; j1 <= 4; ++j1)
                    {
                        for (size_t j0 = 0; j0 <= 4 - j1; ++j0)
                        {
                            T product = mWeights[j1][j0] * Buu[j1][j0];
                            Nuu += product * mControls[j1][j0];
                            Duu += product;
                            product = mWeights[j1][j0] * Buv[j1][j0];
                            Nuv += product * mControls[j1][j0];
                            Duv += product;
                            product = mWeights[j1][j0] * Bvv[j1][j0];
                            Nvv += product * mControls[j1][j0];
                            Dvv += product;
                        }
                    }
                    Vector3<T> termDuu = D * (D * Nuu - Duu * N);
                    Vector3<T> termDuv = D * (D * Nuv - Duv * N - Du * Nv - Dv * Nu);
                    Vector3<T> termDvv = D * (D * Nvv - Dvv * N);
                    jet[3] = (D * termDuu - C_<T>(2) * Du * numerDU) / Dcub;
                    jet[4] = (D * termDuv + C_<T>(2) * Du * Dv * N) / Dcub;
                    jet[5] = (D * termDvv - C_<T>(2) * Dv * numerDV) / Dcub;
                }
            }
        }

    private:
        // For simplicity of the implementation, 2-dimensional arrays
        // of size 5-by-5 are used. Only array[r][c] is used where
        // 0 <= r <= 4 and 0 <= c < 4 - r.
        std::array<std::array<Vector3<T>, 5>, 5> mControls;
        std::array<std::array<T, 5>, 5> mWeights;
    };

    template <typename T>
    class NURBSHalfSphereDegree3 : public NURBSSurface<T, 3>
    {
    public:
        NURBSHalfSphereDegree3()
            :
            NURBSSurface<T, 3>(
                std::array<typename BasisFunction<T>::Input, 2>{
                    typename BasisFunction<T>::Input(4, 3),
                    typename BasisFunction<T>::Input(4, 3)
                },
                nullptr, nullptr)
        {
            // weight[j][i] is mWeights[i + 4 * j], 0 <= i < 4, 0 <= j < 4
            this->mWeights[0] = C_<T>(1);
            this->mWeights[1] = C_<T>(1, 3);
            this->mWeights[2] = C_<T>(1, 3);
            this->mWeights[3] = C_<T>(1);
            this->mWeights[4] = C_<T>(1, 3);
            this->mWeights[5] = C_<T>(1, 9);
            this->mWeights[6] = C_<T>(1, 9);
            this->mWeights[7] = C_<T>(1, 3);
            this->mWeights[8] = C_<T>(1, 3);
            this->mWeights[9] = C_<T>(1, 9);
            this->mWeights[10] = C_<T>(1, 9);
            this->mWeights[11] = C_<T>(1, 3);
            this->mWeights[12] = C_<T>(1);
            this->mWeights[13] = C_<T>(1, 3);
            this->mWeights[14] = C_<T>(1, 3);
            this->mWeights[15] = C_<T>(1);

            // control[j][i] is mControls[i + 4 * j], 0 <= i < 4, 0 <= j < 4
            this->mControls[0] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[1] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[2] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[3] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[4] = { C_<T>(2), C_<T>(0), C_<T>(1) };
            this->mControls[5] = { C_<T>(2), C_<T>(4), C_<T>(1) };
            this->mControls[6] = { -C_<T>(2), C_<T>(4), C_<T>(1) };
            this->mControls[7] = { -C_<T>(2), C_<T>(0), C_<T>(1) };
            this->mControls[8] = { C_<T>(2), C_<T>(0), -C_<T>(1) };
            this->mControls[9] = { C_<T>(2), C_<T>(4), -C_<T>(1) };
            this->mControls[10] = { -C_<T>(2), C_<T>(4), -C_<T>(1) };
            this->mControls[11] = { -C_<T>(2), C_<T>(0), -C_<T>(1) };
            this->mControls[12] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
            this->mControls[13] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
            this->mControls[14] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
            this->mControls[15] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
        }
    };

    template <typename T>
    class NURBSFullSphereDegree3 : public NURBSSurface<T, 3>
    {
    public:
        NURBSFullSphereDegree3()
            :
            NURBSSurface<T, 3>(CreateBasisFunctionInputs(), nullptr, nullptr)
        {
            // weight[j][i] is mWeights[i + 4 * j], 0 <= i < 4, 0 <= j < 7
            this->mWeights[0] = C_<T>(1);
            this->mWeights[4] = C_<T>(1, 3);
            this->mWeights[8] = C_<T>(1, 3);
            this->mWeights[12] = C_<T>(1);
            this->mWeights[16] = C_<T>(1, 3);
            this->mWeights[20] = C_<T>(1, 3);
            this->mWeights[24] = C_<T>(1);
            this->mWeights[1] = C_<T>(1, 3);
            this->mWeights[5] = C_<T>(1, 9);
            this->mWeights[9] = C_<T>(1, 9);
            this->mWeights[13] = C_<T>(1, 3);
            this->mWeights[17] = C_<T>(1, 9);
            this->mWeights[21] = C_<T>(1, 9);
            this->mWeights[25] = C_<T>(1, 3);
            this->mWeights[2] = C_<T>(1, 3);
            this->mWeights[6] = C_<T>(1, 9);
            this->mWeights[10] = C_<T>(1, 9);
            this->mWeights[14] = C_<T>(1, 3);
            this->mWeights[18] = C_<T>(1, 9);
            this->mWeights[22] = C_<T>(1, 9);
            this->mWeights[26] = C_<T>(1, 3);
            this->mWeights[3] = C_<T>(1);
            this->mWeights[7] = C_<T>(1, 3);
            this->mWeights[11] = C_<T>(1, 3);
            this->mWeights[15] = C_<T>(1);
            this->mWeights[19] = C_<T>(1, 3);
            this->mWeights[23] = C_<T>(1, 3);
            this->mWeights[27] = C_<T>(1);

            // control[j][i] is mControls[i + 4 * j], 0 <= i < 4, 0 <= j < 7
            this->mControls[0] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[4] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[8] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[12] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[16] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[20] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[24] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            this->mControls[1] = { C_<T>(2), C_<T>(0), C_<T>(1) };
            this->mControls[5] = { C_<T>(2), C_<T>(4), C_<T>(1) };
            this->mControls[9] = { -C_<T>(2), C_<T>(4), C_<T>(1) };
            this->mControls[13] = { -C_<T>(2), C_<T>(0), C_<T>(1) };
            this->mControls[17] = { -C_<T>(2), -C_<T>(4), C_<T>(1) };
            this->mControls[21] = { C_<T>(2), -C_<T>(4), C_<T>(1) };
            this->mControls[25] = { C_<T>(2), C_<T>(0), C_<T>(1) };
            this->mControls[2] = { C_<T>(2), C_<T>(0), -C_<T>(1) };
            this->mControls[6] = { C_<T>(2), C_<T>(4), -C_<T>(1) };
            this->mControls[10] = { -C_<T>(2), C_<T>(4), -C_<T>(1) };
            this->mControls[14] = { -C_<T>(2), C_<T>(0), -C_<T>(1) };
            this->mControls[18] = { -C_<T>(2), -C_<T>(4), -C_<T>(1) };
            this->mControls[22] = { C_<T>(2), -C_<T>(4), -C_<T>(1) };
            this->mControls[26] = { C_<T>(2), C_<T>(0), -C_<T>(1) };
            this->mControls[3] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
            this->mControls[7] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
            this->mControls[11] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
            this->mControls[15] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
            this->mControls[19] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
            this->mControls[23] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
            this->mControls[27] = { C_<T>(0), C_<T>(0), -C_<T>(1) };
        }

    private:
        static std::array<typename BasisFunction<T>::Input, 2> CreateBasisFunctionInputs()
        {
            using Input = typename BasisFunction<T>::Input;
            using UniqueKnot = typename BasisFunction<T>::UniqueKnot;

            std::array<Input, 2> input{};

            input[0] = typename BasisFunction<T>::Input(4, 3);

            input[1].numControls = 7;
            input[1].degree = 3;
            input[1].uniform = true;
            input[1].periodic = false;
            input[1].uniqueKnots.resize(3);
            input[1].uniqueKnots[0] = UniqueKnot(C_<T>(0), 4);
            input[1].uniqueKnots[1] = UniqueKnot(C_<T>(1, 2), 3);
            input[1].uniqueKnots[2] = UniqueKnot(C_<T>(1), 4);

            return input;
        }
    };
}
