// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The SingularValueDecomposition class is an implementation of Algorithm
// 8.3.2 (The SVD Algorithm) described in "Matrix Computations, 2nd
// edition" by G. H. Golub and Charles F. Van Loan, The Johns Hopkins
// Press, Baltimore MD, Fourth Printing 1993. Algorithm 5.4.2 (Householder
// Bidiagonalization) is used to reduce A to bidiagonal B. Algorithm 8.3.1
// (Golub-Kahan SVD Step) is used for the iterative reduction from bidiagonal
// to diagonal. If A is the original matrix, S is the matrix whose diagonal
// entries are the singular values, and U and V are corresponding matrices,
// then theoretically U^T*A*V = S. Numerically, we have errors E = U^T*A*V-S.
// Algorithm 8.3.2 mentions that one expects |E| is approximately
// unitRoundoff*|A|, where |A| denotes the Frobenius norm of A and where
// unitRoundoff is 2^{-23} for Real=float, which is
//   std::numeric_limits<float>::epsilon() = 1.192092896e-7f
// or 2^{-52} for Real=double, which is
//   std::numeric_limits<double>::epsilon() = 2.2204460492503131e-16.
//
// During the iterations that process B, the bidiagonalized A, a superdiagonal
// entry is determined to be effectively zero when compared to its neighboring
// diagonal and superdiagonal elements,
//   |b(i,i+1)| <= e * (|b(i,i) + b(i+1,i+1)|)
// The suggestion by Golub and van Loan is to choose e to be a small positive
// multiple of the unit roundoff, e = multiplier * unitRoundoff. A diagonal
// entry is determined to be effectively zero relative to a norm of B,
//   |b(i,i)| <= e * |B|
// The implementation uses the L-infinity norm for |B|, which is the largest
// absolute value of the diagonal and superdiagonal elements of B.
//
// The authors suggest that once you have the bidiagonal matrix, a practical
// implementation will store the diagonal and superdiagonal entries in linear
// arrays, ignoring the theoretically zero values not in the 2-band. This is
// good for cache coherence. The implementation uses separate storage for
// the Householder u-vectors, so the essential parts of these vectors is not
// stored in the local copy of A (as suggested by Golub and van Loan) in order
// to make the implementation more readable.

#include <Mathematics/Logger.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

namespace gte
{
    template <typename Real>
    class SingularValueDecomposition
    {
    public:
        // The solver processes MxN symmetric matrices, where M >= N > 1
        // ('numRows' is M and 'numCols' is N) and the matrix is stored in
        // row-major order. The maximum number of iterations ('maxIterations')
        // must be specified for the reduction of a bidiagonal matrix to a
        // diagonal matrix. The goal is to compute MxM orthogonal U, NxN
        // orthogonal V and MxN matrix S for which U^T*A*V = S. The only
        // nonzero entries of S are on the diagonal; the diagonal entries
        // are the singular values of the original matrix.
        SingularValueDecomposition(size_t numRows, size_t numCols, size_t maxIterations)
            :
            mNumRows(numRows),
            mNumCols(numCols),
            mMaxIterations(maxIterations),
            mMatrix{},
            mUMatrix{},
            mVMatrix{},
            mSMatrix{},
            mDiagonal{},
            mSuperdiagonal{},
            mLHouseholder{},
            mRHouseholder{},
            mLGivens{},
            mRGivens{}
        {
            LogAssert(
                mNumCols >= 2 && mNumRows >= mNumCols && mMaxIterations > 0,
                "Invalid input.");

            mMatrix.resize(mNumRows * mNumCols);
            mUMatrix.resize(mNumRows * mNumRows);
            mVMatrix.resize(mNumCols * mNumCols);
            mSMatrix.resize(mNumRows * mNumCols);

            mDiagonal.resize(mNumCols);
            mSuperdiagonal.resize(mNumCols - 1);

            mLHouseholder.resize(mNumCols);
            for (size_t col = 0; col < mLHouseholder.size(); ++col)
            {
                mLHouseholder[col].resize(mNumRows);
            }

            mRHouseholder.resize(mNumCols - 2);
            for (size_t row = 0; row < mRHouseholder.size(); ++row)
            {
                mRHouseholder[row].resize(mNumCols);
            }

            mLGivens.reserve(mMaxIterations* (mNumCols - 1));
            mRGivens.reserve(mMaxIterations* (mNumCols - 1));
        }

