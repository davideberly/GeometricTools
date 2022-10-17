// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

#include <GTL/Mathematics/Algebra/Matrix.h>
#include <GTL/Mathematics/Curves/ParametricCurve.h>
#include <array>
#include <vector>

// Documentation for natural splines is found in
// https://www.geometrictools.com/Documentation/NaturalSplines.pdf
// The number of points must be 2 or larger. The points[] and times[] arrays
// must have the same number of elements. The times[] values must be strictly
// increasing.

namespace gtl
{
    template <typename T, size_t N>
    class NaturalQuinticSpline : public ParametricCurve<T, N>
    {
    public:
        // Construct a free spline by setting 'isFree' to true or construct a
        // closed spline by setting 'isFree' to false. The function values are
        // f0[] and the first derivative values are f1[].
        NaturalQuinticSpline(bool isFree, size_t numPoints,
            Vector<T, N> const* f0, Vector<T, N> const* f1, T const* times)
            :
            ParametricCurve<T, N>(numPoints - 1, times),
            mPolynomials{},
            mDelta{}
        {
            GTL_ARGUMENT_ASSERT(
                numPoints >= 2 && f0 != nullptr && f1 != nullptr && times != nullptr,
                "Invalid input.");

            size_t numPm1 = numPoints - 1;
            mPolynomials.resize(numPm1);
            mDelta.resize(numPm1);
            for (size_t i0 = 0, i1 = 1; i1 < numPoints; i0 = i1++)
            {
                mDelta[i0] = times[i1] - times[i0];
            }

            // Free splines and closed splines have the last two B-entries
            // set to the zero vector.
            Vector<T, N> boundary0{}, boundary1{};  // zero vectors
            Matrix4x4<T> R{};  // zero matrix
            size_t const numBElements = 4 * numPm1;
            std::vector<Vector<T, N>> B(numBElements);
            OnPresolve(numPoints, f0, f1, boundary0, boundary1, R, B);

            if (isFree)
            {
                R(2, 1) = C_<T>(1);
                R(2, 2) = C_<T>(4);
                R(2, 3) = C_<T>(10);
                Solve(0, 1, numPoints, f0, f1, R, B);
            }
            else // is closed
            {
                size_t const numPm2 = numPoints - 2;
                T lambda = mDelta[0] / mDelta[numPm2];
                T lambdasqr = lambda * lambda;
                T lambdacub = lambdasqr * lambda;
                R(2, 0) = -lambdasqr;
                R(2, 1) = C_<T>(-3) * lambdasqr;
                R(2, 2) = C_<T>(-6) * lambdasqr;
                R(2, 3) = C_<T>(-10) * lambdasqr;
                R(3, 1) = C_<T>(-1) * lambdacub;
                R(3, 2) = C_<T>(-4) * lambdacub;
                R(3, 3) = C_<T>(-10) * lambdacub;
                Solve(1, 1, numPoints, f0, f1, R, B);
            }
        }

        NaturalQuinticSpline(bool isFree, std::vector<Vector<T, N>> const& f0,
            std::vector<Vector<T, N>> const& f1, std::vector<T> const& times)
            :
            NaturalQuinticSpline(isFree, f0.size(), f0.data(), f1.data(), times.data())
        {
        }

        // Construct a clamped spline.
        NaturalQuinticSpline(size_t numPoints, Vector<T, N> const* f0,
            Vector<T, N> const* f1, T const* times,
            Vector<T, N> const& derivative0, Vector<T, N> const& derivative1)
            :
            ParametricCurve<T, N>(numPoints - 1, times),
            mPolynomials{},
            mDelta{}
        {
            GTL_ARGUMENT_ASSERT(
                numPoints >= 2 && f0 != nullptr && f1 != nullptr && times != nullptr,
                "Invalid input.");

            size_t numPm1 = numPoints - 1;
            mPolynomials.resize(numPm1);
            mDelta.resize(numPm1);
            for (size_t i0 = 0, i1 = 1; i1 < numPoints; i0 = i1++)
            {
                mDelta[i0] = times[i1] - times[i0];
            }

            size_t const numPm2 = numPoints - 2;
            T coeff0 = C_<T>(1, 2) * mDelta[0] * mDelta[0];
            T coeff1 = C_<T>(1, 2) * mDelta[numPm2] * mDelta[numPm2];
            Vector<T, N> boundary0 = coeff0 * derivative0;
            Vector<T, N> boundary1 = coeff1 * derivative1;
            Matrix4x4<T> R{};  // zero matrix
            size_t const numBElements = 4 * numPm1;
            std::vector<Vector<T, N>> B(numBElements);
            OnPresolve(numPoints, f0, f1, boundary0, boundary1, R, B);

            R(3, 0) = C_<T>(1);
            R(3, 1) = C_<T>(3);
            R(3, 2) = C_<T>(6);
            R(3, 3) = C_<T>(10);
            Solve(1, 0, numPoints, f0, f1, R, B);
        }

