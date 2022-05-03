// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <GTL/Mathematics/Algebra/Matrix.h>
#include <cmath>

namespace gtl
{
    template <typename T, size_t...> class CholeskyDecomposition;

    // Implementation for size known at compile time. To solve A*X = B,
    //   Matrix<T, N, N> A = user_specified_symmetric_matrix;
    //   Vector<T, N> B = user_specified_vector;
    //   Vector<T, N> X = solution_to_be_determined;
    //   CholeskyDecomposition<T, N> decomposer{};
    //   bool success = decomposer.Factor(A);
    //   if (success)
    //   {
    //       X = B;
    //       decomposer.SolveLower(A, X);
    //       decomposer.SolveUpper(A, X);
    //   }
    //
    // or
    //
    //   Matrix<T, N, N> A = user_specified_symmetric_matrix;
    //   Matrix<T, N, N> L = lower_triangular_to_be_determined;
    //   Vector<T, N> B = user_specified_vector;
    //   Vector<T, N> X = solution_to_be_determined;
    //   CholeskyDecomposition<T, N> decomposer{};
    //   bool success = decomposer.Factor(A, L);
    //   if (success)
    //   {
    //       X = B;
    //       decomposer.SolveLower(L, X);
    //       decomposer.SolveUpper(L, X);
    //   }
    template <typename T, size_t N>
    class CholeskyDecomposition<T, N>
    {
    public:
        CholeskyDecomposition()
        {
            static_assert(
                N > 0,
                "Invalid size in CholeskyDecomposition constructor.");
        }

        // The input matrix A must be symmetric. Only the lower-triangular
        // portion is modified, including the diagonal. On output, the
        // lower-triangular portion is L where A = L * L^T. You can pass A
        // to SolveLower and SolveUpper because those functions access the
        // original entries of A in the lower-triangular part of the matrix.
        bool Factor(Matrix<T, N, N>& A) const
        {
            for (size_t c = 0; c < N; ++c)
            {
                if (A(c, c) <= C_<T>(0))
                {
                    return false;
                }
                A(c, c) = std::sqrt(A(c, c));

                for (size_t r = c + 1; r < N; ++r)
                {
                    A(r, c) /= A(c, c);
                }

                for (size_t k = c + 1; k < N; ++k)
                {
                    auto const& Akc = A(k, c);
                    for (size_t r = k; r < N; ++r)
                    {
                        A(r, k) -= A(r, c) * Akc;
                    }
                }
            }
            return true;
        }

