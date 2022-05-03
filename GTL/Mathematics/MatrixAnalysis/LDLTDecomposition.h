// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <GTL/Mathematics/MatrixAnalysis/GaussianElimination.h>

// Factor a positive symmetric matrix A = L * D * L^T, where L is a lower
// triangular matrix with diagonal entries all 1 (L is lower unit triangular)
// and where D is a diagonal matrix with diagonal entries all positive.

namespace gtl
{
    template <typename T, size_t...> class LDLTDecomposition;

    // Implementation for sizes known at compile time.
    template <typename T, size_t N>
    class LDLTDecomposition<T, N>
    {
    public:
        LDLTDecomposition()
        {
            static_assert(
                N > 0,
                "Invalid size.");
        }

        // The matrix A must be positive definite. The implementation uses
        // only the lower-triangular portion of A. On output, L is lower
        // unit triangular and D is diagonal.
        bool Factor(Matrix<T, N, N> const& A, Matrix<T, N, N>& L, Matrix<T, N, N>& D)
        {
            MakeZero(L);
            MakeZero(D);
            for (size_t j = 0; j < N; ++j)
            {
                T Djj = A(j, j);
                for (size_t k = 0; k < j; ++k)
                {
                    T Ljk = L(j, k);
                    T Dkk = D(k, k);
                    Djj -= Ljk * Ljk * Dkk;
                }
                D(j, j) = Djj;
                if (Djj == C_<T>(0))
                {
                    return false;
                }

                L(j, j) = C_<T>(1);
                for (size_t i = j + 1; i < N; ++i)
                {
                    T Lij = A(i, j);
                    for (size_t k = 0; k < j; ++k)
                    {
                        T Lik = L(i, k);
                        T Ljk = L(j, k);
                        T Dkk = D(k, k);
                        Lij -= Lik * Ljk * Dkk;
                    }
                    Lij /= Djj;
                    L(i, j) = Lij;
                }
            }
            return true;
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring before the call.
        void Solve(Matrix<T, N, N> const& L, Matrix<T, N, N> const& D,
            Vector<T, N> const& B, Vector<T, N>& X)
        {
            // Solve L * Z = L * (D * L^T * X) = B for Z.
            for (size_t r = 0; r < N; ++r)
            {
                X[r] = B[r];
                for (size_t c = 0; c < r; ++c)
                {
                    X[r] -= L(r, c) * X[c];
                }
            }

            // Solve D * Y = D * (L^T * X) = Z for Y.
            for (size_t r = 0; r < N; ++r)
            {
                X[r] /= D(r, r);
            }

            // Solve L^T * Y = Z for X.
            for (size_t k = 0, r = N - 1; k < N; ++k, --r)
            {
                for (size_t c = r + 1; c < N; ++c)
                {
                    X[r] -= L(c, r) * X[c];
                }
            }
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring during the call.
        void Solve(Matrix<T, N, N> const& A, Vector<T, N> const& B, Vector<T, N>& X)
        {
            Matrix<T, N, N> L{}, D{};
            Factor(A, L, D);
            Solve(L, D, B, X);
        }
    };

    // Implementation for sizes known only at run time.
    template <typename T>
    class LDLTDecomposition<T>
    {
    public:
        size_t const N;

        LDLTDecomposition(size_t inN)
            :
            N(inN)
        {
            GTL_ARGUMENT_ASSERT(
                N > 0,
                "Invalid size.");
        }

        // The matrix A must be positive definite. The implementation uses
        // only the lower-triangular portion of A. On output, L is lower
        // unit triangular and D is diagonal.
        bool Factor(Matrix<T> const& A, Matrix<T>& L, Matrix<T>& D)
        {
            GTL_ARGUMENT_ASSERT(
                A.GetNumRows() == N && A.GetNumCols() == N,
                "Invalid size.");

            L.resize(N, N);
            MakeZero(L);
            D.resize(N, N);
            MakeZero(D);

            for (size_t j = 0; j < N; ++j)
            {
                T Djj = A(j, j);
                for (size_t k = 0; k < j; ++k)
                {
                    T Ljk = L(j, k);
                    T Dkk = D(k, k);
                    Djj -= Ljk * Ljk * Dkk;
                }
                D(j, j) = Djj;
                if (Djj == C_<T>(0))
                {
                    return false;
                }

                L(j, j) = C_<T>(1);
                for (size_t i = j + 1; i < N; ++i)
                {
                    T Lij = A(i, j);
                    for (size_t k = 0; k < j; ++k)
                    {
                        T Lik = L(i, k);
                        T Ljk = L(j, k);
                        T Dkk = D(k, k);
                        Lij -= Lik * Ljk * Dkk;
                    }

                    Lij /= Djj;
                    L(i, j) = Lij;
                }
            }
            return true;
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring before the call.
        void Solve(Matrix<T> const& L, Matrix<T> const& D,
            Vector<T> const& B, Vector<T>& X)
        {
            GTL_ARGUMENT_ASSERT(
                L.GetNumRows() == N && L.GetNumCols() == N &&
                D.GetNumRows() == N && D.GetNumCols() && B.size() == N,
                "Invalid size.");

            X.resize(N);

            // Solve L * Z = L * (D * L^T * X) = B for Z.
            for (size_t r = 0; r < N; ++r)
            {
                X[r] = B[r];
                for (size_t c = 0; c < r; ++c)
                {
                    X[r] -= L(r, c) * X[c];
                }
            }

            // Solve D * Y = D * (L^T * X) = Z for Y.
            for (size_t r = 0; r < N; ++r)
            {
                X[r] /= D(r, r);
            }

            // Solve L^T * Y = Z for X.
            for (size_t k = 0, r = N - 1; k < N; ++k, --r)
            {
                for (size_t c = r + 1; c < N; ++c)
                {
                    X[r] -= L(c, r) * X[c];
                }
            }
        }

        // Solve A*X = B for positive definite A = L * D * L^T.
        void Solve(Matrix<T> const& A, Vector<T> const& B, Vector<T>& X)
        {
            GTL_ARGUMENT_ASSERT(
                A.GetNumRows() == N && A.GetNumCols() == N && B.size() == N,
                "Invalid size.");

            Matrix<T> L{}, D{};
            Factor(A, L, D);
            Solve(L, D, B, X);
        }
    };
}