        // A copy of the MxN input is made internally. The multiplier is a
        // small positive number used to compute e that is described in the
        // preamble comments of this file. The default is 8, but you can
        // adjust this based on the needs of your application. The return
        // value is the number of iterations consumed when convergence
        // occurs or std::numeric_limits<size_t>::max() when convergence
        // does not occur.
        size_t Solve(Real const* input, Real multiplier = static_cast<Real>(8))
        {
            Real const zero = static_cast<Real>(0);
            LogAssert(
                input != nullptr && multiplier > zero,
                "Invalid input to Solve.");

            // Copy the input to mMatrix. The latter matrix is modified
            // internally by the solver.
            for (size_t i = 0; i < mMatrix.size(); ++i)
            {
                mMatrix[i] = input[i];
            }

            // Reduce mMatrix to bidiagonal form, storing the diagonal
            // mMatrix(d,d) and superdiagonal mMatrix(d,d+1) in mDiagonal
            // and mSuperdiagonal, respectively.
            Bidiagonalize();

            // The threshold is used to determine whether a diagonal entry of
            // the bidiagonal matrix B is sufficiently small so that it is
            // considered tobe zero. It is defined by
            //   threshold = multiplier * unitRoundoff * |B|, where
            // unitRoundoff is std::numeric_limits<Real>::epsilon(), |B| is a
            // matrix norm and the multiplier is a small number [as suggested
            // before Algorithm 8.3.2 (The SVD Algorithm) in Golub and Van
            // Loan]. The L-infinity norm is used for B.
            Real epsilon{}, threshold{};
            ComputeCutoffs(multiplier, epsilon, threshold);

            size_t constexpr invalid = std::numeric_limits<size_t>::max();
            mRGivens.clear();
            mLGivens.clear();
            for (size_t iteration = 0; iteration < mMaxIterations; ++iteration)
            {
                // Set superdiagonal entries to zero if they are effectively
                // zero compared to the neighboring diagonal entries.
                size_t numZero = 0;
                for (size_t i = 0; i <= mNumCols - 2; ++i)
                {
                    Real absSuper = std::fabs(mSuperdiagonal[i]);
                    Real absDiag0 = std::fabs(mDiagonal[i]);
                    Real absDiag1 = std::fabs(mDiagonal[i + 1]);
                    if (absSuper <= epsilon * (absDiag0 + absDiag1))
                    {
                        mSuperdiagonal[i] = zero;
                        ++numZero;
                    }
                }

                if (numZero == mNumCols - 1)
                {
                    // The superdiagonal terms are all effectively zero, so
                    // the algorithm has converged. Compute U, V and S.
                    ComputeOrthogonalMatrices();
                    return iteration;
                }

                // Find the largest sequence {iMin,...,iMax} for which the
                // superdiagonal entries are all not effectively zero. On
                // loop termination, iMax != invalid because if all
                // superdiagonal terms were zero, the previous if-statement
                // "if (numZero == mNumCols - 1)" would ensure an exit from
                // the function.
                size_t iMax;
                for (iMax = mNumCols - 2; iMax != 0; --iMax)
                {
                    if (mSuperdiagonal[iMax] != zero)
                    {
                        break;
                    }
                }
                ++iMax;

                size_t iMin;
                for (iMin = iMax - 1; iMin != invalid; --iMin)
                {
                    if (mSuperdiagonal[iMin] == zero)
                    {
                        break;
                    }
                }
                ++iMin;

                // The subblock corresponding to {iMin,...,iMax} has all
                // superdiagonal entries not effectively zero. Determine
                // whether this subblock has a diagonal entry that is
                // effectively zero. If it does, use Givens rotations to
                // zero-out the row containing that entry.
                if (DiagonalEntriesNonzero(iMin, iMax, threshold))
                {
                    DoGolubKahanStep(iMin, iMax);
                }
            }
            return invalid;
        }

