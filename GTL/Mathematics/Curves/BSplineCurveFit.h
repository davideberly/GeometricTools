// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// The algorithm implemented here is based on the document
// https://www.geometrictools.com/Documentation/BSplineCurveLeastSquaresFit.pdf

#include <GTL/Utility/Multiarray.h>
#include <GTL/Mathematics/Curves/BSplineCurve.h>
#include <GTL/Mathematics/MatrixAnalysis/BandedMatrix.h>
#include <GTL/Mathematics/Algebra/Vector.h>

namespace gtl
{
    template <typename T, size_t N>
    class BSplineCurveFit
    {
    public:
        // The preconditions for calling Execute are
        // 1 <= degree && degree < numControls <= numSamples - degree - 1
        // and samples is not null and points to an array of N-dimensional
        // points.
        static void Execute(size_t degree, size_t numControls,
            std::vector<Vector<T, N>> const& samples, BSplineCurve<T, N>& spline)
        {
            static_assert(
                N >= 1,
                "Invalid dimension.");

            size_t const numSamples = samples.size();
            GTL_ARGUMENT_ASSERT(
                1 <= degree && degree < numControls &&
                numControls + degree + 1 <= numSamples,
                "Invalid argument.");

            typename BasisFunction<T>::Input input{};
            input.numControls = numControls;
            input.degree = degree;
            input.uniform = true;
            input.periodic = false;
            input.uniqueKnots.resize(numControls - degree + 1);
            input.uniqueKnots[0].t = C_<T>(0);
            input.uniqueKnots[0].multiplicity = degree + 1;
            size_t last = input.uniqueKnots.size() - 1;
            T factor = C_<T>(1) / static_cast<T>(last);
            for (size_t i = 1; i < last; ++i)
            {
                input.uniqueKnots[i].t = factor * static_cast<T>(i);
                input.uniqueKnots[i].multiplicity = 1;
            }
            input.uniqueKnots[last].t = C_<T>(1);
            input.uniqueKnots[last].multiplicity = degree + 1;

            spline = BSplineCurve<T, N>(input, nullptr);
            BasisFunction<T> const* basisFunction = &spline.GetBasisFunction();

            // Fit the data points with a B-spline curve using a least-squares
            // error metric. The problem is of the form A^T*A*Q = A^T*P,
            // where A^T*A is a banded matrix, P contains the sample data, and
            // Q is the unknown vector of control points.
            T tMultiplier = C_<T>(1) / static_cast<T>(numSamples - 1);
            T t;
            size_t i0, i1, i2, imin = 0, imax = 0;

            // Construct the matrix A^T*A.
            size_t numBands = (numControls > degree + 1 ? degree + 1 : degree);
            BandedMatrix<T> ATAMat(numControls, numBands, numBands);
            for (i0 = 0; i0 < numControls; ++i0)
            {
                for (i1 = 0; i1 < i0; ++i1)
                {
                    ATAMat(i0, i1) = ATAMat(i1, i0);
                }

                size_t i1Max = i0 + degree;
                if (i1Max >= numControls)
                {
                    i1Max = numControls - 1;
                }

                for (i1 = i0; i1 <= i1Max; ++i1)
                {
                    T value = C_<T>(0);
                    for (i2 = 0; i2 < numSamples; ++i2)
                    {
                        t = tMultiplier * static_cast<T>(i2);
                        basisFunction->Evaluate(t, 0, imin, imax);
                        if (imin <= i0 && i0 <= imax && imin <= i1 && i1 <= imax)
                        {
                            T b0 = basisFunction->GetValue(0, i0);
                            T b1 = basisFunction->GetValue(0, i1);
                            value += b0 * b1;
                        }
                    }
                    ATAMat(i0, i1) = value;
                }
            }

            // Construct the matrix A^T.
            Multiarray<T, false> ATMat{ numControls, numSamples };
            ATMat.fill(C_<T>(0));
            for (i0 = 0; i0 < numControls; ++i0)
            {
                for (i1 = 0; i1 < numSamples; ++i1)
                {
                    t = tMultiplier * static_cast<T>(i1);
                    basisFunction->Evaluate(t, 0, imin, imax);
                    if (imin <= i0 && i0 <= imax)
                    {
                        ATMat(i0, i1) = basisFunction->GetValue(0, i0);
                    }
                }
            }

            // Compute X0 = (A^T*A)^{-1}*A^T by solving the linear system
            // A^T*A*X = A^T.
            bool solved = ATAMat.SolveSystem(ATMat.data(), numSamples);
            GTL_RUNTIME_ASSERT(
                solved,
                "Failed to solve linear system.");

            // The control points for the fitted curve are stored in the
            // vector Q = X0*P, where P is the vector of sample data.
            Vector<T, N>* controls = spline.GetControls();
            for (i0 = 0; i0 < numControls; ++i0)
            {
                Vector<T, N> sum{};
                for (i1 = 0; i1 < numSamples; ++i1)
                {
                    T xValue = ATMat(i0, i1);
                    sum += xValue * samples[i1];
                }
                controls[i0] = sum;
            }
        }
    };
}