        NaturalQuinticSpline(std::vector<Vector<T, N>> const& f0,
            std::vector<Vector<T, N>> const& f1, std::vector<T> const& times,
            Vector<T, N> const& derivative0, Vector<T, N> const& derivative1)
            :
            NaturalQuinticSpline(static_cast<int32_t>(f0.size()),
                f0.data(), f1.data(), times.data(), derivative0, derivative1)
        {
        }

        virtual ~NaturalQuinticSpline() = default;

        using Polynomial = std::array<Vector<T, N>, 6>;

        inline std::vector<Polynomial> const& GetPolynomials() const
        {
            return mPolynomials;
        }

        // Evaluation of the curve. If you want only the position, pass in
        // order of 0. If you want the position and first derivative, pass in
        // order of 1, and so on. The output array 'jet' must have enough
        // storage to support the specified order. The values are ordered as:
        // position, first derivative, second derivative, and so on.
        virtual void Evaluate(T const& t, size_t order, Vector<T, N>* jet) const override
        {
            size_t key = 0;
            T u = C_<T>(0);
            GetKeyInfo(t, key, u);
            auto const& poly = mPolynomials[key];

            // Compute position.
            jet[0] = poly[0] + u * (poly[1] + u * (poly[2] + u * (poly[3] + u * (poly[4] + u * poly[5]))));
            if (order >= 1)
            {
                // Compute first derivative.
                T const r2 = static_cast<T>(2);
                T const r3 = static_cast<T>(3);
                T const r4 = static_cast<T>(4);
                T const r5 = static_cast<T>(5);
                T denom = mDelta[key];
                jet[1] = (poly[1] + u * (r2 * poly[2] + u * (r3 * poly[3] +
                    u * (r4 * poly[4] + u * (r5 * poly[5]))))) / denom;
                if (order >= 2)
                {
                    // Compute second derivative.
                    T const r6 = static_cast<T>(6);
                    T const r12 = static_cast<T>(12);
                    T const r20 = static_cast<T>(20);
                    denom *= mDelta[key];
                    jet[2] = (r2 * poly[2] + u * (r6 * poly[3] + u * (r12 * poly[4] +
                        u * (r20 * poly[5])))) / denom;
                    if (order >= 3)
                    {
                        // Compute third derivative.
                        T const r24 = static_cast<T>(24);
                        T const r60 = static_cast<T>(60);
                        denom *= mDelta[key];
                        jet[3] = (r6 * poly[3] + u * (r24 * poly[4] +
                            u * (r60 * poly[5]))) / denom;
                        if (order >= 4)
                        {
                            // Compute fourth derivative.
                            denom *= mDelta[key];
                            jet[4] = (r24 * poly[4] + u * (r60 * poly[5])) / denom;

                            if (order >= 5)
                            {
                                // Compute fifth derivative.
                                denom *= mDelta[key];
                                jet[5] = (r60 * poly[5]) / denom;

                                for (uint32_t i = 6; i <= order; ++i)
                                {
                                    // Derivatives of order 6 and higher are zero.
                                    MakeZero(jet[i]);
                                }
                            }
                        }
                    }
                }
            }
        }