        // Get the U-matrix, which is MxM and stored in row-major order.
        void GetU(Real* uMatrix) const
        {
            LogAssert(
                uMatrix != nullptr,
                "Nonnull pointer required for uMatrix.");

            for (size_t i = 0; i < mUMatrix.size(); ++i)
            {
                uMatrix[i] = mUMatrix[i];
            }
        }

        // Get the V-matrix, which is NxN and stored in row-major order.
        void GetV(Real* vMatrix) const
        {
            LogAssert(
                vMatrix != nullptr,
                "Nonnull pointer required for vMatrix.");

            for (size_t i = 0; i < mUMatrix.size(); ++i)
            {
                vMatrix[i] = mVMatrix[i];
            }
        }

        // Get the S-matrix, which is MxN and stored in row-major order.
        void GetS(Real* sMatrix) const
        {
            LogAssert(
                sMatrix != nullptr,
                "Nonnull pointer required for sMatrix.");

            for (size_t i = 0; i < mSMatrix.size(); ++i)
            {
                sMatrix[i] = mSMatrix[i];
            }
        }

        Real GetSingularValue(size_t index) const
        {
            LogAssert(
                index < mNumCols,
                "Invalid index for singular value.");

            return mSMatrix[index + mNumCols * index];
        }

        void GetUColumn(size_t index, Real* uColumn) const
        {
            LogAssert(
                index < mNumRows && uColumn != nullptr,
                "Invalid index or null pointer for U-column.");

            for (size_t row = 0; row < mNumRows; ++row)
            {
                uColumn[row] = mUMatrix[index + mNumRows * row];
            }
        }

        void GetVColumn(size_t index, Real* vColumn) const
        {
            LogAssert(
                index < mNumCols && vColumn != nullptr,
                "Invalid index or null pointer for V-column.");

            for (size_t row = 0; row < mNumCols; ++row)
            {
                vColumn[row] = mVMatrix[index + mNumCols * row];
            }
        }

        // Get the singular values, which is an N-element array.
        void GetSingularValues(Real* singularValues) const
        {
            LogAssert(
                singularValues != nullptr,
                "Nonnull pointer required for singularValues.");

            for (size_t index = 0; index < mNumCols; ++index)
            {
                singularValues[index] = mSMatrix[index + mNumCols * index];
            }
        }

    private:
        static Real constexpr unitRoundoff = std::numeric_limits<Real>::epsilon();

        // Algorithm 5.1.1 (Householder Vector). The matrix A has size
        // numRows-by-numCols with numRows >= numCols and the vector v has
        // size numRows.
        static void ComputeHouseholderU(size_t numRows, size_t numCols,
            Real const* A, size_t selectCol, Real* v)
        {
            // Extract the column vector v[] where v[row] = A(row, selectCol).
            // The elements v[row] = 0 for 0 <= row < selectCol to avoid
            // conceptual uninitialized memory; the caller should not
            // reference these elements.
            Real const zero = static_cast<Real>(0);
            size_t row;
            for (row = 0; row < selectCol; ++row)
            {
                v[row] = zero;
            }

            Real mu = zero;
            for (; row < numRows; ++row)
            {
                Real element = A[selectCol + numCols * row];
                mu += element * element;
                v[row] = element;
            }
            mu = std::sqrt(mu);

            if (mu != zero)
            {
                Real beta = v[selectCol] + (v[selectCol] >= zero ? mu : -mu);
                for (row = selectCol + 1; row < numRows; ++row)
                {
                    v[row] /= beta;
                }
            }
            v[selectCol] = static_cast<Real>(1);
        }

