// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the tension-continuity-bias (TCB) spline for a set of key frames.
// The algorithm was invented by Kochanek and Bartels and is described in
// https://www.geometrictools.com/Documentation/KBSplines.pdf

#include <Mathematics/Logger.h>
#include <Mathematics/ParametricCurve.h>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <int32_t N, typename T>
    class TCBSplineCurve : public ParametricCurve<N, T>
    {
    public:
        // The inputs point[], time[], tension[], continuity[] and bias[] must
        // have the same number of elements n >= 2. If you want the speed to be
        // continuous for the entire spline, the input lambda[] must have n
        // elements that are all positive; otherwise lambda[] should have 0
        // elements. If you want to specify the outgoing tangent at time[0]
        // and the incoming tangent at time[n-1], pass nonnull pointers for
        // those parameters; otherwise, the boundary tangents are computed by
        // internally duplicating the boundary points, which effectively means
        // point[-1] = point[0] and point[n] = point[n-1].
        TCBSplineCurve(
            std::vector<Vector<N, T>> const& point,
            std::vector<T> const& time,
            std::vector<T> const& tension,
            std::vector<T> const& continuity,
            std::vector<T> const& bias,
            std::vector<T> const& lambda,
            Vector<N, T> const* firstOutTangent,
            Vector<N, T> const* lastInTangent)
            :
            ParametricCurve<N, T>(
                (point.size() >= 2 ? static_cast<int32_t>(point.size() - 1) : 0),
                time.data()),
            mPoint(point),
            mTension(tension),
            mContinuity(continuity),
            mBias(bias),
            mLambda(lambda),
            mInTangent(point.size()),
            mOutTangent(point.size()),
            mA(this->GetNumSegments()),
            mB(this->GetNumSegments()),
            mC(this->GetNumSegments()),
            mD(this->GetNumSegments())
        {
            LogAssert(
                point.size() >= 2 &&
                time.size() == point.size() &&
                tension.size() == point.size() &&
                continuity.size() == point.size() &&
                bias.size() == point.size() &&
                (lambda.size() == 0 || lambda.size() == point.size()),
                "Invalid size in TCBSpline constructor.");

            ComputeFirstTangents(firstOutTangent);
            ComputeInteriorTangents();
            ComputeLastTangents(lastInTangent);
            ComputeCoefficients();
        }

        virtual ~TCBSplineCurve() = default;


        // Member access.
        inline size_t GetNumKeyFrames() const
        {
            return mPoint.size();
        }

        inline std::vector<Vector<N, T>> const& GetPoints() const
        {
            return mPoint;
        }

        inline std::vector<T> const& GetTensions() const
        {
            return mTension;
        }

        inline std::vector<T> const& GetContinuities() const
        {
            return mContinuity;
        }

        inline std::vector<T> const& GetBiases() const
        {
            return mBias;
        }

        inline std::vector<T> const& GetLambdas() const
        {
            return mLambda;
        }

        inline std::vector<Vector<N, T>> const& GetInTangents() const
        {
            return mInTangent;
        }

        inline std::vector<Vector<N, T>> const& GetOutTangents() const
        {
            return mOutTangent;
        }

        // Evaluation of the curve. It is required that order <= 3, which
        // allows computing derivatives through order 3. If you want only the
        // position, pass in order of 0. If you want the position and first
        // derivative, pass in order of 1, and so on. The output array 'jet'
        // must have enough storage to support the specified order. The values
        // are ordered as: position, first derivative, second derivative, and
        // so on.
        virtual void Evaluate(T t, uint32_t order, Vector<N, T>* jet) const override
        {
            size_t key = 0;
            T u = static_cast<T>(0);
            GetKeyInfo(t, key, u);

            // Compute the position.
            jet[0] = mA[key] + u * (mB[key] + u * (mC[key] + u * mD[key]));
            if (order >= 1)
            {
                // Compute the first-order derivative.
                T delta = this->mTime[key + 1] - this->mTime[key];
                jet[1] = mB[key] + u * (static_cast<T>(2) * mC[key] + (static_cast<T>(3) * u) * mD[key]);
                jet[1] /= delta;
                if (order >= 2)
                {
                    // Compute the second-order derivative.
                    T deltaSqr = delta * delta;
                    jet[2] = static_cast<T>(2) * mC[key] + (static_cast<T>(6) * u) * mD[key];
                    jet[2] /= deltaSqr;
                    if (order == 3)
                    {
                        T deltaCub = deltaSqr * delta;
                        jet[3] = static_cast<T>(6) * mD[key];
                        jet[3] /= deltaCub;
                    }
                }
            }
        }

    protected:
        // Support for construction.
        void ComputeFirstTangents(Vector<N, T> const* firstOutTangent)
        {
            if (firstOutTangent != nullptr)
            {
                mOutTangent[0] = *firstOutTangent;
            }
            else
            {
                T omT = static_cast<T>(1) - mTension[0];
                T omC = static_cast<T>(1) - mContinuity[0];
                T omB = static_cast<T>(1) - mBias[0];
                T twoDelta = static_cast<T>(2) * (this->mTime[1] - this->mTime[0]);
                T coeff = omT * omC * omB / twoDelta;
                mOutTangent[0] = coeff * (mPoint[1] - mPoint[0]);
            }

            if (mLambda.size() > 0)
            {
                mOutTangent[0] *= mLambda[0];
            }

            mInTangent[0] = mOutTangent[0];
        }

        void ComputeLastTangents(Vector<N, T> const* lastInTangent)
        {
            size_t const nm1 = mPoint.size() - 1;
            if (lastInTangent != nullptr)
            {
                mInTangent[nm1] = *lastInTangent;
            }
            else
            {
                size_t const nm2 = nm1 - 1;
                T omT = static_cast<T>(1) - mTension[nm1];
                T omC = static_cast<T>(1) - mContinuity[nm1];
                T opB = static_cast<T>(1) + mBias[nm1];
                T twoDelta = static_cast<T>(2) * (this->mTime[nm1] - this->mTime[nm2]);
                T coeff = omT * omC * opB / twoDelta;
                mInTangent[nm1] = coeff * (mPoint[nm1] - mPoint[nm2]);
            }

            if (mLambda.size() > 0)
            {
                mInTangent[nm1] *= mLambda[nm1];
            }

            mOutTangent[nm1] = mInTangent[nm1];
        }

        void ComputeInteriorTangents()
        {
            size_t const n = mPoint.size();
            for (size_t km1 = 0, k = 1, kp1 = 2; kp1 < n; km1 = k, k = kp1++)
            {
                Vector<N, T> const& P0 = mPoint[km1];
                Vector<N, T> const& P1 = mPoint[k];
                Vector<N, T> const& P2 = mPoint[kp1];
                Vector<N, T> P1mP0 = P1 - P0;
                Vector<N, T> P2mP1 = P2 - P1;
                T omT = static_cast<T>(1) - mTension[k];
                T omC = static_cast<T>(1) - mContinuity[k];
                T opC = static_cast<T>(1) + mContinuity[k];
                T omB = static_cast<T>(1) - mBias[k];
                T opB = static_cast<T>(1) + mBias[k];
                T twoDelta0 = static_cast<T>(2) * (this->mTime[k] - this->mTime[km1]);
                T twoDelta1 = static_cast<T>(2) * (this->mTime[kp1] - this->mTime[k]);
                T inCoeff0 = omT * omC * opB / twoDelta0;
                T inCoeff1 = omT * opC * omB / twoDelta1;
                T outCoeff0 = omT * opC * opB / twoDelta0;
                T outCoeff1 = omT * omC * omB / twoDelta1;
                mInTangent[k] = inCoeff0 * P1mP0 + inCoeff1 * P2mP1;
                mOutTangent[k] = outCoeff0 * P1mP0 + outCoeff1 * P2mP1;
            }

            if (mLambda.size() > 0)
            {
                for (size_t k = 1, kp1 = 2; kp1 < n; k = kp1++)
                {
                    T inLength = Length(mInTangent[k]);
                    T outLength = Length(mOutTangent[k]);
                    T common = static_cast<T>(2) * mLambda[k] / (inLength + outLength);
                    T inCoeff = outLength * common;
                    T outCoeff = inLength * common;
                    mInTangent[k] *= inCoeff;
                    mOutTangent[k] *= outCoeff;
                }
            }
        }

        void ComputeCoefficients()
        {
            for (size_t k = 0, kp1 = 1; kp1 < mPoint.size(); k = kp1++)
            {
                auto const& P0 = mPoint[k];
                auto const& P1 = mPoint[kp1];
                auto const& TOut0 = mOutTangent[k];
                auto const& TIn1 = mInTangent[kp1];
                Vector<N, T> P1mP0 = P1 - P0;
                T delta = this->mTime[kp1] - this->mTime[k];
                mA[k] = P0;
                mB[k] = delta * TOut0;
                mC[k] = static_cast<T>(3) * P1mP0 - delta * (static_cast<T>(2) * TOut0 + TIn1);
                mD[k] = static_cast<T>(-2) * P1mP0 + delta * (TOut0 + TIn1);
            }
        }

        // Determine the index i for which time[i] <= t < time[i+1]. The
        // returned value is u is in [0,1].
        void GetKeyInfo(T const& t, size_t& key, T& u) const
        {
            auto const* time = this->mTime.data();
            if (t <= time[0])
            {
                key = 0;
                u = static_cast<T>(0);
                return;
            }

            size_t const numSegments = mA.size();
            if (t < time[numSegments])
            {
                for (size_t i = 0; i < numSegments; ++i)
                {
                    if (t < time[i + 1])
                    {
                        key = i;
                        u = (t - time[i]) / (time[i + 1] - time[i]);
                        return;
                    }
                }
            }

            key = numSegments - 1;
            u = static_cast<T>(1);
        }

        // The constructor inputs.
        std::vector<Vector<N, T>> mPoint;
        std::vector<T> mTension, mContinuity, mBias, mLambda;

        // Tangent vectors derived from the constructor inputs.
        std::vector<Vector<N, T>> mInTangent;
        std::vector<Vector<N, T>> mOutTangent;

        // Polynomial coefficients. The mA[] are the degree 0 coefficients,
        // the mB[] are the degree 1 coefficients, the mC[] are the degree 2
        // coefficients and the mD[] are the degree 3 coefficients.
        std::vector<Vector<N, T>> mA, mB, mC, mD;
    };
}