    private:
        void OnPresolve(size_t numPoints, Vector<T, N> const* f0,
            Vector<T, N> const* f1, Vector<T, N> const& boundary0,
            Vector<T, N> const& boundary1, Matrix4x4<T>& R,
            std::vector<Vector<T, N>>& B)
        {
            size_t const numPm1 = numPoints - 1;
            size_t const numPm2 = numPoints - 2;
            size_t const numPm3 = numPoints - 3;

            std::array<T, 4> coeff0{ C_<T>(10), C_<T>(-20), C_<T>(15), C_<T>(-4) };
            std::array<T, 4> coeff1{ C_<T>(-6), C_<T>(14), C_<T>(-11), C_<T>(3) };
            for (size_t i0 = 0, i1 = 1; i0 <= numPm3; i0 = i1++)
            {
                Vector<T, N> diff0 = f0[i1] - f0[i0] - mDelta[i0] * f1[i0];
                Vector<T, N> diff1 = mDelta[i0] * (f1[i1] - f1[i0]);
                for (size_t j = 0, k = 4 * i0; j < 4; ++j, ++k)
                {
                    B[k] = coeff0[j] * diff0 + coeff1[j] * diff1;
                }
            }

            B[B.size() - 4] = f0[numPm1] - f0[numPm2] - mDelta[numPm2] * f1[numPm2];
            B[B.size() - 3] = mDelta[numPm2] * (f1[numPm1] - f1[numPm2]);
            B[B.size() - 2] = boundary0;
            B[B.size() - 1] = boundary1;

            R(0, 0) = C_<T>(1);
            R(0, 1) = C_<T>(1);
            R(0, 2) = C_<T>(1);
            R(0, 3) = C_<T>(1);
            R(1, 0) = C_<T>(2);
            R(1, 1) = C_<T>(3);
            R(1, 2) = C_<T>(4);
            R(1, 3) = C_<T>(5);
        }

        void Solve(size_t ell20, size_t ell31, size_t numPoints,
            Vector<T, N> const* f0, Vector<T, N> const* f1,
            Matrix4x4<T>& R, std::vector<Vector<T, N>>& B)
        {
            RowReduce(ell20, ell31, numPoints, R, B);
            BackSubstitute(f0, f1, R, B);
        }

        void RowReduce(size_t ell20, size_t ell31, size_t numPoints,
            Matrix4x4<T>& R, std::vector<Vector<T, N>>& B)
        {
            // Apply the row reductions to convert the matrix system to
            // upper-triangular block-matrix system.
            size_t const numPm3 = numPoints - 3;

            if (ell20 == 1)
            {
                Vector<T, N>& Btrg = B[B.size() - 2];
                Btrg -= B[0];
                T sigma = mDelta[0] / mDelta[1];
                T sigmasqr = sigma * sigma;
                T sigmacub = sigmasqr * sigma;
                T LUProd0 = C_<T>(-3) * sigmasqr;
                T LUProd1 = sigmacub;
                T sign = C_<T>(-1);

                for (size_t i = 1; i <= numPm3; ++i)
                {
                    Btrg -= sign * (LUProd0 * B[4 * i] + LUProd1 * B[4 * i + 1]);
                    sigma = mDelta[i] / mDelta[i + 1];
                    sigmasqr = sigma * sigma;
                    sigmacub = sigmasqr * sigma;
                    T temp0 = sigmasqr * (C_<T>(-3) * LUProd0 + C_<T>(8) * LUProd1);
                    T temp1 = sigmacub * (LUProd0 - C_<T>(3) * LUProd1);
                    LUProd0 = temp0;
                    LUProd1 = temp1;
                    sign = -sign;
                }

                R(2, 0) += sign * LUProd0;
                R(2, 1) += sign * LUProd1;
            }

            if (ell31 == 1)
            {
                Vector<T, N>& Btrg = B[B.size() - 1];
                Btrg -= B[1];
                T sigma = mDelta[0] / mDelta[1];
                T sigmasqr = sigma * sigma;
                T sigmacub = sigmasqr * sigma;
                T LUProd0 = C_<T>(8) * sigmasqr;
                T LUProd1 = C_<T>(-3) * sigmacub;
                T sign = C_<T>(-1);

                for (size_t i = 1; i <= numPm3; ++i)
                {
                    Btrg -= sign * (LUProd0 * B[4 * i] + LUProd1 * B[4 * i + 1]);
                    sigma = mDelta[i] / mDelta[i + 1];
                    sigmasqr = sigma * sigma;
                    sigmacub = sigmasqr * sigma;
                    T temp0 = sigmasqr * (C_<T>(-3) * LUProd0 + C_<T>(8) * LUProd1);
                    T temp1 = sigmacub * (LUProd0 - C_<T>(3) * LUProd1);
                    LUProd0 = temp0;
                    LUProd1 = temp1;
                    sign = -sign;
                }

                R(3, 0) += sign * LUProd0;
                R(3, 1) += sign * LUProd1;
            }
        }

