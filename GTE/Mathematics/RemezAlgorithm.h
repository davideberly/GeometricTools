// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

#include <Mathematics/Constants.h>
#include <Mathematics/Polynomial1.h>
#include <Mathematics/Logger.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <vector>

namespace gte
{
    template <typename T>
    class RemezAlgorithm
    {
    public:
        using Function = std::function<T(T const&)>;

        RemezAlgorithm()
            :
            mF{},
            mFDer{},
            mXMin(0),
            mXMax(0),
            mDegree(0),
            mMaxRemezIterations(0),
            mMaxBisectionIterations(0),
            mMaxBracketIterations(0),
            mPCoefficients{},
            mEstimatedMaxError(0),
            mXNodes{},
            mErrors{},
            mFValues{},
            mUCoefficients{},
            mVCoefficients{},
            mPartition{}
        {
        }

        size_t Execute(Function const& F, Function const& FDer, T const& xMin,
            T const& xMax, size_t degree, size_t maxRemezIterations,
            size_t maxBisectionIterations, size_t maxBracketIterations)
        {
            LogAssert(
                xMin < xMax &&
                degree > 0 &&
                maxRemezIterations > 0 &&
                maxBisectionIterations > 0 &&
                maxBracketIterations > 0,
                "Invalid input.");

            mF = F;
            mFDer = FDer;
            mXMin = xMin;
            mXMax = xMax;
            mDegree = degree;
            mMaxRemezIterations = maxRemezIterations;
            mMaxBisectionIterations = maxBisectionIterations;
            mMaxBracketIterations = maxBracketIterations;

            mPCoefficients.resize(mDegree + 1);
            mEstimatedMaxError = static_cast<T>(0);
            mXNodes.resize(mDegree + 2);
            mErrors.resize(mDegree + 2);

            mFValues.resize(mDegree + 2);
            mUCoefficients.resize(mDegree + 1);
            mVCoefficients.resize(mDegree + 1);
            mEstimatedMaxError = static_cast<T>(0);
            mPartition.resize(mDegree + 3);

            ComputeInitialXNodes();
            size_t iteration{};
            for (iteration = 0; iteration < mMaxRemezIterations; ++iteration)
            {
                ComputeFAtXNodes();
                ComputeUCoefficients();
                ComputeVCoefficients();
                ComputeEstimatedError();
                ComputePCoefficients();
                if (IsOscillatory())
                {
                    ComputePartition();
                    ComputeXExtremes();
                }
                else
                {
                    iteration = std::numeric_limits<size_t>::max();
                    break;
                }
            }
            return iteration;
        }

        // The output of the algorithm.
        inline std::vector<T> const& GetCoefficients() const
        {
            return mPCoefficients;
        }

        inline T GetEstimatedMaxError() const
        {
            return mEstimatedMaxError;
        }

        inline std::vector<T> const& GetXNodes() const
        {
            return mXNodes;
        }

        inline std::vector<T> const& GetErrors() const
        {
            return mErrors;
        }

    private:
        void ComputeInitialXNodes()
        {
            // Get the Chebyshev nodes for the interval [-1,1].
            size_t const numNodes = mXNodes.size();
            T const halfPiDivDegree = static_cast<T>(GTE_C_HALF_PI) / static_cast<T>(mDegree);
            std::vector<T> cosAngles(numNodes);
            cosAngles.front() = -static_cast<T>(1);
            for (size_t i = 1, j = 2 * mDegree - 1; i <= mDegree; ++i, j -= 2)
            {
                T angle = static_cast<T>(j) * halfPiDivDegree;
                cosAngles[i] = std::cos(angle);
            }
            cosAngles.back() = static_cast<T>(1);
            if (numNodes & 1)
            {
                // Avoid the rounding errors when the angle is pi/2, where
                // cos(pi/2) is theoretically zero.
                cosAngles[numNodes / 2] = static_cast<T>(0);
            }

            // Transform the nodes to the interval [xMin, xMax].
            T const half = static_cast<T>(0.5);
            T const center = half * (mXMax + mXMin);
            T const radius = half * (mXMax - mXMin);
            mXNodes.front() = mXMin;
            for (size_t i = 1; i <= mDegree; ++i)
            {
                mXNodes[i] = center + radius * cosAngles[i];
            }
            mXNodes.back() = mXMax;
        }

        void ComputeFAtXNodes()
        {
            for (size_t i = 0; i < mXNodes.size(); ++i)
            {
                mFValues[i] = mF(mXNodes[i]);
            }
        }

