// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// The algorithm implemented here is based on the document
// https://www.geometrictools.com/Documentation/BSplineSurfaceLeastSquaresFit.pdf

#include <GTL/Utility/Multiarray.h>
#include <GTL/Mathematics/Surfaces/BSplineSurface.h>
#include <GTL/Mathematics/MatrixAnalysis/BandedMatrix.h>
#include <GTL/Mathematics/Algebra/Vector.h>

namespace gtl
{
    template <typename T, size_t N>
    class BSplineSurfaceFit
    {
    public:
        // The preconditions for calling Execute are
        //   1 <= degree0 && degree0 + 1 < numControls0 <= numSamples0
        //   1 <= degree1 && degree1 + 1 < numControls1 <= numSamples1
        // and samples is not null and points to a 2D array of N-dimensional
        // points stored in row-major order.
        static void Execute(
            std::array<size_t, 2> const& degree,
            std::array<size_t, 2> const& numControls,
            std::array<size_t, 2> const& numSamples,
            std::vector<Vector<T, N>> const& samples, BSplineSurface<T, N>& spline)
        {
            for (size_t dim = 0; dim < 2; ++dim)
            {
                GTL_ARGUMENT_ASSERT(
                    1 <= degree[dim] && degree[dim] < numControls[dim] &&
                    numControls[dim] <= numSamples[dim],
                    "Invalid argument.");
            }
            GTL_ARGUMENT_ASSERT(
                samples.size() == numSamples[0] * numSamples[1],
                "Invalid argument.");

            std::array<typename BasisFunction<T>::Input, 2> input{};
            for (size_t dim = 0; dim < 2; ++dim)
            {
                input[dim].numControls = numControls[dim];
                input[dim].degree = degree[dim];
                input[dim].uniform = true;
                input[dim].periodic = false;
                input[dim].uniqueKnots.resize(numControls[dim] - degree[dim] + 1);
                input[dim].uniqueKnots[0].t = C_<T>(0);
                input[dim].uniqueKnots[0].multiplicity = degree[dim] + 1;
                size_t last = input[dim].uniqueKnots.size() - 1;
                T factor = C_<T>(1) / static_cast<T>(last);
                for (size_t i = 1; i < last; ++i)
                {
                    input[dim].uniqueKnots[i].t = factor * static_cast<T>(i);
                    input[dim].uniqueKnots[i].multiplicity = 1;
                }
                input[dim].uniqueKnots[last].t = C_<T>(1);
                input[dim].uniqueKnots[last].multiplicity = degree[dim] + 1;
            }

            spline = BSplineSurface<T, N>(input, nullptr);
            std::array<BasisFunction<T> const*, 2> basisFunction
            {
                &spline.GetBasisFunction(0),
                &spline.GetBasisFunction(1)
            };

            // Fit the data points with a B-spline surface using a
            // least-squares error metric. The problem is of the form
            // A0^T*A0*Q*A1^T*A1 = A0^T*P*A1, where A0^T*A0 and A1^T*A1 are
            // banded matrices, P contains the sample data, and Q is the
            // unknown matrix of control points.
            std::array<T, 2> tMultiplier
            {
                C_<T>(1) / static_cast<T>(numSamples[0] - 1),
                C_<T>(1) / static_cast<T>(numSamples[1] - 1)
            };
            T t;
            size_t i0, i1, i2, imin = 0, imax = 0;

            // Construct the matrices A0^T*A0 and A1^T*A1.
            std::array<size_t, 2> numBands
            {
                numControls[0] > degree[0] + 1 ? degree[0] + 1 : degree[0],
                numControls[1] > degree[1] + 1 ? degree[1] + 1 : degree[1]
            };
            std::array<BandedMatrix<T>, 2> ATAMat =
            {
                BandedMatrix<T>(numControls[0], numBands[0], numBands[0]),
                BandedMatrix<T>(numControls[1], numBands[1], numBands[1])
            };

            for (size_t dim = 0; dim < 2; ++dim)
            {
                for (i0 = 0; i0 < numControls[dim]; ++i0)
                {
                    for (i1 = 0; i1 < i0; ++i1)
                    {
                        ATAMat[dim](i0, i1) = ATAMat[dim](i1, i0);
                    }

                    size_t i1Max = i0 + degree[dim];
                    if (i1Max >= numControls[dim])
                    {
                        i1Max = numControls[dim] - 1;
                    }

                    for (i1 = i0; i1 <= i1Max; ++i1)
                    {
                        T value = C_<T>(0);
                        for (i2 = 0; i2 < numSamples[dim]; ++i2)
                        {
                            t = tMultiplier[dim] * static_cast<T>(i2);
                            basisFunction[dim]->Evaluate(t, 0, imin, imax);
                            if (imin <= i0 && i0 <= imax && imin <= i1 && i1 <= imax)
                            {
                                T b0 = basisFunction[dim]->GetValue(0, i0);
                                T b1 = basisFunction[dim]->GetValue(0, i1);
                                value += b0 * b1;
                            }
                        }
                        ATAMat[dim](i0, i1) = value;
                    }
                }
            }

            // Construct the matrices A0^T and A1^T.
            std::array<Multiarray<T, false>, 2> ATMat;
            for (size_t dim = 0; dim < 2; dim++)
            {
                ATMat[dim] = Multiarray<T, false>{ numControls[dim], numSamples[dim] };
                ATMat[dim].fill(C_<T>(0));
                for (i0 = 0; i0 < numControls[dim]; ++i0)
                {
                    for (i1 = 0; i1 < numSamples[dim]; ++i1)
                    {
                        t = tMultiplier[dim] * static_cast<T>(i1);
                        basisFunction[dim]->Evaluate(t, 0, imin, imax);
                        if (imin <= i0 && i0 <= imax)
                        {
                            ATMat[dim](i0, i1) = basisFunction[dim]->GetValue(0, i0);
                        }
                    }
                }
            }

            // Compute X0 = (A0^T*A0)^{-1}*A0^T and X1 = (A1^T*A1)^{-1}*A1^T
            // by solving the linear systems A0^T*A0*X0 = A0^T and
            // A1^T*A1*X1 = A1^T.
            for (size_t dim = 0; dim < 2; ++dim)
            {
                bool solved = ATAMat[dim].SolveSystem(ATMat[dim].data(), numSamples[dim]);
                GTL_RUNTIME_ASSERT(
                    solved,
                    "Failed to solve linear system.");
            }

            // The control points for the fitted surface are stored in the
            // matrix Q = X0*P*X1^T, where P is the matrix of sample data.
            Vector<T, N>* controls = spline.GetControls();
            for (i1 = 0; i1 < numControls[1]; ++i1)
            {
                for (i0 = 0; i0 < numControls[0]; ++i0)
                {
                    Vector<T, N> sum{};
                    for (size_t j1 = 0; j1 < numSamples[1]; ++j1)
                    {
                        T x1Value = ATMat[1](i1, j1);
                        for (size_t j0 = 0; j0 < numSamples[0]; ++j0)
                        {
                            T x0Value = ATMat[0](i0, j0);
                            sum += (x0Value * x1Value) * samples[j0 + numSamples[0] * j1];
                        }
                    }
                    controls[i0 + numControls[0] * i1] = sum;
                }
            }
        }
    };
}