        // Algorithm 5.1.1 (Householder Vector). The matrix A has size
        // numRows-by-numCols with numRows >= numCols and the vector v has
        // size numCols.
        static void ComputeHouseholderV(size_t numRows, size_t numCols,
            Real const* A, size_t selectRow, Real* v)
        {
            // Extract the row vector v[] where v[col] = A(selectRow, col).
            // The elements v[col] = 0 for 0 <= col <= selectRow to avoid
            // conceptual uninitialized memory; the caller should not
            // reference these elements.
            (void)numRows;
            Real const zero = static_cast<Real>(0);
            size_t selectRowP1 = selectRow + 1;
            size_t col;
            for (col = 0; col < selectRowP1; ++col)
            {
                v[col] = zero;
            }

            Real mu = zero;
            for (; col < numCols; ++col)
            {
                Real element = A[col + numCols * selectRow];
                mu += element * element;
                v[col] = element;
            }
            mu = std::sqrt(mu);

            if (mu != zero)
            {
                Real beta = v[selectRowP1] + (v[selectRowP1] >= zero ? mu : -mu);
                for (col = selectRowP1 + 1; col < numCols; ++col)
                {
                    v[col] /= beta;
                }
            }
            v[selectRowP1] = static_cast<Real>(1);
        }

        // Algorithm 5.1.2 (Householder Pre-Multiplication)
        static void DoHouseholderPremultiply(size_t numRows, size_t numCols,
            Real const* v, size_t selectCol, Real* A)
        {
            Real const zero = static_cast<Real>(0);
            Real vSqrLength = zero;
            for (size_t row = selectCol; row < numRows; ++row)
            {
                vSqrLength += v[row] * v[row];
            }
            Real beta = static_cast<Real>(-2) / vSqrLength;

            std::vector<Real> w(numCols);
            for (size_t col = selectCol; col < numCols; ++col)
            {
                w[col] = zero;
                for (size_t row = selectCol; row < numRows; ++row)
                {
                    w[col] += v[row] * A[col + numCols * row];
                }
                w[col] *= beta;
            }

            for (size_t row = selectCol; row < numRows; ++row)
            {
                for (size_t col = selectCol; col < numCols; ++col)
                {
                    A[col + numCols * row] += v[row] * w[col];
                }
            }
        }

        // Algorithm 5.1.3 (Householder Post-Multiplication)
        static void DoHouseholderPostmultiply(size_t numRows, size_t numCols,
            Real const* v, size_t selectRow, Real* A)
        {
            Real const zero = static_cast<Real>(0);
            Real vSqrLength = zero;
            for (size_t col = selectRow; col < numCols; ++col)
            {
                vSqrLength += v[col] * v[col];
            }
            Real beta = static_cast<Real>(-2) / vSqrLength;

            std::vector<Real> w(numRows);
            for (size_t row = selectRow; row < numRows; ++row)
            {
                w[row] = zero;
                for (size_t col = selectRow; col < numCols; ++col)
                {
                    w[row] += A[col + numCols * row] * v[col];
                }
                w[row] *= beta;
            }

            for (size_t row = selectRow; row < numRows; ++row)
            {
                for (size_t col = selectRow; col < numCols; ++col)
                {
                    A[col + numCols * row] += w[row] * v[col];
                }
            }
        }

        // Bidiagonalize using Householder reflections. On input, mMatrix is
        // a copy of the input matrix passed to Solve(...). On output,
        // mDiagonal and mSuperdiagonal contain the bidiagonalized results.
        void Bidiagonalize()
        {
            std::vector<Real> uVector(mNumRows), vVector(mNumCols);
            for (size_t i = 0; i < mNumCols; ++i)
            {
                // Compute the u-Householder vector.
                ComputeHouseholderU(mNumRows, mNumCols, mMatrix.data(),
                    i, mLHouseholder[i].data());

                // Update A = (I - 2*u*u^T/u^T*u) * A.
                DoHouseholderPremultiply(mNumRows, mNumCols, mLHouseholder[i].data(),
                    i, mMatrix.data());

                if (i < mRHouseholder.size())
                {
                    // Compute the v-Householder vectors.
                    ComputeHouseholderV(mNumRows, mNumCols, mMatrix.data(),
                        i, mRHouseholder[i].data());

                    // Update A = A * (I - 2*v*v^T/v^T*v).
                    DoHouseholderPostmultiply(mNumRows, mNumCols, mRHouseholder[i].data(),
                        i, mMatrix.data());
                }
            }

            // Copy the diagonal and subdiagonal for cache coherence in the
            // Golub-Kahan iterations.
            for (size_t d = 0; d < mNumCols; ++d)
            {
                mDiagonal[d] = mMatrix[d + mNumCols * d];
            }
            for (size_t s = 0; s < mNumCols - 1; ++s)
            {
                mSuperdiagonal[s] = mMatrix[(s + 1) + mNumCols * s];
            }
        }

