// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Use Gaussian elimination to solve a linear system, invert a matrix or
// compute the determinant of a matrix.

#include <GTL/Mathematics/Algebra/Matrix.h>
#include <GTL/Mathematics/Algebra/MatrixAccessor.h>
#include <cmath>
#include <functional>
#include <utility>
#include <vector>

namespace gtl
{
    template <typename T>
    class GaussianElimination
    {
    public:
        // Solve M*X = B, where M is NxN and where X and B are NxK. The
        // size inputs are numRows for N and numCols for K. The return
        // value is true if and only if there is a unique solution.
        static bool SolveSystem(size_t numRows, size_t numCols, T const* M,
            T const* B, T* X, bool isRowMajor = true)
        {
            return Execute(numRows, numCols, M, nullptr, nullptr, B, X, isRowMajor);
        }

        // Compute the inverse M^{-1} of M, where M is NxN. The size input
        // is numRows for N. If the inverse exists, the return value is true
        // and inverseM stores the inverse. If the return value is false,
        // inverseM is invalid.
        static bool GetInverse(size_t numRows, T const* M, T* inverseM,
            bool isRowMajor = true)
        {
            return Execute(numRows, 0, M, inverseM, nullptr, nullptr, nullptr, isRowMajor);
        }

        // Compute the determinant of M, where M is NxN. The size input is
        // numRows for N. If the matrix is invertible, the return value is
        // true. If the return value is false, the matrix is not
        // invertible and the determinant is zero.
        static bool GetDeterminant(size_t numRows, T const* M, T& determinant,
            bool isRowMajor = true)
        {
            return Execute(numRows, 0, M, nullptr, &determinant, nullptr, nullptr, isRowMajor);
        }

        // Compute the inverse and determinant of M, where M is NxN. The size
        // input is numRows for N. Read the comments for GetInverse(...) and
        // GetDeterminant(...) about the computed values and the return value.
        static bool GetInverseAndDeterminant(size_t numRows, T const* M,
            T* inverseM, T& determinant, bool isRowMajor = true)
        {
            return Execute(numRows, 0, M, inverseM, &determinant, nullptr, nullptr, isRowMajor);
        }