        // Compute polynomial u(x) for which u(x[i]) = F(x[i]).
        void ComputeUCoefficients()
        {
            for (size_t i = 0; i < mUCoefficients.size(); ++i)
            {
                mUCoefficients[i] = mFValues[i];
                for (size_t j = 0; j < i; ++j)
                {
                    mUCoefficients[i] -= mUCoefficients[j];
                    mUCoefficients[i] /= mXNodes[i] - mXNodes[j];
                }
            }
        }

        // Compute polynomial v(x) for which v(x[i]) = (-1)^i.
        void ComputeVCoefficients()
        {
            T sign = static_cast<T>(1);
            for (size_t i = 0; i < mVCoefficients.size(); ++i)
            {
                mVCoefficients[i] = sign;
                for (size_t j = 0; j < i; ++j)
                {
                    mVCoefficients[i] -= mVCoefficients[j];
                    mVCoefficients[i] /= mXNodes[i] - mXNodes[j];
                }
                sign = -sign;
            }
        }

        void ComputeEstimatedError()
        {
            T const powNegOne = ((mDegree & 1) ? static_cast<T>(-1) : static_cast<T>(1));
            T const& xBack = mXNodes.back();
            T const& fBack = mFValues.back();
            T uBack = EvaluateU(xBack);
            T vBack = EvaluateV(xBack);
            mEstimatedMaxError = (uBack - fBack) / (vBack + powNegOne);
        }

        void ComputePCoefficients()
        {
            // Compute the P-polynomial symbolically as a Newton polynomial
            // in order to obtain the coefficients from the t-powers.
            std::vector<T> constant(mUCoefficients.size());
            for (size_t i = 0; i < mUCoefficients.size(); ++i)
            {
                constant[i] = mUCoefficients[i] - mEstimatedMaxError * mVCoefficients[i];
            }

            size_t index = mUCoefficients.size() - 1;
            Polynomial1<T> poly{ constant[index--] };
            for (size_t i = 1; i < mUCoefficients.size(); ++i, --index)
            {
                Polynomial1<T> linear{ -mXNodes[index], static_cast<T>(1)};
                poly = constant[index] + linear * poly;
            }

            for (size_t i = 0; i < mPCoefficients.size(); ++i)
            {
                mPCoefficients[i] = poly[static_cast<int32_t>(i)];
            }
        }

        bool IsOscillatory()
        {
            // Compute the errors |F(x)-P(x)| for the current nodes and
            // verify they are oscillatory.
            for (size_t i = 0; i < mXNodes.size(); ++i)
            {
                mErrors[i] = mF(mXNodes[i]) - EvaluateP(mXNodes[i]);
            }

            T const zero = static_cast<T>(0);
            for (size_t i0 = 0, i1 = 1; i1 < mXNodes.size(); i0 = i1++)
            {
                if ((mErrors[i0] > zero && mErrors[i1] > zero) ||
                    (mErrors[i0] < zero && mErrors[i1] < zero))
                {
                    // The process terminates when the errors are not
                    // oscillatory.
                    return false;
                }
            }
            return true;
        }

        void ComputePartition()
        {
            // Define E(x) = F(x) - P(x). Use bisection to compute the roots
            // of E(x). The algorithm partitions [xMin, xMax] into degree+2
            // subintervals, each subinterval with E(x) positive or with E(x)
            // negative. Later, the local extrema on the subintervals are
            // computed using a quadratic-fit line-search algorithm. The
            // extreme locations become the next set of x-nodes.
            T const zero = static_cast<T>(0);
            T const half = static_cast<T>(0.5);
            mPartition.front() = mXMin;
            mPartition.back() = mXMax;
            for (size_t i0 = 0, i1 = 1; i1 < mXNodes.size(); i0 = i1++)
            {
                T x0 = mXNodes[i0], x1 = mXNodes[i1], xMid = zero, eMid = zero;
                int32_t sign0 = (mErrors[i0] > zero ? 1 : -1);
                int32_t sign1 = (mErrors[i1] > zero ? 1 : -1);
                int32_t signMid{};

                size_t iteration{};
                for (iteration = 0; iteration < mMaxBisectionIterations; ++iteration)
                {
                    xMid = half * (x0 + x1);
                    if (xMid == x0 || xMid == x1)
                    {
                        // We are at the limit of floating-point precision for
                        // the average of endpoints.
                        break;
                    }

                    // Update the correct endpoint to the midpoint.
                    eMid = mF(xMid) - EvaluateP(xMid);
                    signMid = (eMid > zero ? 1 : (eMid < zero ? -1 : 0));
                    if (signMid == sign0)
                    {
                        x0 = xMid;
                    }
                    else if (signMid == sign1)
                    {
                        x1 = xMid;
                    }
                    else
                    {
                        // Found a root (numerically rounded to zero).
                        break;
                    }
                }

                // It is possible that the maximum number of bisections was
                // applied without convergence. Use the last computed xMid
                // as the root.
                mPartition[i1] = xMid;
            }
        }