        void ComputeCutoffs(Real multiplier, Real& epsilon, Real& threshold) const
        {
            Real norm = static_cast<Real>(0);
            for (size_t i = 0; i < mNumCols; ++i)
            {
                Real abs = std::fabs(mDiagonal[i]);
                if (abs > norm)
                {
                    norm = abs;
                }
            }

            for (size_t i = 0; i < mNumCols - 1; ++i)
            {
                Real abs = std::fabs(mSuperdiagonal[i]);
                if (abs > norm)
                {
                    norm = abs;
                }
            }

            epsilon = multiplier * unitRoundoff;
            threshold = epsilon * norm;
        }

        // A helper for generating Givens rotation sine and cosine robustly
        // when solving sn * x + cs * y = 0.
        void GetSinCos(Real x, Real y, Real& cs, Real& sn)
        {
            Real const zero = static_cast<Real>(0);
            Real const one = static_cast<Real>(1);
            Real tau;
            if (y != zero)
            {
                if (std::fabs(y) > std::fabs(x))
                {
                    tau = -x / y;
                    sn = one / std::sqrt(one + tau * tau);
                    cs = sn * tau;
                }
                else
                {
                    tau = -y / x;
                    cs = one / std::sqrt(one + tau * tau);
                    sn = cs * tau;
                }
            }
            else
            {
                cs = one;
                sn = zero;
            }
        }

        // Test for diagonal entries that are effectively zero through all
        // but the last. For each such entry, the B matrix decouples. Perform
        // that decoupling. If there are no zero-valued entries, then the
        // Golub-Kahan step must be performed.
        bool DiagonalEntriesNonzero(size_t iMin, size_t iMax, Real threshold)
        {
            Real const zero = static_cast<Real>(0);
            for (size_t i = iMin; i < iMax; ++i)
            {
                if (std::fabs(mDiagonal[i]) <= threshold)
                {
                    // Use planar rotations to chase the superdiagonal entry
                    // out of the matrix, which produces a row of zeros.
                    Real x, z, cs, sn;
                    Real y = mSuperdiagonal[i];
                    mSuperdiagonal[i] = zero;
                    for (size_t j = i + 1; j <= iMax; ++j)
                    {
                        x = mDiagonal[j];
                        GetSinCos(x, y, cs, sn);
                        // NOTE: The Givens parameters are (cs,-sn). The
                        // negative sign is not a coding error.
                        mLGivens.push_back(GivensRotation(i, j, cs, -sn));
                        mDiagonal[j] = cs * x - sn * y;
                        if (j < iMax)
                        {
                            z = mSuperdiagonal[j];
                            mSuperdiagonal[j] = cs * z;
                            y = sn * z;
                        }
                    }
                    return false;
                }
            }
            return true;
        }

