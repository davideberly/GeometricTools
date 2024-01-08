// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// See GaussNewtonMinimizer.h for a formulation of the minimization
// problem and how Levenberg-Marquardt relates to Gauss-Newton.

#include <Mathematics/CholeskyDecomposition.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <utility>

namespace gte
{
    template <typename T>
    class LevenbergMarquardtMinimizer
    {
    public:
        // Convenient types for the domain vectors, the range vectors, the
        // function F and the Jacobian J.
        typedef GVector<T> DVector;  // numPDimensions
        typedef GVector<T> RVector;  // numFDImensions
        typedef GMatrix<T> JMatrix;  // numFDimensions-by-numPDimensions
        typedef GMatrix<T> JTJMatrix;  // numPDimensions-by-numPDimensions
        typedef GVector<T> JTFVector;  // numPDimensions
        typedef std::function<void(DVector const&, RVector&)> FFunction;
        typedef std::function<void(DVector const&, JMatrix&)> JFunction;
        typedef std::function<void(DVector const&, JTJMatrix&, JTFVector&)> JPlusFunction;

        // Create the minimizer that computes F(p) and J(p) directly.
        LevenbergMarquardtMinimizer(int32_t numPDimensions, int32_t numFDimensions,
            FFunction const& inFFunction, JFunction const& inJFunction)
            :
            mNumPDimensions(numPDimensions),
            mNumFDimensions(numFDimensions),
            mFFunction(inFFunction),
            mJFunction(inJFunction),
            mF(mNumFDimensions),
            mJ(mNumFDimensions, mNumPDimensions),
            mJTJ(mNumPDimensions, mNumPDimensions),
            mNegJTF(mNumPDimensions),
            mDecomposer(mNumPDimensions),
            mUseJFunction(true)
        {
            LogAssert(mNumPDimensions > 0 && mNumFDimensions > 0, "Invalid dimensions.");
        }

        // Create the minimizer that computes J^T(p)*J(p) and -J(p)*F(p).
        LevenbergMarquardtMinimizer(int32_t numPDimensions, int32_t numFDimensions,
            FFunction const& inFFunction, JPlusFunction const& inJPlusFunction)
            :
            mNumPDimensions(numPDimensions),
            mNumFDimensions(numFDimensions),
            mFFunction(inFFunction),
            mJPlusFunction(inJPlusFunction),
            mF(mNumFDimensions),
            mJ(mNumFDimensions, mNumPDimensions),
            mJTJ(mNumPDimensions, mNumPDimensions),
            mNegJTF(mNumPDimensions),
            mDecomposer(mNumPDimensions),
            mUseJFunction(false)
        {
            LogAssert(mNumPDimensions > 0 && mNumFDimensions > 0, "Invalid dimensions.");
        }

        // Disallow copy, assignment and move semantics.
        LevenbergMarquardtMinimizer(LevenbergMarquardtMinimizer const&) = delete;
        LevenbergMarquardtMinimizer& operator=(LevenbergMarquardtMinimizer const&) = delete;
        LevenbergMarquardtMinimizer(LevenbergMarquardtMinimizer&&) = delete;
        LevenbergMarquardtMinimizer& operator=(LevenbergMarquardtMinimizer&&) = delete;

        inline int32_t GetNumPDimensions() const { return mNumPDimensions; }
        inline int32_t GetNumFDimensions() const { return mNumFDimensions; }

        // The lambda is positive, the multiplier is positive, and the initial
        // guess for the p-parameter is p0.  Typical choices are lambda =
        // 0.001 and multiplier = 10.  TODO: Explain lambda in more detail,
        // Multiview Geometry mentions lambda = 0.001*average(diagonal(JTJ)),
        // but let's just expose the factor in front of the average.

        struct Result
        {
            Result()
                :
                minLocation{},
                minError(static_cast<T>(0)),
                minErrorDifference(static_cast<T>(0)),
                minUpdateLength(static_cast<T>(0)),
                numIterations(0),
                numAdjustments(0),
                converged(false)
            {
                minLocation.MakeZero();
            }

            DVector minLocation;
            T minError;
            T minErrorDifference;
            T minUpdateLength;
            size_t numIterations;
            size_t numAdjustments;
            bool converged;
        };