    private:
        static bool Execute(size_t numRows, size_t numCols, T const* M,
            T* inverseM, T* determinant, T const* B, T* X, bool isRowMajor)
        {
            GTL_ARGUMENT_ASSERT(
                (B != nullptr && X != nullptr && numCols > 0) ||
                (B == nullptr && X == nullptr && numCols == 0),
                "Invalid input.");

            size_t const sizeM = numRows * numRows;
            size_t const sizeX = numRows * numCols;
            bool wantInverse = (inverseM != nullptr);
            bool wantDeterminant = (determinant != nullptr);
            bool wantSolution = (B != nullptr && X != nullptr);

            // Gaussian elimination is applied to a copy of M for in-place
            // computing of M^{-1}.
            std::vector<T> storageInvM;
            if (!wantInverse)
            {
                // The storage is provided locally but its contents are not
                // required by the caller.
                storageInvM.resize(sizeM);
                inverseM = storageInvM.data();
            }
            // else: The storage is what inverseM points to.

            // Copy M for the elimination process.
            for (size_t i = 0; i < sizeM; ++i)
            {
                inverseM[i] = M[i];
            }

            if (wantSolution)
            {
                // Copy B for the elimination process.
                for (size_t i = 0; i < sizeX; ++i)
                {
                    X[i] = B[i];
                }
            }

            // Create indexing functions based on the matrix storage order
            // specified by the caller.
            std::function<T& (size_t, size_t)> locInverseM{};
            std::function<T& (size_t, size_t)> locX{};
            if (isRowMajor)
            {
                locInverseM = [numRows, inverseM](size_t row, size_t col) -> T&
                {
                    return inverseM[col + numRows * row];
                };

                if (wantSolution)
                {
                    locX = [numCols, X](size_t row, size_t col) -> T&
                    {
                        return X[col + numCols * row];
                    };
                }
            }
            else
            {
                locInverseM = [numRows, inverseM](size_t row, size_t col) -> T&
                {
                    return inverseM[row + numRows * col];
                };

                if (wantSolution)
                {
                    locX = [numRows, X](size_t row, size_t col) -> T&
                    {
                        return X[row + numRows * col];
                    };
                }
            }

            T locDeterminant = C_<T>(1);
            bool odd = false;

            // Eliminate using full pivoting.
            size_t row = 0, col = 0;
            std::vector<size_t> rowIndex(numRows), colIndex(numRows);
            std::vector<size_t> pivoted(numRows, 0);
            for (size_t i0 = 0; i0 < numRows; ++i0)
            {
                // Search the matrix, excluding pivoted rows, for the maximum
                // absolute entry.
                T maxValue = C_<T>(0);
                for (size_t i1 = 0; i1 < numRows; ++i1)
                {
                    if (!pivoted[i1])
                    {
                        for (size_t i2 = 0; i2 < numRows; ++i2)
                        {
                            if (!pivoted[i2])
                            {
                                T absValue = std::fabs(locInverseM(i1, i2));
                                if (absValue > maxValue)
                                {
                                    maxValue = absValue;
                                    row = i1;
                                    col = i2;
                                }
                            }
                        }
                    }
                }

                if (maxValue == C_<T>(0))
                {
                    // The matrix is not invertible.
                    if (wantInverse)
                    {
                        // Set all elements to zero to signal singularity.
                        for (size_t i = 0; i < sizeM; ++i)
                        {
                            inverseM[i] = C_<T>(0);
                        }
                    }

                    if (wantDeterminant)
                    {
                        *determinant = C_<T>(0);
                    }

                    if (wantSolution)
                    {
                        for (size_t i = 0; i < sizeX; ++i)
                        {
                            X[i] = C_<T>(0);
                        }
                    }
                    return false;
                }

                pivoted[col] = true;

                // Swap rows so that the pivot entry is in row 'col'.
                if (row != col)
                {
                    odd = !odd;
                    for (size_t i = 0; i < numRows; ++i)
                    {
                        std::swap(locInverseM(row, i), locInverseM(col, i));
                    }

                    if (wantSolution)
                    {
                        for (size_t i = 0; i < numCols; ++i)
                        {
                            std::swap(locX(row, i), locX(col, i));
                        }
                    }
                }

                // Keep track of the permutations of the rows.
                rowIndex[i0] = row;
                colIndex[i0] = col;

                // Scale the row so that the pivot entry is 1.
                T diagonal = locInverseM(col, col);
                locDeterminant *= diagonal;
                T invDiagonal = C_<T>(1) / diagonal;
                locInverseM(col, col) = C_<T>(1);
                for (size_t i2 = 0; i2 < numRows; ++i2)
                {
                    locInverseM(col, i2) *= invDiagonal;
                }

                if (wantSolution)
                {
                    for (size_t i2 = 0; i2 < numCols; ++i2)
                    {
                        locX(col, i2) *= invDiagonal;
                    }
                }

                // Zero out the pivot column locations in the other rows.
                for (size_t i1 = 0; i1 < numRows; ++i1)
                {
                    if (i1 != col)
                    {
                        T const save = locInverseM(i1, col);
                        locInverseM(i1, col) = C_<T>(0);
                        for (size_t i2 = 0; i2 < numRows; ++i2)
                        {
                            locInverseM(i1, i2) -= locInverseM(col, i2) * save;
                        }

                        if (wantSolution)
                        {
                            for (size_t i2 = 0; i2 < numCols; ++i2)
                            {
                                locX(i1, i2) -= locX(col, i2) * save;
                            }
                        }
                    }
                }
            }

            if (wantInverse)
            {
                // Reorder rows to undo any permutations in Gaussian elimination.
                size_t i1 = numRows - 1;
                for (size_t j = 0; j < numRows; ++j, --i1)
                {
                    if (rowIndex[i1] != colIndex[i1])
                    {
                        for (size_t i2 = 0; i2 < numRows; ++i2)
                        {
                            std::swap(locInverseM(i2, rowIndex[i1]), locInverseM(i2, colIndex[i1]));
                        }
                    }
                }

                for (size_t i = 0; i < storageInvM.size(); ++i)
                {
                    inverseM[i] = storageInvM[i];
                }
            }

            if (wantDeterminant)
            {
                if (odd)
                {
                    locDeterminant = -locDeterminant;
                }

                *determinant = locDeterminant;
            }

            return true;
        }
    };