        // Algorithm 8.3.1 (Golub-Kahan SVD Step).
        void DoGolubKahanStep(size_t iMin, size_t iMax)
        {
            // The implicit shift. Let A = {{a00,a01},{a01,a11}} be the lower
            // right 2x2 block of B^T*B. Compute the eigenvalue u of A that
            //  is closer to a11 than to a00.
            Real const zero = static_cast<Real>(0);
            Real f0, f1, d1, d2;
            if (iMax > 1)
            {
                f0 = mSuperdiagonal[iMax - 2];
                f1 = mSuperdiagonal[iMax - 1];
                d1 = mDiagonal[iMax - 1];
                d2 = mDiagonal[iMax];
            }
            else
            {
                f0 = zero;
                f1 = mSuperdiagonal[0];
                d1 = mDiagonal[0];
                d2 = mDiagonal[1];
            }

            // Compute the lower right 2x2 block of B^T*B.
            Real a00 = d1 * d1 + f0 * f0;
            Real a01 = d1 * f1;
            Real a11 = d2 * d2 + f1 * f1;

            // The eigenvalues are ((a00+a11) +/- sqrt((a00-a11)^2+a01^2))/2,
            // which are equidistant from (a00+a11)/2. If a11 >= a00, the
            // required eigenvalue uses the (+) sqrt term. If a11 <= a00, the
            // required eigenvalue uses the (-) sqrt term.
            Real sum = a00 + a11;
            Real dif = a00 - a11;
            Real root = std::sqrt(dif * dif + a01 * a01);
            Real lambda = static_cast<Real>(0.5) * (a11 >= a00 ? sum + root : sum - root);
            Real x = mDiagonal[iMin] * mDiagonal[iMin] - lambda;
            Real y = mDiagonal[iMin] * mSuperdiagonal[iMin];

            Real a12, a21, a22, a23, cs, sn;
            Real a02 = zero;
            for (size_t i0 = iMin - 1, i1 = iMin, i2 = iMin + 1; i1 <= iMax - 1; ++i0, ++i1, ++i2)
            {
                // Compute the Givens rotation G and save it for use in
                // computing V in U^T*A*V = S.
                GetSinCos(x, y, cs, sn);
                mRGivens.push_back(GivensRotation(i1, i2, cs, sn));

                // Update B0 = B*G.
                if (i1 > iMin)
                {
                    mSuperdiagonal[i0] = cs * mSuperdiagonal[i0] - sn * a02;
                }

                a11 = mDiagonal[i1];
                a12 = mSuperdiagonal[i1];
                a22 = mDiagonal[i2];
                mDiagonal[i1] = cs * a11 - sn * a12;
                mSuperdiagonal[i1] = sn * a11 + cs * a12;
                mDiagonal[i2] = cs * a22;
                a21 = -sn * a22;

                // Update the parameters for the next Givens rotations.
                x = mDiagonal[i1];
                y = a21;

                // Compute the Givens rotation G and save it for use in
                // computing U in U^T*A*V = S.
                GetSinCos(x, y, cs, sn);
                mLGivens.push_back(GivensRotation(i1, i2, cs, sn));

                // Update B1 = G^T*B0.
                a11 = mDiagonal[i1];
                a12 = mSuperdiagonal[i1];
                a22 = mDiagonal[i2];
                mDiagonal[i1] = cs * a11 - sn * a21;
                mSuperdiagonal[i1] = cs * a12 - sn * a22;
                mDiagonal[i2] = sn * a12 + cs * a22;

                if (i1 < iMax - 1)
                {
                    a23 = mSuperdiagonal[i2];
                    a02 = -sn * a23;
                    mSuperdiagonal[i2] = cs * a23;

                    // Update the parameters for the next Givens rotations.
                    x = mSuperdiagonal[i1];
                    y = a02;
                }
            }
        }

        void ComputeOrthogonalMatrices()
        {
            // Compute U and V given the current signed singular values.
            ComputeUOrthogonal();
            ComputeVOrthogonal();

            // Ensure the singular values are nonnegative. The sign changes
            // are absorbed by the U-matrix. The nonnegative values are
            // stored in the S-matrix.
            EnsureNonnegativeSingularValues();

            // Sort the singular values in descending order. The sort
            // permutations are absorbed by the U-matrix and V-matrix.
            SortSingularValues();
        }

