// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// NOTE: This class is now deprecated and will not be ported to GTL. Use
// instead the new class NaturalCubicSpline. There is also an extension of
// the idea in the new class NaturalQuinticSpline.

#include <Mathematics/LinearSystem.h>
#include <Mathematics/ParametricCurve.h>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <int32_t N, typename Real>
    class NaturalSplineCurve : public ParametricCurve<N, Real>
    {
    public:
        // Construction and destruction. The object copies the input arrays.
        // The number of points M must be at least 2. The first constructor
        // is for a spline with second derivatives zero at the endpoints
        // (isFree = true) or a spline that is closed (isFree = false). The
        // second constructor is for clamped splines, where you specify the
        // first derivatives at the endpoints.  Usually, derivative0 =
        // points[1] - points[0] at the first point and derivative1 =
        // points[M-1] - points[M-2]. To validate construction, create an
        // object as shown:
        //     NaturalSplineCurve<N, Real> curve(parameters);
        //     if (!curve) { <constructor failed, handle accordingly>; }
        NaturalSplineCurve(bool isFree, int32_t numPoints,
            Vector<N, Real> const* points, Real const* times)
            :
            ParametricCurve<N, Real>(numPoints - 1, times),
            mNumPoints(0),
            mNumSegments(0)
        {
            LogAssert(
                numPoints >= 2 && points != nullptr && times != nullptr,
                "Invalid input.");

            mNumPoints = static_cast<size_t>(numPoints);
            mNumSegments = mNumPoints - 1;
            mCoefficients.resize(4 * mNumPoints - 2);
            mA = mCoefficients.data();
            mB = mA + mNumPoints;
            mC = mB + mNumSegments;
            mD = mC + mNumSegments + 1;
            for (size_t i = 0; i < mNumPoints; ++i)
            {
                mA[i] = points[i];
            }

            if (isFree)
            {
                CreateFree();
            }
            else
            {
                CreateClosed();
            }

            this->mConstructed = true;
        }

        NaturalSplineCurve(int32_t numPoints, Vector<N, Real> const* points,
            Real const* times, Vector<N, Real> const& derivative0,
            Vector<N, Real> const& derivative1)
            :
            ParametricCurve<N, Real>(numPoints - 1, times),
            mNumPoints(0),
            mNumSegments(0)
        {
            LogAssert(
                numPoints >= 2 && points != nullptr && times != nullptr,
                "Invalid input.");

            mNumPoints = static_cast<size_t>(numPoints);
            mNumSegments = mNumPoints - 1;
            mCoefficients.resize(4 * static_cast<size_t>(mNumPoints) - 2);
            mA = mCoefficients.data();
            mB = mA + mNumPoints;
            mC = mB + mNumSegments;
            mD = mC + mNumSegments + 1;
            for (size_t i = 0; i < mNumPoints; ++i)
            {
                mA[i] = points[i];
            }

            CreateClamped(derivative0, derivative1);
            this->mConstructed = true;
        }

        virtual ~NaturalSplineCurve() = default;

        // Member access.
        inline size_t GetNumPoints() const
        {
            return mNumPoints;
        }

        inline Vector<N, Real> const* GetPoints() const
        {
            return mA;
        }

        // Evaluation of the function and its derivatives through order 3. If
        // you want only the position, pass in order 0. If you want the 
        // position and first derivative, pass in order of 1 and so on. The
        // output array 'jet' must have 'order + 1' elements. The values are
        // ordered as position, first derivative, second derivative and so on.
        virtual void Evaluate(Real t, uint32_t order, Vector<N, Real>* jet) const override
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
            Real dt = (Real)0;
            GetKeyInfo(t, key, dt);

            // Compute position.
            jet[0] = mA[key] + dt * (mB[key] + dt * (mC[key] + dt * mD[key]));
            if (order >= 1)
            {
                // Compute first derivative.
                jet[1] = mB[key] + dt * ((Real)2 * mC[key] + (Real)3 * dt * mD[key]);
                if (order >= 2)
                {
                    // Compute second derivative.
                    jet[2] = (Real)2 * mC[key] + (Real)6 * dt * mD[key];
                    if (order >= 3)
                    {
                        jet[3] = (Real)6 * mD[key];

                        for (uint32_t i = 4; i <= order; ++i)
                        {
                            jet[i].MakeZero();
                        }
                    }
                }
            }
        }

    protected:
        void CreateFree()
        {
            size_t const numP = mNumPoints;
            size_t const numS = mNumSegments;
            size_t const numSm1 = mNumSegments - 1;

            // Minimize allocation and deallocations when splines are created
            // and destroyed frequently in an application.
            //   Real* dt               : numSegments
            //   Real* dt2              : numSegments
            //   Vector<N,Real>* alpha  : numSegments
            //   Real* ell              : numSegments + 1
            //   Real* mu               : numSegments
            //   Vector<N,Real>*z       : numSegments + 1
            size_t storageSize =
                numS +
                numS +
                numS +
                static_cast<size_t>(N) * numP +
                numS +
                static_cast<size_t>(N) * numP;

            std::vector<Real> storage(storageSize);
            auto dt = storage.data();
            auto d2t = dt + numS;
            auto alpha = reinterpret_cast<Vector<N, Real>*>(d2t + numS);
            auto ell = reinterpret_cast<Real*>(alpha + numS);
            auto mu = ell + numP;
            auto z = reinterpret_cast<Vector<N, Real>*>(mu + numS);

            Real const r0 = static_cast<Real>(0);
            Real const r1 = static_cast<Real>(1);
            Real const r2 = static_cast<Real>(2);
            Real const r3 = static_cast<Real>(3);

            for (size_t i = 0, ip1 = 1; i < numS; ++i, ++ip1)
            {
                dt[i] = this->mTime[ip1] - this->mTime[i];
            }

            d2t[0] = r0;  // unused
            for (size_t im1 = 0, i = 1, ip1 = 2; i < numS; im1 = i, i = ip1++)
            {
                d2t[i] = this->mTime[ip1] - this->mTime[im1];
            }

            alpha[0].MakeZero();  // unused
            for (size_t im1 = 0, i = 1, ip1 = 2; i < numS; im1 = i, i = ip1++)
            {
                auto numer = r3 * (dt[im1] * mA[ip1] - d2t[i] * mA[i] + dt[i] * mA[im1]);
                Real denom = dt[im1] * dt[i];
                alpha[i] = numer / denom;
            }

            ell[0] = r1;
            mu[0] = r0;
            z[0].MakeZero();
            for (size_t im1 = 0, i = 1; i < numS; im1 = i++)
            {
                ell[i] = r2 * d2t[i] - dt[im1] * mu[im1];
                mu[i] = dt[i] / ell[i];
                z[i] = (alpha[i] - dt[im1] * z[im1]) / ell[i];
            }
            ell[numS] = r1;
            z[numS].MakeZero();

            mC[numS].MakeZero();
            for (size_t j = 0, i = numSm1; j < numS; ++j, --i)
            {
                mC[i] = z[i] - mu[i] * mC[i + 1];
                mB[i] =  (mA[i + 1] - mA[i]) / dt[i] - dt[i] * (mC[i + 1] + r2 * mC[i]) / r3;
                mD[i] = (mC[i + 1] - mC[i]) / (r3 * dt[i]);
            }
        }

        void CreateClosed()
        {
            size_t const numP = mNumPoints;
            size_t const numS = mNumSegments;
            size_t const numSm1 = mNumSegments - 1;

            // Minimize allocation and deallocations when splines are created
            // and destroyed frequently in an application. The matrices mat
            // and invMat are stored in row-major order.
            //   Real* dt       : numSegments
            //   Real* mat      : (numSegments + 1) * (numSegments + 1)
            //   Real* solution : (numSegments + 1) * N
            size_t storageSize =
                numS +
                numP * numP +
                static_cast<size_t>(N) * numP;

            std::vector<Real> storage(storageSize);
            auto dt = storage.data();
            auto mat = dt + numS;
            auto solution = mat + numP * numP;

            Real const r1 = static_cast<Real>(1);
            Real const r2 = static_cast<Real>(2);
            Real const r3 = static_cast<Real>(3);

            for (size_t i = 0, ip1 = 1; i < numS; ++i, ++ip1)
            {
                dt[i] = this->mTime[ip1] - this->mTime[i];
            }

            // Construct matrix of system.
            mat[0 + numP * 0] = r1;  // mat(0,0)
            mat[numS + numP * 0] = -r1;  // mat(0,numS)
            for (size_t im1 = 0, i = 1, ip1 = 2; i <= numSm1; im1 = i, i = ip1++)
            {
                mat[im1 + numP * i] = dt[im1];  // mat(i,im1)
                mat[i   + numP * i] = r2 * (dt[im1] + dt[i]);  // mat(i, i)
                mat[ip1 + numP * i] = dt[i];  // mat(i,ip1)
            }
            mat[numSm1 + numP * numS] = dt[numSm1];  // mat(numS,numSm1)
            mat[0 + numP * numS] = r2 * (dt[numSm1] + dt[0]);  // mat(numS,0)
            mat[1 + numP * numS] = dt[0];  // mat(numS,1)

            // Construct right-hand side of system.
            mC[0].MakeZero();
            for (size_t im1 = 0, i = 1, ip1 = 2; ip1 <= numS; im1 = i, i = ip1++)
            {
                mC[i] = r3 * ((mA[ip1] - mA[i]) / dt[i] - (mA[i] - mA[im1]) / dt[im1]);
            }
            mC[numS] = r3 * ((mA[1] - mA[0]) / dt[0] - (mA[0] - mA[numSm1]) / dt[numSm1]);

            // Solve the linear systems.
            bool solved = LinearSystem<Real>::Solve(static_cast<int32_t>(numP),
                N, mat, reinterpret_cast<Real const*>(mC), solution);
            LogAssert(
                solved,
                "Failed to solve linear system.");

            for (size_t i = 0, k = 0; i <= numS; ++i)
            {
                for (int32_t j = 0; j < N; ++j, ++k)
                {
                    mC[i][j] = solution[k];
                }
            }

            for (size_t i = 0; i < numS; ++i)
            {
                mB[i] = (mA[i + 1] - mA[i]) / dt[i] - (mC[i + 1] + r2 * mC[i]) * dt[i] / r3;
                mD[i] = (mC[i + 1] - mC[i]) / (r3 * dt[i]);
            }
        }

        void CreateClamped(Vector<N, Real> const& derivative0, Vector<N, Real> const& derivative1)
        {
            size_t const numP = mNumPoints;
            size_t const numS = mNumSegments;
            size_t const numSm1 = mNumSegments - 1;

            // Minimize allocation and deallocations when splines are created
            // and destroyed frequently in an application.
            //   Real* dt               : numSegments
            //   Real* dt2              : numSegments
            //   Vector<N,Real>* alpha  : numSegments + 1
            //   Real* ell              : numSegments + 1
            //   Real* mu               : numSegments
            //   Vector<N,Real>*z       : numSegments + 1
            size_t storageSize =
                numS +
                numS +
                static_cast<size_t>(N) * numP +
                numP +
                numS +
                static_cast<size_t>(N) * numP;

            std::vector<Real> storage(storageSize);
            auto dt = storage.data();
            auto d2t = dt + numS;
            auto alpha = reinterpret_cast<Vector<N, Real>*>(d2t + numS);
            auto ell = reinterpret_cast<Real*>(alpha + numS + 1);
            auto mu = ell + numS + 1;
            auto z = reinterpret_cast<Vector<N, Real>*>(mu + numS);

            Real const r2 = static_cast<Real>(2);
            Real const r3 = static_cast<Real>(3);
            Real const rHalf = static_cast<Real>(0.5);

            for (size_t i = 0, ip1 = 1; i < numS; i = ip1++)
            {
                dt[i] = this->mTime[ip1] - this->mTime[i];
            }

            for (size_t im1 = 0, i = 1, ip1 = 2; i < numS; im1 = i, i = ip1++)
            {
                d2t[i] = this->mTime[ip1] - this->mTime[im1];
            }

            alpha[0] = r3 * ((mA[1] - mA[0]) / dt[0] - derivative0);
            alpha[numS] = r3 * (derivative1 -
                (mA[numS] - mA[numS - 1]) / dt[numS - 1]);
            for (size_t im1 = 0, i = 1, ip1 = 2; i < numS; im1 = i, i = ip1++)
            {
                auto numer = r3 * (dt[im1] * mA[ip1] - d2t[i] * mA[i] + dt[i] * mA[im1]);
                Real denom = dt[im1] * dt[i];
                alpha[i] = numer / denom;
            }

            ell[0] = r2 * dt[0];
            mu[0] = rHalf;
            z[0] = alpha[0] / ell[0];
            for (size_t im1 = 0, i = 1; i < numS; im1 = i++)
            {
                ell[i] = r2 * d2t[i] - dt[im1] * mu[im1];
                mu[i] = dt[i] / ell[i];
                z[i] = (alpha[i] - dt[im1] * z[im1]) / ell[i];
            }
            ell[numS] = dt[numSm1] * (r2 - mu[numSm1]);
            z[numS] = (alpha[numS] - dt[numSm1] * z[numSm1]) / ell[numS];

            mC[numS] = z[numS];
            for (size_t j = 0, i = numSm1; j < numS; ++j, --i)
            {
                mC[i] = z[i] - mu[i] * mC[i + 1];
                mB[i] = (mA[i + 1] - mA[i]) / dt[i] - dt[i] * (mC[i + 1] + r2 * mC[i]) / r3;
                mD[i] = (mC[i + 1] - mC[i]) / (r3 * dt[i]);
            }
        }

        // Determine the index i for which times[i] <= t < times[i+1].
        void GetKeyInfo(Real t, size_t& key, Real& dt) const
        {
            if (t <= this->mTime[0])
            {
                key = 0;
                dt = static_cast<Real>(0);
            }
            else if (t >= this->mTime[mNumSegments])
            {
                key = mNumSegments - 1;
                dt = this->mTime[mNumSegments] - this->mTime[mNumSegments - 1];
            }
            else
            {
                for (size_t i = 0, ip1 = 1; i < mNumSegments; i = ip1++)
                {
                    if (t < this->mTime[ip1])
                    {
                        key = i;
                        dt = t - this->mTime[i];
                        break;
                    }
                }
            }
        }

        // Polynomial coefficients. mA are the points (constant coefficients of
        // polynomials. mB are the degree 1 coefficients, mC are the degree 2
        // coefficients and mD are the degree 3 coefficients.
        size_t mNumPoints, mNumSegments;
        Vector<N, Real>* mA;
        Vector<N, Real>* mB;
        Vector<N, Real>* mC;
        Vector<N, Real>* mD;

    private:
        std::vector<Vector<N, Real>> mCoefficients;
    };
}