        // The input matrix A must be symmetric. The output matrix is L which
        // is lower triangular and A = L * L^T. You must pass L to SolveLower
        // and SolveUpper.
        bool Factor(Matrix<T, N, N> const& A, Matrix<T, N, N>& L) const
        {
            L = A;
            if (Factor(L))
            {
                for (size_t r = 0; r < N; ++r)
                {
                    for (size_t c = r + 1; c < N; ++c)
                    {
                        L(r, c) = C_<T>(0);
                    }
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        // Solve L*Y = B, where L is lower triangular and invertible. The
        // input value of Y is B. On output, Y is the solution.
        void SolveLower(Matrix<T, N, N> const& L, Vector<T, N>& Y) const
        {
            for (size_t r = 0; r < N; ++r)
            {
                for (size_t c = 0; c < r; ++c)
                {
                    Y[r] -= L(r, c) * Y[c];
                }
                Y[r] /= L(r, r);
            }
        }

        // Solve L^T*X = Y, where L is lower triangular (L^T is upper
        // triangular) and invertible. The input value of X is Y. On
        // output, X is the solution.
        void SolveUpper(Matrix<T, N, N> const& L, Vector<T, N>& X) const
        {
            for (size_t k = 0, r = N - 1; k < N; ++k, --r)
            {
                for (size_t c = r + 1; c < N; ++c)
                {
                    X[r] -= L(c, r) * X[c];
                }
                X[r] /= L(r, r);
            }
        }
    };

    // Implementation for size known only at run time. To solve A*X = B,
    //   size_t const N = user_specified_dimension
    //   Matrix<T> A(N, N) = user_specified_symmetric_matrix;
    //   Vector<T> B(N) = user_specified_vector;
    //   Vector<T> X(N) = solution_to_be_determined;
    //   CholeskyDecomposition<T> decomposer(N);
    //   bool success = decomposer.Factor(A);
    //   if (success)
    //   {
    //       X = B;
    //       decomposer.SolveLower(A, X);
    //       decomposer.SolveUpper(A, X);
    //   }
    //
    // or
    //
    //   size_t const N = user_specified_dimension
    //   Matrix<T> A(N, N) = user_specified_symmetric_matrix;
    //   Matrix<T> L(N, N) = lower_triangular_to_be_determined;
    //   Vector<T> B(N) = user_specified_vector;
    //   Vector<T> X(N) = solution_to_be_determined;
    //   CholeskyDecomposition<T> decomposer(N);
    //   bool success = decomposer.Factor(A, L);
    //   if (success)
    //   {
    //       X = B;
    //       decomposer.SolveLower(L, X);
    //       decomposer.SolveUpper(L, X);
    //   }
    template <typename T>
    class CholeskyDecomposition<T>
    {
    public:
        size_t const N;

        CholeskyDecomposition(size_t inN)
            :
            N(inN)
        {
            GTL_LENGTH_ASSERT(
                N > 0,
                "The matrix size must be positive.");
        }

        // The input matrix A must be symmetric. Only the lower-triangular
        // portion is modified, including the diagonal. On output, the
        // lower-triangular portion is L where A = L * L^T. You can pass A
        // to SolveLower and SolveUpper because those functions access the
        // original entries of A in the lower-triangular part of the matrix.
        bool Factor(Matrix<T>& A) const
        {
            GTL_LENGTH_ASSERT(
                A.GetNumRows() == N && A.GetNumCols() == N,
                "Matrix A must be valid and square.");

            for (size_t c = 0; c < N; ++c)
            {
                if (A(c, c) <= C_<T>(0))
                {
                    return false;
                }
                A(c, c) = std::sqrt(A(c, c));

                for (size_t r = c + 1; r < N; ++r)
                {
                    A(r, c) /= A(c, c);
                }

                for (size_t k = c + 1; k < N; ++k)
                {
                    auto const& Akc = A(k, c);
                    for (size_t r = k; r < N; ++r)
                    {
                        A(r, k) -= A(r, c) * Akc;
                    }
                }
            }
            return true;
        }

        // The input matrix A must be symmetric. The output matrix is L which
        // is lower triangular and A = L * L^T. You must pass L to SolveLower
        // and SolveUpper.
        bool Factor(Matrix<T> const& A, Matrix<T>& L) const
        {
            GTL_LENGTH_ASSERT(
                A.GetNumRows() == N && A.GetNumCols() == N,
                "Matrix A must be valid and square.");

            L = A;
            if (Factor(L))
            {
                for (size_t r = 0; r < N; ++r)
                {
                    for (size_t c = r + 1; c < N; ++c)
                    {
                        L(r, c) = C_<T>(0);
                    }
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        // Solve L*Y = B, where L is lower triangular and invertible. The
        // input value of Y is B. On output, Y is the solution.
        void SolveLower(Matrix<T> const& L, Vector<T>& Y) const
        {
            GTL_LENGTH_ASSERT(
                L.GetNumRows() == N && L.GetNumCols() == N && Y.size() == N,
                "Matrix L must be valid, square and compatible with Y.");

            for (size_t r = 0; r < N; ++r)
            {
                for (size_t c = 0; c < r; ++c)
                {
                    Y[r] -= L(r, c) * Y[c];
                }
                Y[r] /= L(r, r);
            }
        }

        // Solve L^T*X = Y, where L is lower triangular (L^T is upper
        // triangular) and invertible. The input value of X is Y. On
        // output, X is the solution.
        void SolveUpper(Matrix<T> const& L, Vector<T>& X) const
        {
            GTL_LENGTH_ASSERT(
                L.GetNumRows() == N && L.GetNumCols() == N && X.size() == N,
                "Matrix L must be valid, square and compatible with Y.");

            for (size_t k = 0, r = N - 1; k < N; ++k, --r)
            {
                for (size_t c = r + 1; c < N; ++c)
                {
                    X[r] -= L(c, r) * X[c];
                }
                X[r] /= L(r, r);
            }
        }
    };
}
