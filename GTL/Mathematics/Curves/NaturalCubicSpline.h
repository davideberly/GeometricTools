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
    class NaturalCubicSpline : public ParametricCurve<T, N>
    {
    public:
        // Construct a free spline by setting 'isFree' to true or construct a
        // closed spline by setting 'isFree' to false.
        NaturalCubicSpline(bool isFree, size_t numPoints,
            Vector<T, N> const* f0, T const* times)
            :
            ParametricCurve<T, N>(numPoints - 1, times),
            mPolynomials{},
            mDelta{}
        {
            GTL_ARGUMENT_ASSERT(
                numPoints >= 2 && f0 != nullptr && times != nullptr,
                "Invalid input.");

            size_t numPm1 = numPoints - 1;
            mPolynomials.resize(numPm1);
            mDelta.resize(numPm1);
            for (size_t i0 = 0, i1 = 1; i1 < numPoints; i0 = i1++)
            {
                mDelta[i0] = times[i1] - times[i0];
            }

            Vector<T, N> boundary0{}, boundary1{};  // zero vectors
            Matrix3x3<T> R{};  // zero matrix
            size_t const numBElements = 3 * numPm1;
            std::vector<Vector<T, N>> B(numBElements);
            OnPresolve(numPoints, f0, boundary0, boundary1, R, B);

            if (isFree)
            {
                R(1, 1) = C_<T>(1);
                R(1, 2) = C_<T>(3);
                Solve(0, 1, numPoints, f0, R, B);
            }
            else // is closed
            {
                size_t const numPm2 = numPoints - 2;
                T lambda = mDelta[0] / mDelta[numPm2];
                T lambdasqr = lambda * lambda;
                R(1, 0) = -lambda;
                R(1, 1) = C_<T>(-2) * lambda;
                R(1, 2) = C_<T>(-3) * lambda;
                R(2, 1) = C_<T>(-1) * lambdasqr;
                R(2, 2) = C_<T>(-3) * lambdasqr;
                Solve(1, 1, numPoints, f0, R, B);
            }
        }

        NaturalCubicSpline(bool isFree, std::vector<Vector<T, N>> const& f0,
            std::vector<T> const& times)
            :
            NaturalCubicSpline(isFree, f0.size(), f0.data(), times.data())
        {
        }

        // Construct a clamped spline.
        NaturalCubicSpline(size_t numPoints, Vector<T, N> const* f0,
            T const* times, Vector<T, N> const& derivative0,
            Vector<T, N> const& derivative1)
            :
            ParametricCurve<T, N>(numPoints - 1, times),
            mPolynomials{},
            mDelta{}
        {
            GTL_ARGUMENT_ASSERT(
                numPoints >= 2 && f0 != nullptr && times != nullptr,
                "Invalid input.");

            size_t const numPm1 = numPoints - 1;
            mPolynomials.resize(numPm1);
            mDelta.resize(numPm1);
            for (size_t i0 = 0, i1 = 1; i1 < numPoints; i0 = i1++)
            {
                mDelta[i0] = times[i1] - times[i0];
            }

            size_t const numPm2 = numPoints - 2;
            Vector<T, N> boundary0 = mDelta[0] * derivative0;
            Vector<T, N> boundary1 = mDelta[numPm2] * derivative1;
            Matrix3x3<T> R{};  // zero matrix
            size_t const numBElements = 3 * numPm1;
            std::vector<Vector<T, N>> B(numBElements);
            OnPresolve(numPoints, f0, boundary0, boundary1, R, B);

            R(2, 0) = C_<T>(1);
            R(2, 1) = C_<T>(2);
            R(2, 2) = C_<T>(3);
            Solve(1, 0, numPoints, f0, R, B);
        }

        NaturalCubicSpline(std::vector<Vector<T, N>> const& f0,
            std::vector<T> const& times, Vector<T, N> const& derivative0,
            Vector<T, N> const& derivative1)
            :
            NaturalCubicSpline(f0.size(), f0.data(), times.data(),
                derivative0, derivative1)
        {
        }

        virtual ~NaturalCubicSpline() = default;

        using Polynomial = std::array<Vector<T, N>, 4>;

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
            jet[0] = poly[0] + u * (poly[1] + u * (poly[2] + u * poly[3]));
            if (order >= 1)
            {
                // Compute first derivative.
                T denom = mDelta[key];
                jet[1] = (poly[1] + u * (C_<T>(2) * poly[2] + u * (C_<T>(3) * poly[3]))) / denom;
                if (order >= 2)
                {
                    // Compute second derivative.
                    denom *= mDelta[key];
                    jet[2] = (C_<T>(2) * poly[2] + u * (C_<T>(6) * poly[3])) / denom;
                    if (order >= 3)
                    {
                        // Compute third derivative.
                        denom *= mDelta[key];
                        jet[3] = (C_<T>(6) * poly[3]) / denom;

                        for (size_t i = 4; i <= order; ++i)
                        {
                            // Derivatives of order 4 and higher are zero.
                            MakeZero(jet[i]);
                        }
                    }
                }
            }
        }

    private:
        void OnPresolve(size_t numPoints, Vector<T, N> const* f0,
            Vector<T, N> const& boundary0, Vector<T, N> const& boundary1,
            Matrix3x3<T>& R, std::vector<Vector<T, N>>& B)
        {
            size_t const numPm1 = numPoints - 1;
            size_t const numPm2 = numPoints - 2;
            size_t const numPm3 = numPoints - 3;

            std::array<T, 3> const coeff{ C_<T>(3), C_<T>(-3), C_<T>(1)};
            for (size_t i0 = 0, i1 = 1; i0 <= numPm3; i0 = i1++)
            {
                Vector<T, N> diff = f0[i1] - f0[i0];
                for (size_t j = 0, k = 3 * i0; j < 3; ++j, ++k)
                {
                    B[k] = coeff[j] * diff;
                }
            }

            B[B.size() - 3] = f0[numPm1] - f0[numPm2];
            B[B.size() - 2] = boundary0;
            B[B.size() - 1] = boundary1;

            R(0, 0) = C_<T>(1);
            R(0, 1) = C_<T>(1);
            R(0, 2) = C_<T>(1);
        }

        void Solve(size_t ell10, size_t ell21, size_t numPoints,
            Vector<T, N> const* f0, Matrix3x3<T>& R, std::vector<Vector<T, N>>& B)
        {
            RowReduce(ell10, ell21, numPoints, R, B);
            BackSubstitute(f0, R, B);
        }

        void RowReduce(size_t ell10, size_t ell21, size_t numPoints,
            Matrix3x3<T>& R, std::vector<Vector<T, N>>& B)
        {
            // Apply the row reductions to convert the matrix system to
            // upper-triangular block-matrix system.
            size_t const numPm3 = numPoints - 3;

            if (ell10 == 1)
            {
                Vector<T, N>& Btrg = B[B.size() - 2];
                Btrg -= B[0];
                T sigma = mDelta[0] / mDelta[1];
                T sigmasqr = sigma * sigma;
                T LUProd0 = C_<T>(2) * sigma;
                T LUProd1 = -sigmasqr;
                T sign = C_<T>(-1);

                for (size_t i = 1; i <= numPm3; ++i)
                {
                    Btrg -= sign * (LUProd0 * B[3 * i] + LUProd1 * B[3 * i + 1]);
                    sigma = mDelta[i] / mDelta[i + 1];
                    sigmasqr = sigma * sigma;
                    T temp0 = sigma * (C_<T>(2) * LUProd0 - C_<T>(3) * LUProd1);
                    T temp1 = sigmasqr * (-LUProd0 + C_<T>(2) * LUProd1);
                    LUProd0 = temp0;
                    LUProd1 = temp1;
                    sign = -sign;
                }

                R(1, 0) += sign * LUProd0;
                R(1, 1) += sign * LUProd1;
            }

            if (ell21 == 1)
            {
                Vector<T, N>& Btrg = B[B.size() - 1];
                Btrg -= B[1];
                T sigma = mDelta[0] / mDelta[1];
                T sigmasqr = sigma * sigma;
                T LUProd0 = C_<T>(-3) * sigma;
                T LUProd1 = C_<T>(2) * sigmasqr;
                T sign = C_<T>(-1);

                for (size_t i = 1; i <= numPm3; ++i)
                {
                    Btrg -= sign * (LUProd0 * B[3 * i] + LUProd1 * B[3 * i + 1]);
                    sigma = mDelta[i] / mDelta[i + 1];
                    sigmasqr = sigma * sigma;
                    T temp0 = sigma * (C_<T>(2) * LUProd0 - C_<T>(3) * LUProd1);
                    T temp1 = sigmasqr * (-LUProd0 + C_<T>(2) * LUProd1);
                    LUProd0 = temp0;
                    LUProd1 = temp1;
                    sign = -sign;
                }

                R(2, 0) += sign * LUProd0;
                R(2, 1) += sign * LUProd1;
            }
        }

        void BackSubstitute(Vector<T, N> const* f0, Matrix3x3<T> const& R,
            std::vector<Vector<T, N>> const& B)
        {
            T determinant = C_<T>(0);
            Matrix3x3<T> invR = GetInverse(R, &determinant);
            GTL_RUNTIME_ASSERT(
                determinant != C_<T>(0),
                "R matrix is not invertible.");

            auto& poly = mPolynomials.back();
            size_t j0 = B.size() - 3;
            size_t j1 = j0 + 1;
            size_t j2 = j0 + 2;

            poly[0] = f0[mPolynomials.size() - 1];
            poly[1] = invR(0, 0) * B[j0] + invR(0, 1) * B[j1] + invR(0, 2) * B[j2];
            poly[2] = invR(1, 0) * B[j0] + invR(1, 1) * B[j1] + invR(1, 2) * B[j2];
            poly[3] = invR(2, 0) * B[j0] + invR(2, 1) * B[j1] + invR(2, 2) * B[j2];

            size_t const numPm2 = mPolynomials.size() - 2;
            for (size_t k = 0, i1 = numPm2, i0 = i1 + 1; k <= numPm2; ++k, i0 = i1--)
            {
                auto const& prev = mPolynomials[i0];
                auto& curr = mPolynomials[i1];
                T sigma = mDelta[i1] / mDelta[i0];
                T sigmasqr = sigma * sigma;
                T u00 = C_<T>(2) * sigma;
                T u01 = -sigmasqr;
                T u10 = C_<T>(-3) * sigma;
                T u11 = C_<T>(2) * sigmasqr;
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