        void ComputeUOrthogonal()
        {
            // Start with the identity matrix for U.
            Real const zero = static_cast<Real>(0);
            Real const one = static_cast<Real>(1);
            std::fill(mUMatrix.begin(), mUMatrix.end(), zero);
            for (size_t d = 0; d < mNumRows; ++d)
            {
                mUMatrix[d + mNumRows * d] = one;
            }

            // Multiply the Householder reflections using backward
            // accumulation. This allows DoHouseholderPremultiply. A forward
            // accumulation using DoHouseholderPostmultiply does not work
            // because the semantics of DoHouseholderPostmultiply are
            // slightly different from those of DoHouseholderPremultiply.
            for (size_t k = 0, col = mNumCols - 1; k <= mNumCols - 1; ++k, --col)
            {
                DoHouseholderPremultiply(mNumRows, mNumRows, mLHouseholder[col].data(),
                    col, mUMatrix.data());
            }

            // Multiply the Givens rotations using forward accumulation.
            for (auto const& givens : mLGivens)
            {
                size_t j0 = givens.index0;
                size_t j1 = givens.index1;
                for (size_t row = 0; row < mNumRows; ++row, j0 += mNumRows, j1 += mNumRows)
                {
                    Real& q0 = mUMatrix[j0];
                    Real& q1 = mUMatrix[j1];
                    Real prd0 = givens.cs * q0 - givens.sn * q1;
                    Real prd1 = givens.sn * q0 + givens.cs * q1;
                    q0 = prd0;
                    q1 = prd1;
                }
            }
        }

        void ComputeVOrthogonal()
        {
            // Start with the identity matrix for V.
            Real const zero = static_cast<Real>(0);
            Real const one = static_cast<Real>(1);
            std::fill(mVMatrix.begin(), mVMatrix.end(), zero);
            for (size_t d = 0; d < mNumCols; ++d)
            {
                mVMatrix[d + mNumCols * d] = one;
            }

            // Multiply the Householder reflections using backward
            // accumulation. This allows DoHouseholderPremultiply. A forward
            // accumulation using DoHouseholderPostmultiply does not work
            // because the semantics of DoHouseholderPostmultiply are
            // slightly different from those of DoHouseholderPremultiply.
            for (size_t k = 0, col = mNumCols - 3; k <= mNumCols - 3; ++k, --col)
            {
                DoHouseholderPremultiply(mNumCols, mNumCols, mRHouseholder[col].data(),
                    col, mVMatrix.data());
            }

            // Multiply the Givens rotations using forward accumulation.
            for (auto const& givens : mRGivens)
            {
                size_t j0 = givens.index0;
                size_t j1 = givens.index1;
                for (size_t col = 0; col < mNumCols; ++col, j0 += mNumCols, j1 += mNumCols)
                {
                    Real& q0 = mVMatrix[j0];
                    Real& q1 = mVMatrix[j1];
                    Real prd0 = givens.cs * q0 - givens.sn * q1;
                    Real prd1 = givens.sn * q0 + givens.cs * q1;
                    q0 = prd0;
                    q1 = prd1;
                }
            }
        }

        void EnsureNonnegativeSingularValues()
        {
            Real const zero = static_cast<Real>(0);
            std::fill(mSMatrix.begin(), mSMatrix.end(), zero);
            for (size_t i = 0; i < mNumCols; ++i)
            {
                if (mDiagonal[i] >= zero)
                {
                    mSMatrix[i + mNumCols * i] = mDiagonal[i];
                }
                else
                {
                    mSMatrix[i + mNumCols * i] = -mDiagonal[i];
                    for (size_t row = 0; row < mNumRows; ++row)
                    {
                        Real& element = mUMatrix[i + mNumRows * row];
                        element = -element;
                    }
                }
            }
        }