        // Use a quadratic-fit line-search (QFLS) to find the local extrema.
        void ComputeXExtremes()
        {
            std::vector<T> nextXNodes(mXNodes.size());
            nextXNodes[0] = mXMin;
            for (size_t i0 = 1, i1 = 2; i0 < mDegree + 1; i0 = i1++)
            {
                nextXNodes[i0] = GetXExtreme(mPartition[i0], mPartition[i1]);
            }
            nextXNodes[mDegree + 1] = mXMax;
            mXNodes = nextXNodes;
        }

        T GetXExtreme(T x0, T x1)
        {
            T const zero = static_cast<T>(0);
            T const half = static_cast<T>(0.5);
            T eder0 = mFDer(x0) - EvaluatePDer(x0);
            T eder1 = mFDer(x1) - EvaluatePDer(x1);
            int32_t signEDer0 = (eder0 > zero ? 1 : (eder0 < zero ? -1 : 0));
            int32_t signEDer1 = (eder1 > zero ? 1 : (eder1 < zero ? -1 : 0));
            LogAssert(
                signEDer0 * signEDer1 == -1,
                "The interval [x0,x1] does not bound a root.");

            T xmid = zero, ederMid = zero;
            int32_t signEMid = 0;
            for (size_t i = 0; i < mMaxBisectionIterations; ++i)
            {
                xmid = half * (x0 + x1);
                if (xmid == x0 || xmid == x1)
                {
                    return xmid;
                }

                ederMid = mFDer(xmid) - EvaluatePDer(xmid);
                signEMid = (ederMid > zero ? 1 : (ederMid < zero ? -1 : 0));
                if (signEMid == signEDer0)
                {
                    x0 = xmid;
                }
                else if (signEMid == signEDer1)
                {
                    x1 = xmid;
                }
                else
                {
                    break;
                }
            }
            return xmid;
        }

        // Evaluate u(x) =
        //   u[0]+(x-xn[0])*(u[1]+(x-xn[1])*(u[2]+...+(x-xn[n-1])*u[n-1]))
        T EvaluateU(T const& x)
        {
            size_t index = mUCoefficients.size() - 1;
            T result = mUCoefficients[index--];
            for (size_t i = 1; i < mUCoefficients.size(); ++i, --index)
            {
                result = mUCoefficients[index] + (x - mXNodes[index]) * result;
            }
            return result;
        }

        // Evaluate v(x) =
        //   v[0]+(x-xn[0])*(v[1]+(x-xn[1])*(v[2]+...+(x-xn[n-1])*v[n-1]))
        T EvaluateV(T const& x)
        {
            size_t index = mVCoefficients.size() - 1;
            T result = mVCoefficients[index--];
            for (size_t i = 1; i < mVCoefficients.size(); ++i, --index)
            {
                result = mVCoefficients[index] + (x - mXNodes[index]) * result;
            }
            return result;
        }

        // Evaluate p(x) = sum_{i=0}^{n} p[i] * x^i.
        T EvaluateP(T const& x)
        {
            size_t index = mPCoefficients.size() - 1;
            T result = mPCoefficients[index--];
            for (size_t i = 1; i < mPCoefficients.size(); ++i, --index)
            {
                result = mPCoefficients[index] + x * result;
            }
            return result;
        }

        T EvaluatePDer(T const& x)
        {
            size_t index = mPCoefficients.size() - 1;
            T result = static_cast<T>(index) * mPCoefficients[index];
            --index;
            for (size_t i = 2; i < mPCoefficients.size(); ++i, --index)
            {
                result = static_cast<T>(index) * mPCoefficients[index] + x * result;
            }
            return result;
        }

        // Inputs to Execute(...).
        Function mF;
        Function mFDer;
        T mXMin;
        T mXMax;
        size_t mDegree;
        size_t mMaxRemezIterations;
        size_t mMaxBisectionIterations;
        size_t mMaxBracketIterations;

        // Outputs from Execute(...).
        std::vector<T> mPCoefficients;
        T mEstimatedMaxError;
        std::vector<T> mXNodes;
        std::vector<T> mErrors;

        // Members used in the intermediate computations.
        std::vector<T> mFValues;
        std::vector<T> mUCoefficients;
        std::vector<T> mVCoefficients;
        std::vector<T> mPartition;
    };
}
