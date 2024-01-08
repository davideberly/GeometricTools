// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The cone vertex is V, the unit-length axis direction is U and the
// cone angle is A in (0,pi/2). The cone is defined algebraically by
// those points X for which
//     Dot(U,X-V)/Length(X-V) = cos(A)
// This can be written as a quadratic equation
//     (V-X)^T * (cos(A)^2 - U * U^T) * (V-X) = 0
// with the implicit constraint that Dot(U, X-V) > 0 (X is on the
// "positive" cone). Define W = U/cos(A), so Length(W) > 1 and
//     F(X;V,W) = (V-X)^T * (I - W * W^T) * (V-X) = 0
// The nonlinear least squares fitting of points {X[i]}_{i=0}^{n-1}
// computes V and W to minimize the error function
//     E(V,W) = sum_{i=0}^{n-1} F(X[i];V,W)^2
// I recommend using the Gauss-Newton minimizer when your cone points
// are truly nearly a cone; otherwise, try the Levenberg-Marquardt
// minimizer.
//
// The mathematics used in this implementation are found in
//   https://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf

#include <Mathematics/ApprHeightLine2.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/GaussNewtonMinimizer.h>
#include <Mathematics/LevenbergMarquardtMinimizer.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

namespace gte
{
    template <typename T>
    class ApprCone3
    {
    public:
        ApprCone3()
            :
            mNumPoints(0),
            mPoints(nullptr)
        {
            // F[i](V,W) = D^T * (I - W * W^T) * D, D = V - X[i], P = (V,W)
            mFFunction = [this](GVector<T> const& P, GVector<T>& F)
            {
                Vector3<T> V = { P[0], P[1], P[2] };
                Vector3<T> W = { P[3], P[4], P[5] };
                for (int32_t i = 0; i < mNumPoints; ++i)
                {
                    Vector3<T> delta = V - mPoints[i];
                    T deltaDotW = Dot(delta, W);
                    F[i] = Dot(delta, delta) - deltaDotW * deltaDotW;
                }
            };

            // dF[i]/dV = 2 * (D - Dot(W, D) * W)
            // dF[i]/dW = -2 * Dot(W, D) * D
            mJFunction = [this](GVector<T> const& P, GMatrix<T>& J)
            {
                T const two = static_cast<T>(2);
                Vector3<T> V = { P[0], P[1], P[2] };
                Vector3<T> W = { P[3], P[4], P[5] };
                for (int32_t row = 0; row < mNumPoints; ++row)
                {
                    Vector3<T> delta = V - mPoints[row];
                    T deltaDotW = Dot(delta, W);
                    Vector3<T> temp0 = delta - deltaDotW * W;
                    Vector3<T> temp1 = deltaDotW * delta;
                    for (int32_t col = 0; col < 3; ++col)
                    {
                        J(row, col) = two * temp0[col];
                        J(row, col + 3) = -two * temp1[col];
                    }
                }
            };
        }

        // If you want to specify that coneVertex, coneAxis and coneAngle
        // are the initial guesses for the minimizer, set the parameter
        // useConeInputAsInitialGuess to 'true'. If you want the function
        // to compute initial guesses, set that parameter to 'false'.
        // A Gauss-Newton minimizer is used to fit a cone using nonlinear
        // least-squares. The fitted cone is returned in coneVertex,
        // coneAxis and coneAngle. See GaussNewtonMinimizer.h for a
        // description of the least-squares algorithm and the parameters
        // that it requires.
        typename GaussNewtonMinimizer<T>::Result
        operator()(int32_t numPoints, Vector3<T> const* points,
            size_t maxIterations, T updateLengthTolerance, T errorDifferenceTolerance,
            bool useConeInputAsInitialGuess,
            Vector3<T>& coneVertex, Vector3<T>& coneAxis, T& coneAngle)
        {
            mNumPoints = numPoints;
            mPoints = points;
            GaussNewtonMinimizer<T> minimizer(6, mNumPoints, mFFunction, mJFunction);

            if (useConeInputAsInitialGuess)
            {
                Normalize(coneAxis);
            }
            else
            {
                ComputeInitialCone(coneVertex, coneAxis, coneAngle);
            }

            // The initial guess for the cone vertex.
            GVector<T> initial(6);
            initial[0] = coneVertex[0];
            initial[1] = coneVertex[1];
            initial[2] = coneVertex[2];

            // The initial guess for the weighted cone axis.
            T coneCosAngle = std::cos(coneAngle);
            initial[3] = coneAxis[0] / coneCosAngle;
            initial[4] = coneAxis[1] / coneCosAngle;
            initial[5] = coneAxis[2] / coneCosAngle;

            auto result = minimizer(initial, maxIterations, updateLengthTolerance,
                errorDifferenceTolerance);

            // No test is made for result.converged so that we return some
            // estimates of the cone. The caller can decide how to respond
            // when result.converged is false.
            for (int32_t i = 0; i < 3; ++i)
            {
                coneVertex[i] = result.minLocation[i];
                coneAxis[i] = result.minLocation[i + 3];
            }

            // We know that coneCosAngle will be nonnegative. The std::min
            // call guards against rounding errors leading to a number
            // slightly larger than 1. The clamping ensures std::acos will
            // not return a NaN.
            T const one = static_cast<T>(1);
            coneCosAngle = std::min(one / Normalize(coneAxis), one);
            coneAngle = std::acos(coneCosAngle);

            mNumPoints = 0;
            mPoints = nullptr;
            return result;
        }