        void SortSingularValues()
        {
            // Sort the nonnegative singular values.
            std::vector<SingularInfo> sorted(mNumCols);
            for (size_t i = 0; i < mNumCols; ++i)
            {
                sorted[i].singular = mSMatrix[i + mNumCols * i];
                sorted[i].inversePermute = i;
            }
            std::sort(sorted.begin(), sorted.end(), std::greater<SingularInfo>());
            for (size_t i = 0; i < mNumCols; ++i)
            {
                mSMatrix[i + mNumCols * i] = sorted[i].singular;
            }

            // Compute the inverse permutation of the sorting.
            std::vector<size_t> permute(mNumCols);
            for (size_t i = 0; i < mNumCols; ++i)
            {
                permute[sorted[i].inversePermute] = i;
            }

            // Permute the columns of the U-matrix to be consistent with the
            // sorted singular values.
            std::vector<Real> sortedUMatrix(mNumRows * mNumRows);
            size_t col;
            for (col = 0; col < mNumCols; ++col)
            {
                for (size_t row = 0; row < mNumRows; ++row)
                {
                    Real const& source = mUMatrix[col + mNumRows * row];
                    Real& target = sortedUMatrix[permute[col] + mNumRows * row];
                    target = source;
                }
            }
            for (; col < mNumRows; ++col)
            {
                for (size_t row = 0; row < mNumRows; ++row)
                {
                    Real const& source = mUMatrix[col + mNumRows * row];
                    Real& target = sortedUMatrix[col + mNumRows * row];
                    target = source;
                }
            }
            mUMatrix = std::move(sortedUMatrix);

            // Permute the columns of the U-matrix to be consistent with the
            // sorted singular values.
            std::vector<Real> sortedVMatrix(mNumCols * mNumCols);
            for (col = 0; col < mNumCols; ++col)
            {
                for (size_t row = 0; row < mNumCols; ++row)
                {
                    Real const& source = mVMatrix[col + mNumCols * row];
                    Real& target = sortedVMatrix[permute[col] + mNumCols * row];
                    target = source;
                }
            }
            mVMatrix = std::move(sortedVMatrix);
        }

        // The number rows and columns of the matrices to be processed.
        size_t mNumRows, mNumCols;

        // The maximum number of iterations for reducing the bidiagonal matrix
        // to a diagonal matrix.
        size_t mMaxIterations;

        // The internal copy of a matrix passed to the solver. This is
        // stored in row-major order.
        std::vector<Real> mMatrix; // MxN elements

        // The U-matrix, V-matrix and S-matrix for which U*A*V^T = S. These
        // are stored in row-major order.
        std::vector<Real> mUMatrix;  // MxM
        std::vector<Real> mVMatrix;  // NxN
        std::vector<Real> mSMatrix;  // MxN

        // The diagonal and superdiagonal of the bidiagonalized matrix.
        std::vector<Real> mDiagonal;  // N elements
        std::vector<Real> mSuperdiagonal;  // N-1 elements

        // The Householder reflections used to reduce the input matrix to
        // a bidiagonal matrix.
        std::vector<std::vector<Real>> mLHouseholder;
        std::vector<std::vector<Real>> mRHouseholder;

        // The Givens rotations used to reduce the initial bidiagonal matrix
        // to a diagonal matrix. A rotation is the identity with the following
        // replacement entries: R(index0,index0) = cs, R(index0,index1) = sn,
        // R(index1,index0) = -sn and R(index1,index1) = cs. If N is the
        // number of matrix columns and K is the maximum number of iterations,
        // the maximum number of right or left Givens rotations is K*(N-1).
        // The maximum amount of memory is allocated to store these. However,
        // we also potentially need left rotations to decouple the matrix when
        // diagonal terms are zero. Worst case is a number of matrices
        // quadratic in N, so for now we just use std::vector<GivensRotation>
        // whose initial capacity is K*(N-1).
        struct GivensRotation
        {
            GivensRotation()
                :
                index0(0),
                index1(0),
                cs(static_cast<Real>(0)),
                sn(static_cast<Real>(0))
            {
            }

            GivensRotation(size_t inIndex0, size_t inIndex1, Real inCs, Real inSn)
                :
                index0(inIndex0),
                index1(inIndex1),
                cs(inCs),
                sn(inSn)
            {
            }

            size_t index0, index1;
            Real cs, sn;
        };

        std::vector<GivensRotation> mLGivens;
        std::vector<GivensRotation> mRGivens;

        // Support for sorting singular values.
        struct SingularInfo
        {
            SingularInfo()
                :
                singular(static_cast<Real>(0)),
                inversePermute(0)
            {
            }

            bool operator>(SingularInfo const& p) const
            {
                return singular > p.singular;
            }

            Real singular;
            size_t inversePermute;
        };
    };
}