        Result operator()(DVector const& p0, size_t maxIterations,
            T updateLengthTolerance, T errorDifferenceTolerance,
            T lambdaFactor, T lambdaAdjust, size_t maxAdjustments)
        {
            Result result{};
            result.minLocation = p0;
            result.minError = std::numeric_limits<T>::max();
            result.minErrorDifference = std::numeric_limits<T>::max();
            result.minUpdateLength = (T)0;
            result.numIterations = 0;
            result.numAdjustments = 0;
            result.converged = false;

            // As a simple precaution, ensure that the lambda inputs are
            // valid.  If invalid, fall back to Gauss-Newton iteration.
            if (lambdaFactor <= (T)0 || lambdaAdjust <= (T)0)
            {
                maxAdjustments = 1;
                lambdaFactor = (T)0;
                lambdaAdjust = (T)1;
            }

            // As a simple precaution, ensure the tolerances are nonnegative.
            updateLengthTolerance = std::max(updateLengthTolerance, (T)0);
            errorDifferenceTolerance = std::max(errorDifferenceTolerance, (T)0);

            // Compute the initial error.
            mFFunction(p0, mF);
            result.minError = Dot(mF, mF);

            // Do the Levenberg-Marquart iterations.
            auto pCurrent = p0;
            for (result.numIterations = 1; result.numIterations <= maxIterations; ++result.numIterations)
            {
                std::pair<bool, bool> status;
                DVector pNext;
                for (result.numAdjustments = 0; result.numAdjustments < maxAdjustments; ++result.numAdjustments)
                {
                    status = DoIteration(pCurrent, lambdaFactor, updateLengthTolerance,
                        errorDifferenceTolerance, pNext, result);
                    if (status.first)
                    {
                        // Either the Cholesky decomposition failed or the
                        // iterates converged within tolerance.  TODO: See the
                        // note in DoIteration about not failing on Cholesky
                        // decomposition.
                        return result;
                    }

                    if (status.second)
                    {
                        // The error has been reduced but we have not yet
                        // converged within tolerance.
                        break;
                    }

                    lambdaFactor *= lambdaAdjust;
                }

                if (result.numAdjustments < maxAdjustments)
                {
                    // The current value of lambda led us to an update that
                    // reduced the error, but the error is not yet small
                    // enough to conclude we converged.  Reduce lambda for the
                    // next outer-loop iteration.
                    lambdaFactor /= lambdaAdjust;
                }
                else
                {
                    // All lambdas tried during the inner-loop iteration did
                    // not lead to a reduced error.  If we do nothing here,
                    // the next inner-loop iteration will continue to multiply
                    // lambda, risking eventual floating-point overflow.  To
                    // avoid this, fall back to a Gauss-Newton iterate.
                    status = DoIteration(pCurrent, lambdaFactor, updateLengthTolerance,
                        errorDifferenceTolerance, pNext, result);
                    if (status.first)
                    {
                        // Either the Cholesky decomposition failed or the
                        // iterates converged within tolerance.  TODO: See the
                        // note in DoIteration about not failing on Cholesky
                        // decomposition.
                        return result;
                    }
                }

                pCurrent = pNext;
            }

            return result;
        }

    private:
        void ComputeLinearSystemInputs(DVector const& pCurrent, T lambda)
        {
            if (mUseJFunction)
            {
                mJFunction(pCurrent, mJ);
                mJTJ = MultiplyATB(mJ, mJ);
                mNegJTF = -(mF * mJ);
            }
            else
            {
                mJPlusFunction(pCurrent, mJTJ, mNegJTF);
            }

            T diagonalSum(0);
            for (int32_t i = 0; i < mNumPDimensions; ++i)
            {
                diagonalSum += mJTJ(i, i);
            }

            T diagonalAdjust = lambda * diagonalSum / static_cast<T>(mNumPDimensions);
            for (int32_t i = 0; i < mNumPDimensions; ++i)
            {
                mJTJ(i, i) += diagonalAdjust;
            }
        }

        // The returned 'first' is true when the linear system cannot be
        // solved (result.converged is false in this case) or when the
        // error is reduced to within the tolerances specified by the caller
        // (result.converged is true in this case).  When the 'first' value
        // is true, the 'second' value is true when the error is reduced or
        // false when it is not.
        std::pair<bool, bool> DoIteration(DVector const& pCurrent, T lambdaFactor,
            T updateLengthTolerance, T errorDifferenceTolerance, DVector& pNext,
            Result& result)
        {
            ComputeLinearSystemInputs(pCurrent, lambdaFactor);
            if (!mDecomposer.Factor(mJTJ))
            {
                // TODO: The matrix mJTJ is positive semi-definite, so the
                // failure can occur when mJTJ has a zero eigenvalue in
                // which case mJTJ is not invertible.  Generate an iterate
                // anyway, perhaps using gradient descent?
                return std::make_pair(true, false);
            }
            mDecomposer.SolveLower(mJTJ, mNegJTF);
            mDecomposer.SolveUpper(mJTJ, mNegJTF);

            pNext = pCurrent + mNegJTF;
            mFFunction(pNext, mF);
            T error = Dot(mF, mF);
            if (error < result.minError)
            {
                result.minErrorDifference = result.minError - error;
                result.minUpdateLength = Length(mNegJTF);
                result.minLocation = pNext;
                result.minError = error;
                if (result.minErrorDifference <= errorDifferenceTolerance
                    || result.minUpdateLength <= updateLengthTolerance)
                {
                    result.converged = true;
                    return std::make_pair(true, true);
                }
                else
                {
                    return std::make_pair(false, true);
                }
            }
            else
            {
                return std::make_pair(false, false);
            }
        }

        int32_t mNumPDimensions, mNumFDimensions;
        FFunction mFFunction;
        JFunction mJFunction;
        JPlusFunction mJPlusFunction;

        // Storage for J^T(p)*J(p) and -J^T(p)*F(p) during the iterations.
        RVector mF;
        JMatrix mJ;
        JTJMatrix mJTJ;
        JTFVector mNegJTF;

        CholeskyDecomposition<T> mDecomposer;

        bool mUseJFunction;
    };
}