        // The parameters coneVertex, coneAxis and coneAngle are in/out
        // variables. The caller must provide initial guesses for these.
        // The function estimates the cone parameters and returns them. See
        // GaussNewtonMinimizer.h for a description of the least-squares
        // algorithm and the parameters that it requires. (The file
        // LevenbergMarquardtMinimizer.h directs you to the Gauss-Newton
        // file to read about the parameters.)
        typename LevenbergMarquardtMinimizer<T>::Result
        operator()(int32_t numPoints, Vector3<T> const* points,
            size_t maxIterations, T updateLengthTolerance, T errorDifferenceTolerance,
            T lambdaFactor, T lambdaAdjust, size_t maxAdjustments,
            bool useConeInputAsInitialGuess,
            Vector3<T>& coneVertex, Vector3<T>& coneAxis, T& coneAngle)
        {
            mNumPoints = numPoints;
            mPoints = points;
            LevenbergMarquardtMinimizer<T> minimizer(6, mNumPoints, mFFunction, mJFunction);

            if (useConeInputAsInitialGuess)
            {
                Normalize(coneAxis);
            }
            else
            {
                ComputeInitialCone(coneVertex, coneAxis, coneAngle);
            }

            // The initial guess for the cone vertex.
            GVector<T> initial(6);
            initial[0] = coneVertex[0];
            initial[1] = coneVertex[1];
            initial[2] = coneVertex[2];

            // The initial guess for the weighted cone axis.
            T coneCosAngle = std::cos(coneAngle);
            initial[3] = coneAxis[0] / coneCosAngle;
            initial[4] = coneAxis[1] / coneCosAngle;
            initial[5] = coneAxis[2] / coneCosAngle;

            auto result = minimizer(initial, maxIterations, updateLengthTolerance,
                errorDifferenceTolerance, lambdaFactor, lambdaAdjust, maxAdjustments);

            // No test is made for result.converged so that we return some
            // estimates of the cone. The caller can decide how to respond
            // when result.converged is false.
            for (int32_t i = 0; i < 3; ++i)
            {
                coneVertex[i] = result.minLocation[i];
                coneAxis[i] = result.minLocation[i + 3];
            }

            // We know that coneCosAngle will be nonnegative. The std::min
            // call guards against rounding errors leading to a number
            // slightly larger than 1. The clamping ensures std::acos will
            // not return a NaN.
            T const one = static_cast<T>(1);
            coneCosAngle = std::min(one / Normalize(coneAxis), one);
            coneAngle = std::acos(coneCosAngle);

            mNumPoints = 0;
            mPoints = nullptr;
            return result;
        }

    private:
        void ComputeInitialCone(Vector3<T>& coneVertex, Vector3<T>& coneAxis, T& coneAngle)
        {
            // Compute the average of the sample points.
            T const zero = static_cast<T>(0);
            Vector3<T> center{ zero, zero, zero };
            T const tNumPoints = static_cast<T>(mNumPoints);
            for (int32_t i = 0; i < mNumPoints; ++i)
            {
                center += mPoints[i];
            }
            center /= tNumPoints;

            // The cone axis is estimated from ZZTZ (see the PDF).
            coneAxis = { zero, zero, zero };
            for (int32_t i = 0; i < mNumPoints; ++i)
            {
                Vector3<T> delta = mPoints[i] - center;
                coneAxis += delta* Dot(delta, delta);
            }
            Normalize(coneAxis);

            // Compute the signed heights of the points along the cone axis
            // relative to C. These are the projections of the points onto the
            // line C+t*U. Also compute the radial distances of the points
            // from the line C+t*U.
            std::vector<Vector2<T>> hrPairs(mNumPoints);
            T hMin = std::numeric_limits<T>::max(), hMax = -hMin;
            for (int32_t i = 0; i < mNumPoints; ++i)
            {
                Vector3<T> delta = mPoints[i] - center;
                T h = Dot(coneAxis, delta);
                hMin = std::min(hMin, h);
                hMax = std::max(hMax, h);
                Vector3<T> projection = delta - Dot(coneAxis, delta) * coneAxis;
                T r = Length(projection);
                hrPairs[i] = { h, r };
            }

            // The radial distance is considered to be a function of height.
            // Fit the (h,r) pairs with a line:
            //   r - rAverage = hrSlope * (h - hAverage)
            ApprHeightLine2<T> fitter{};
            fitter.Fit(hrPairs);
            auto const& parameters = fitter.GetParameters();
            T hAverage = parameters.first[0];
            T rAverage = parameters.first[1];
            T hrSlope = parameters.second[0];

            // If U is directed so that r increases as h increases, U is the
            // correct cone axis estimate. However, if r decreases as h
            // increases, -U is the correct cone axis estimate.
            if (hrSlope < zero)
            {
                coneAxis = -coneAxis;
                hrSlope = -hrSlope;
                std::swap(hMin, hMax);
                hMin = -hMin;
                hMax = -hMax;
            }

            // Compute the extreme radial distance values for the points.
            T rMin = rAverage + hrSlope * (hMin - hAverage);
            T rMax = rAverage + hrSlope * (hMax - hAverage);
            T hRange = hMax - hMin;
            T rRange = rMax - rMin;

            // Using trigonometry and right triangles, compute the tangent
            // function of the cone angle.
            T tanAngle = rRange / hRange;
            coneAngle = std::atan2(rRange, hRange);

            // Compute the cone vertex.
            T offset = rMax / tanAngle - hMax;
            coneVertex = center - offset * coneAxis;
        }

        int32_t mNumPoints;
        Vector3<T> const* mPoints;
        std::function<void(GVector<T> const&, GVector<T>&)> mFFunction;
        std::function<void(GVector<T> const&, GMatrix<T>&)> mJFunction;
    };
}
