// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The algorithm implemented here is based on the document
// https://www.geometrictools.com/Documentation/BSplineSurfaceLeastSquaresFit.pdf

#include <Mathematics/BandedMatrix.h>
#include <Mathematics/BasisFunction.h>
#include <Mathematics/Vector3.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace gte
{
    template <typename Real>
    class BSplineSurfaceFit
    {
    public:
        // Construction.  The preconditions for calling the constructor are
        //   1 <= degree0 && degree0 + 1 < numControls0 <= numSamples0
        //   1 <= degree1 && degree1 + 1 < numControls1 <= numSamples1
        // The sample data must be in row-major order.  The control data is
        // also stored in row-major order.
        BSplineSurfaceFit(int32_t degree0, int32_t numControls0, int32_t numSamples0,
            int32_t degree1, int32_t numControls1, int32_t numSamples1, Vector3<Real> const* sampleData)
            :
            mSampleData(sampleData),
            mControlData(static_cast<size_t>(numControls0) * static_cast<size_t>(numControls1))
        {
            LogAssert(1 <= degree0 && degree0 + 1 < numControls0, "Invalid degree.");
            LogAssert(numControls0 <= numSamples0, "Invalid number of controls.");
            LogAssert(1 <= degree1 && degree1 + 1 < numControls1, "Invalid degree.");
            LogAssert(numControls1 <= numSamples1, "Invalid number of controls.");
            LogAssert(sampleData, "Invalid sample data.");

            mDegree[0] = degree0;
            mNumSamples[0] = numSamples0;
            mNumControls[0] = numControls0;
            mDegree[1] = degree1;
            mNumSamples[1] = numSamples1;
            mNumControls[1] = numControls1;

            BasisFunctionInput<Real> input;
            Real tMultiplier[2];
            int32_t dim;
            for (dim = 0; dim < 2; ++dim)
            {
                input.numControls = mNumControls[dim];
                input.degree = mDegree[dim];
                input.uniform = true;
                input.periodic = false;
                input.numUniqueKnots = mNumControls[dim] - mDegree[dim] + 1;
                input.uniqueKnots.resize(input.numUniqueKnots);
                input.uniqueKnots[0].t = (Real)0;
                input.uniqueKnots[0].multiplicity = mDegree[dim] + 1;
                int32_t last = input.numUniqueKnots - 1;
                Real factor = (Real)1 / (Real)last;
                for (int32_t i = 1; i < last; ++i)
                {
                    input.uniqueKnots[i].t = factor * (Real)i;
                    input.uniqueKnots[i].multiplicity = 1;
                }
                input.uniqueKnots[last].t = (Real)1;
                input.uniqueKnots[last].multiplicity = mDegree[dim] + 1;
                mBasis[dim].Create(input);

                tMultiplier[dim] = ((Real)1) / ((Real)mNumSamples[dim] - (Real)1);
            }

            // Fit the data points with a B-spline surface using a
            // least-squares error metric.  The problem is of the form
            // A0^T*A0*Q*A1^T*A1 = A0^T*P*A1, where A0^T*A0 and A1^T*A1 are
            // banded matrices, P contains the sample data, and Q is the
            // unknown matrix of control points.
            Real t{};
            int32_t i0{}, i1{}, i2{}, imin{}, imax{};

            // Construct the matrices A0^T*A0 and A1^T*A1.
            BandedMatrix<Real> ATAMat[2] =
            {
                BandedMatrix<Real>(mNumControls[0], mDegree[0] + 1, mDegree[0] + 1),
                BandedMatrix<Real>(mNumControls[1], mDegree[1] + 1, mDegree[1] + 1)
            };

            for (dim = 0; dim < 2; ++dim)
            {
                for (i0 = 0; i0 < mNumControls[dim]; ++i0)
                {
                    for (i1 = 0; i1 < i0; ++i1)
                    {
                        ATAMat[dim](i0, i1) = ATAMat[dim](i1, i0);
                    }

                    int32_t i1Max = i0 + mDegree[dim];
                    if (i1Max >= mNumControls[dim])
                    {
                        i1Max = mNumControls[dim] - 1;
                    }

                    for (i1 = i0; i1 <= i1Max; ++i1)
                    {
                        Real value = (Real)0;
                        for (i2 = 0; i2 < mNumSamples[dim]; ++i2)
                        {
                            t = tMultiplier[dim] * (Real)i2;
                            mBasis[dim].Evaluate(t, 0, imin, imax);
                            if (imin <= i0 && i0 <= imax && imin <= i1 && i1 <= imax)
                            {
                                Real b0 = mBasis[dim].GetValue(0, i0);
                                Real b1 = mBasis[dim].GetValue(0, i1);
                                value += b0 * b1;
                            }
                        }
                        ATAMat[dim](i0, i1) = value;
                    }
                }
            }

            // Construct the matrices A0^T and A1^T.  A[d]^T has
            // mNumControls[d] rows and mNumSamples[d] columns.
            Array2<Real> ATMat[2];
            for (dim = 0; dim < 2; dim++)
            {
                ATMat[dim] = Array2<Real>(mNumSamples[dim], mNumControls[dim]);
                size_t numBytes = static_cast<size_t>(mNumControls[dim]) * static_cast<size_t>(mNumSamples[dim]) * sizeof(Real);
                std::memset(ATMat[dim][0], 0, numBytes);
                for (i0 = 0; i0 < mNumControls[dim]; ++i0)
                {
                    for (i1 = 0; i1 < mNumSamples[dim]; ++i1)
                    {
                        t = tMultiplier[dim] * (Real)i1;
                        mBasis[dim].Evaluate(t, 0, imin, imax);
                        if (imin <= i0 && i0 <= imax)
                        {
                            ATMat[dim][i0][i1] = mBasis[dim].GetValue(0, i0);
                        }
                    }
                }
            }

            // Compute X0 = (A0^T*A0)^{-1}*A0^T and X1 = (A1^T*A1)^{-1}*A1^T
            // by solving the linear systems A0^T*A0*X0 = A0^T and
            // A1^T*A1*X1 = A1^T.
            for (dim = 0; dim < 2; ++dim)
            {
                bool solved = ATAMat[dim].template SolveSystem<true>(ATMat[dim][0], mNumSamples[dim]);
                LogAssert(solved, "Failed to solve linear system in BSplineSurfaceFit constructor.");
            }

            // The control points for the fitted surface are stored in the matrix
            // Q = X0*P*X1^T, where P is the matrix of sample data.
            for (i1 = 0; i1 < mNumControls[1]; ++i1)
            {
                for (i0 = 0; i0 < mNumControls[0]; ++i0)
                {
                    Vector3<Real> sum = Vector3<Real>::Zero();
                    for (int32_t j1 = 0; j1 < mNumSamples[1]; ++j1)
                    {
                        Real x1Value = ATMat[1][i1][j1];
                        for (int32_t j0 = 0; j0 < mNumSamples[0]; ++j0)
                        {
                            Real x0Value = ATMat[0][i0][j0];
                            Vector3<Real> sample =
                                mSampleData[j0 + mNumSamples[0] * j1];
                            sum += (x0Value * x1Value) * sample;
                        }
                    }
                    mControlData[i0 + static_cast<size_t>(mNumControls[0]) * i1] = sum;
                }
            }
        }

        // Access to input sample information.
        inline int32_t GetNumSamples(int32_t dimension) const
        {
            return mNumSamples[dimension];
        }

        inline Vector3<Real> const* GetSampleData() const
        {
            return mSampleData;
        }

        // Access to output control point and surface information.
        inline int32_t GetDegree(int32_t dimension) const
        {
            return mDegree[dimension];
        }

        inline int32_t GetNumControls(int32_t dimension) const
        {
            return mNumControls[dimension];
        }

        inline Vector3<Real> const* GetControlData() const
        {
            return &mControlData[0];
        }

        inline BasisFunction<Real> const& GetBasis(int32_t dimension) const
        {
            return mBasis[dimension];
        }

        // Evaluation of the B-spline surface.  It is defined for
        // 0 <= u <= 1 and 0 <= v <= 1.  If a parameter value is outside
        // [0,1], it is clamped to [0,1].
        Vector3<Real> GetPosition(Real u, Real v) const
        {
            int32_t iumin{}, iumax{}, ivmin{}, ivmax{};
            mBasis[0].Evaluate(u, 0, iumin, iumax);
            mBasis[1].Evaluate(v, 0, ivmin, ivmax);

            Vector3<Real> position = Vector3<Real>::Zero();
            for (int32_t iv = ivmin; iv <= ivmax; ++iv)
            {
                Real value1 = mBasis[1].GetValue(0, iv);
                for (int32_t iu = iumin; iu <= iumax; ++iu)
                {
                    Real value0 = mBasis[0].GetValue(0, iu);
                    Vector3<Real> control = mControlData[iu + static_cast<size_t>(mNumControls[0]) * iv];
                    position += (value0 * value1) * control;
                }
            }
            return position;
        }

    private:
        // Input sample information.
        int32_t mNumSamples[2];
        Vector3<Real> const* mSampleData;

        // The fitted B-spline surface, open and with uniform knots.
        int32_t mDegree[2];
        int32_t mNumControls[2];
        std::vector<Vector3<Real>> mControlData;
        BasisFunction<Real> mBasis[2];
    };
}