    // Specialized operations for GTL matrices whose sizes are known at
    // compile time.
    template <typename T, size_t N>
    bool SolveSystem(Matrix<T, N, N> const& M, Vector<T, N> const& B,
        Vector<T, N>& X)
    {
        return GaussianElimination<T>::SolveSystem(N, 1, M.data(), B.data(), X.data());
    }

    template <typename T, size_t N, size_t K>
    bool SolveSystem(Matrix<T, N, N> const& M, Matrix<T, N, K> const& B,
        Matrix<T, N, K>& X)
    {
        return GaussianElimination<T>::SolveSystem(N, K, M.data(), B.data(), X.data());
    }

    template <typename T, size_t N>
    Matrix<T, N, N> Inverse(Matrix<T, N, N> const& M, T* determinant = nullptr)
    {
        Matrix<T, N, N> inverseM;
        T localDeterminant = C_<T>(0);
        (void)GaussianElimination<T>::GetInverseAndDeterminant(N, M.data(),
            inverseM.data(), localDeterminant);
        if (determinant)
        {
            *determinant = localDeterminant;
        }
        return inverseM;
    }

    template <typename T, size_t N>
    T Determinant(Matrix<T, N, N> const& M)
    {
        T determinant = C_<T>(0);
        (void)GaussianElimination<T>::GetDeterminant(N, M.data(), determinant);
        return determinant;
    }

    // Specialized operations for GTL matrices whose sizes are known only at
    // run time.
    template <typename T>
    bool SolveSystem(Matrix<T> const& M, Vector<T> const& B, Vector<T>& X)
    {
        size_t const N = M.GetNumRows();
        GTL_ARGUMENT_ASSERT(
            N > 0 && N == M.GetNumCols() &&
            N == B.size() && N == X.size(),
            "Incorrect dimension or mismatched size.");

        return GaussianElimination<T>::SolveSystem(N, 1, M.data(), B.data(), X.data());
    }

    template <typename T>
    bool SolveSystem(Matrix<T> const& M, Matrix<T> const& B, Matrix<T>& X)
    {
        size_t const N = M.GetNumRows();
        size_t const K = B.GetNumCols();
        GTL_ARGUMENT_ASSERT(
            N > 0 && N == M.GetNumCols() &&
            N == B.GetNumRows() && N == X.GetNumRows() &&
            K == X.GetNumCols(),
            "Incorrect dimension or mismatched size.");

        return GaussianElimination<T>::SolveSystem(N, K, M.data(), B.data(), X.data());
    }

    template <typename T>
    Matrix<T> Inverse(Matrix<T> const& M, T* determinant = nullptr)
    {
        GTL_ARGUMENT_ASSERT(
            M.GetNumRows() == M.GetNumCols(),
            "Matrix must be square.");

        Matrix<T> inverseM(M.GetNumRows(), M.GetNumRows());
        T localDeterminant = C_<T>(0);
        (void)GaussianElimination<T>::GetInverseAndDeterminant(M.GetNumRows(),
            M.data(), inverseM.data(), localDeterminant);
        if (determinant)
        {
            *determinant = localDeterminant;
        }
        return inverseM;
    }

    template <typename T>
    T Determinant(Matrix<T> const& M)
    {
        T determinant = C_<T>(0);
        (void)GaussianElimination<T>::GetDeterminant(M.GetNumRows(),
            M.data(), determinant);
        return determinant;
    }
}
