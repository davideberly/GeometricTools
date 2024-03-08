// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Documentation for natural splines is found in
// https://www.geometrictools.com/Documentation/NaturalSplines.pdf
// The number of points must be 2 or larger. The points[] and times[] arrays
// must have the same number of elements. The times[] values must be strictly
// increasing.

#include <Mathematics/Matrix.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/ParametricCurve.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <int32_t N, typename T>
    class NaturalCubicSpline : public ParametricCurve<N, T>
    {
    public:
        // Construct a free spline by setting 'isFree' to true or construct a
        // closed spline by setting 'isFree' to false.
        NaturalCubicSpline(bool isFree, int32_t numPoints,
            Vector<N, T> const* f0, T const* times)
            :
            ParametricCurve<N, T>(numPoints - 1, times),
            mPolynomials{},
            mDelta{}
        {
            LogAssert(
                numPoints >= 2 && f0 != nullptr && times != nullptr,
                "Invalid input.");

            int32_t numPm1 = numPoints - 1;
            mPolynomials.resize(numPm1);
            mDelta.resize(numPm1);
            for (int32_t i0 = 0, i1 = 1; i1 < numPoints; i0 = i1++)
            {
                mDelta[i0] = times[i1] - times[i0];
            }

            Vector<N, T> boundary0{}, boundary1{};
            boundary0.MakeZero();
            boundary1.MakeZero();
            Matrix3x3<T> R{};
            int32_t const numBElements = 3 * numPm1;
            std::vector<Vector<N, T>> B(numBElements);
            OnPresolve(numPoints, f0, boundary0, boundary1, R, B);

            T const r1 = static_cast<T>(1);
            T const r2 = static_cast<T>(2);
            T const r3 = static_cast<T>(3);
            if (isFree)
            {
                R(1, 1) = r1;
                R(1, 2) = r3;
                Solve(0, 1, numPoints, f0, R, B);
            }
            else // is closed
            {
                int32_t const numPm2 = numPoints - 2;
                T lambda = mDelta[0] / mDelta[numPm2];
                T lambdasqr = lambda * lambda;
                R(1, 0) = -lambda;
                R(1, 1) = -r2 * lambda;
                R(1, 2) = -r3 * lambda;
                R(2, 1) = -r1 * lambdasqr;
                R(2, 2) = -r3 * lambdasqr;
                Solve(1, 1, numPoints, f0, R, B);
            }

            this->mConstructed = true;
        }

        NaturalCubicSpline(bool isFree, std::vector<Vector<N, T>> const& f0,
            std::vector<T> const& times)
            :
            NaturalCubicSpline(isFree, static_cast<int32_t>(f0.size()),
                f0.data(), times.data())
        {
        }

        // Construct a clamped spline.
        NaturalCubicSpline(int32_t numPoints, Vector<N, T> const* f0,
            T const* times, Vector<N, T> const& derivative0,
            Vector<N, T> const& derivative1)
            :
            ParametricCurve<N, T>(numPoints - 1, times),
            mPolynomials{},
            mDelta{}
        {
            LogAssert(
                numPoints >= 2 && f0 != nullptr && times != nullptr,
                "Invalid input.");

            int32_t const numPm1 = numPoints - 1;
            mPolynomials.resize(numPm1);
            mDelta.resize(numPm1);
            for (int32_t i0 = 0, i1 = 1; i1 < numPoints; i0 = i1++)
            {
                mDelta[i0] = times[i1] - times[i0];
            }

            int32_t const numPm2 = numPoints - 2;
            Vector<N, T> boundary0 = mDelta[0] * derivative0;
            Vector<N, T> boundary1 = mDelta[numPm2] * derivative1;
            Matrix3x3<T> R{};
            int32_t const numBElements = 3 * numPm1;
            std::vector<Vector<N, T>> B(numBElements);
            OnPresolve(numPoints, f0, boundary0, boundary1, R, B);

            T const r1 = static_cast<T>(1);
            T const r2 = static_cast<T>(2);
            T const r3 = static_cast<T>(3);
            R(2, 0) = r1;
            R(2, 1) = r2;
            R(2, 2) = r3;
            Solve(1, 0, numPoints, f0, R, B);

            this->mConstructed = true;
        }

        NaturalCubicSpline(std::vector<Vector<N, T>> const& f0,
            std::vector<T> const& times, Vector<N, T> const& derivative0,
            Vector<N, T> const& derivative1)
            :
            NaturalCubicSpline(static_cast<int32_t>(f0.size()),
                f0.data(), times.data(), derivative0, derivative1)
        {
        }

        virtual ~NaturalCubicSpline() = default;

        using Polynomial = std::array<Vector<N, T>, 4>;

        inline std::vector<Polynomial> const& GetPolynomials() const
        {
            return mPolynomials;
        }

        // Evaluation of the function and its derivatives through order 3. If
        // you want only the position, pass in order 0. If you want the 
        // position and first derivative, pass in order of 1 and so on. The
        // output array 'jet' must have 'order + 1' elements. The values are
        // ordered as position, first derivative, second derivative and so on.
        virtual void Evaluate(T t, uint32_t order, Vector<N, T>* jet) const override
        {
            if (!this->mConstructed)
            {
                // Return a zero-valued jet for invalid state.
                for (uint32_t i = 0; i <= order; ++i)
                {
                    jet[i].MakeZero();
                }
                return;
            }

            size_t key = 0;
            T u = static_cast<T>(0);
            GetKeyInfo(t, key, u);
            auto const& poly = mPolynomials[key];

            // Compute position.
            jet[0] = poly[0] + u * (poly[1] + u * (poly[2] + u * poly[3]));
            if (order >= 1)
            {
                // Compute first derivative.
                T const r2 = static_cast<T>(2);
                T const r3 = static_cast<T>(3);
                T denom = mDelta[key];
                jet[1] = (poly[1] + u * (r2 * poly[2] + u * (r3 * poly[3]))) / denom;
                if (order >= 2)
                {
                    // Compute second derivative.
                    T const r6 = static_cast<T>(6);
                    denom *= mDelta[key];
                    jet[2] = (r2 * poly[2] + u * (r6 * poly[3])) / denom;
                    if (order >= 3)
                    {
                        // Compute third derivative.
                        denom *= mDelta[key];
                        jet[3] = (r6 * poly[3]) / denom;

                        for (uint32_t i = 4; i <= order; ++i)
                        {
                            // Derivatives of order 4 and higher are zero.
                            jet[i].MakeZero();
                        }
                    }
                }
            }
        }

    private:
        void OnPresolve(int32_t numPoints, Vector<N, T> const* f0,
            Vector<N, T> const& boundary0, Vector<N, T> const& boundary1,
            Matrix3x3<T>& R, std::vector<Vector<N, T>>& B)
        {
            int32_t const numPm1 = numPoints - 1;
            int32_t const numPm2 = numPoints - 2;
            int32_t const numPm3 = numPoints - 3;

            T const r1 = static_cast<T>(1);
            T const r3 = static_cast<T>(3);
            std::array<T, 3> const coeff{ r3, -r3, r1 };
            for (int32_t i0 = 0, i1 = 1; i0 <= numPm3; i0 = i1++)
            {
                Vector<N, T> diff = f0[i1] - f0[i0];
                for (int32_t j = 0, k = 3 * i0; j < 3; ++j, ++k)
                {
                    B[k] = coeff[j] * diff;
                }
            }

            B[B.size() - 3] = f0[numPm1] - f0[numPm2];
            B[B.size() - 2] = boundary0;
            B[B.size() - 1] = boundary1;

            R(0, 0) = r1;
            R(0, 1) = r1;
            R(0, 2) = r1;
        }

        void Solve(int32_t ell10, int32_t ell21, int32_t numPoints,
            Vector<N, T> const* f0, Matrix3x3<T>& R, std::vector<Vector<N, T>>& B)
        {
            RowReduce(ell10, ell21, numPoints, R, B);
            BackSubstitute(f0, R, B);
        }

        void RowReduce(int32_t ell10, int32_t ell21, int32_t numPoints,
            Matrix<3, 3, T>& R, std::vector<Vector<N, T>>& B)
        {
            // Apply the row reductions to convert the matrix system to
            // upper-triangular block-matrix system.
            T const r1 = static_cast<T>(1);
            T const r2 = static_cast<T>(2);
            T const r3 = static_cast<T>(3);
            int32_t const numPm3 = numPoints - 3;

            if (ell10 == 1)
            {
                Vector<N, T>& Btrg = B[B.size() - 2];
                Btrg -= B[0];
                T sigma = mDelta[0] / mDelta[1];
                T sigmasqr = sigma * sigma;
                T LUProd0 = r2 * sigma, LUProd1 = -sigmasqr;
                T sign = -r1;

                for (size_t i = 1; i <= static_cast<size_t>(numPm3); ++i)
                {
                    Btrg -= sign * (LUProd0 * B[3 * i] + LUProd1 * B[3 * i + 1]);
                    sigma = mDelta[i] / mDelta[i + 1];
                    sigmasqr = sigma * sigma;
                    T temp0 = sigma * (r2 * LUProd0 - r3 * LUProd1);
                    T temp1 = sigmasqr * (-LUProd0 + r2 * LUProd1);
                    LUProd0 = temp0;
                    LUProd1 = temp1;
                    sign = -sign;
                }

                R(1, 0) += sign * LUProd0;
                R(1, 1) += sign * LUProd1;
            }

            if (ell21 == 1)
            {
                Vector<N, T>& Btrg = B[B.size() - 1];
                Btrg -= B[1];
                T sigma = mDelta[0] / mDelta[1];
                T sigmasqr = sigma * sigma;
                T LUProd0 = -r3 * sigma, LUProd1 = r2 * sigmasqr;
                T sign = -r1;

                for (size_t i = 1; i <= static_cast<size_t>(numPm3); ++i)
                {
                    Btrg -= sign * (LUProd0 * B[3 * i] + LUProd1 * B[3 * i + 1]);
                    sigma = mDelta[i] / mDelta[i + 1];
                    sigmasqr = sigma * sigma;
                    T temp0 = sigma * (r2 * LUProd0 - r3 * LUProd1);
                    T temp1 = sigmasqr * (-LUProd0 + r2 * LUProd1);
                    LUProd0 = temp0;
                    LUProd1 = temp1;
                    sign = -sign;
                }

                R(2, 0) += sign * LUProd0;
                R(2, 1) += sign * LUProd1;
            }
        }

        void BackSubstitute(Vector<N, T> const* f0, Matrix3x3<T> const& R,
            std::vector<Vector<N, T>> const& B)
        {
            bool invertible = false;
            Matrix3x3<T> invR = Inverse(R, &invertible);
            LogAssert(
                invertible,
                "R matrix is not invertible.");

            auto& poly = mPolynomials.back();
            size_t j0 = B.size() - 3;
            size_t j1 = j0 + 1;
            size_t j2 = j0 + 2;

            poly[0] = f0[mPolynomials.size() - 1];
            poly[1] = invR(0, 0) * B[j0] + invR(0, 1) * B[j1] + invR(0, 2) * B[j2];
            poly[2] = invR(1, 0) * B[j0] + invR(1, 1) * B[j1] + invR(1, 2) * B[j2];
            poly[3] = invR(2, 0) * B[j0] + invR(2, 1) * B[j1] + invR(2, 2) * B[j2];

            T const r2 = static_cast<T>(2);
            T const r3 = static_cast<T>(3);
            int32_t const numPolynomials = static_cast<int32_t>(mPolynomials.size());
            for (int32_t i1 = numPolynomials - 2, i0 = i1 + 1; i1 >= 0; i0 = i1--)
            {
                auto const& prev = mPolynomials[i0];
                auto& curr = mPolynomials[i1];
                T sigma = mDelta[i1] / mDelta[i0];
                T sigmasqr = sigma * sigma;
                T u00 = r2 * sigma;
                T u01 = -sigmasqr;
                T u10 = -r3 * sigma;
                T u11 = r2 * sigmasqr;
                T u20 = sigma;
                T u21 = -sigmasqr;

                j0 -= 3;
                j1 -= 3;
                j2 -= 3;

                curr[0] = f0[i1];
                curr[1] = B[j0] - (u00 * prev[1] + u01 * prev[2]);
                curr[2] = B[j1] - (u10 * prev[1] + u11 * prev[2]);
                curr[3] = B[j2] - (u20 * prev[1] + u21 * prev[2]);
            }
        }

        // Determine the index key for which times[key] <= t < times[key+1].
        // Return u = (t - times[key]) / delta[key] which is in [0,1].
        void GetKeyInfo(T const& t, size_t& key, T& u) const
        {
            size_t const numSegments = static_cast<size_t>(this->GetNumSegments());
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
                    u = static_cast<T>(1);
                }

            }
            else
            {
                key = 0;
                u = static_cast<T>(0);
            }
        }

        std::vector<Polynomial> mPolynomials;
        std::vector<T> mDelta;
    };
}