        void BackSubstitute(Vector<T, N> const* f0, Vector<T, N> const* f1,
            Matrix4x4<T> const& R, std::vector<Vector<T, N>> const& B)
        {
            T determinant = C_<T>(0);
            Matrix4x4<T> invR = GetInverse(R, &determinant);
            GTL_RUNTIME_ASSERT(
                determinant != C_<T>(0),
                "R matrix is not invertible.");

            auto& poly = mPolynomials.back();
            size_t j0 = B.size() - 4;
            size_t j1 = j0 + 1;
            size_t j2 = j0 + 2;
            size_t j3 = j0 + 3;

            poly[0] = f0[mPolynomials.size() - 1];
            poly[1] = f1[mPolynomials.size() - 1] * mDelta[mPolynomials.size() - 1];
            poly[2] = invR(0, 0) * B[j0] + invR(0, 1) * B[j1] + invR(0, 2) * B[j2] + invR(0, 3) * B[j3];
            poly[3] = invR(1, 0) * B[j0] + invR(1, 1) * B[j1] + invR(1, 2) * B[j2] + invR(1, 3) * B[j3];
            poly[4] = invR(2, 0) * B[j0] + invR(2, 1) * B[j1] + invR(2, 2) * B[j2] + invR(2, 3) * B[j3];
            poly[5] = invR(3, 0) * B[j0] + invR(3, 1) * B[j1] + invR(3, 2) * B[j2] + invR(3, 3) * B[j3];

            size_t const numPm2 = mPolynomials.size() - 2;
            for (size_t k = 0, i1 = numPm2, i0 = i1 + 1; k <= numPm2; ++k, i0 = i1--)
            {
                auto const& prev = mPolynomials[i0];
                auto& curr = mPolynomials[i1];
                T sigma = mDelta[i1] / mDelta[i0];
                T sigmasqr = sigma * sigma;
                T sigmacub = sigmasqr * sigma;
                T u00 = C_<T>(-3) * sigmasqr;
                T u01 = sigmacub;
                T u10 = C_<T>(8) * sigmasqr;
                T u11 = C_<T>(-3) * sigmacub;
                T u20 = C_<T>(-7) * sigmasqr;
                T u21 = C_<T>(3) * sigmacub;
                T u30 = C_<T>(2) * sigmasqr;
                T u31 = -sigmacub;

                j0 -= 4;
                j1 -= 4;
                j2 -= 4;
                j3 -= 4;

                curr[0] = f0[i1];
                curr[1] = f1[i1] * mDelta[i1];
                curr[2] = B[j0] - (u00 * prev[2] + u01 * prev[3]);
                curr[3] = B[j1] - (u10 * prev[2] + u11 * prev[3]);
                curr[4] = B[j2] - (u20 * prev[2] + u21 * prev[3]);
                curr[5] = B[j3] - (u30 * prev[2] + u31 * prev[3]);
            }
        }

        // Determine the index key for which times[key] <= t < times[key+1].
        // Return u = (t - times[key]) / delta[key] which is in [0,1].
        void GetKeyInfo(T const& t, size_t& key, T& u) const
        {
            size_t const numSegments = this->GetNumSegments();
            if (t > this->mTime[0])
            {
                if (t < this->mTime[numSegments])
                {
                    for (size_t i = 0, ip1 = 1; i < numSegments; i = ip1++)
                    {
                        if (t < this->mTime[ip1])
                        {
                            key = i;
                            u = (t - this->mTime[i]) / mDelta[i];
                            break;
                        }
                    }
                }
                else
                {
                    key = numSegments - 1;
                    u = C_<T>(1);
                }

            }
            else
            {
                key = 0;
                u = C_<T>(0);
            }
        }

        std::vector<Polynomial> mPolynomials;
        std::vector<T> mDelta;
    };
}
