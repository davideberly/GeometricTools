// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <GTL/Mathematics/Algebra/Matrix.h>
#include <GTL/Mathematics/MatrixAnalysis/GaussianElimination.h>
#include <array>
#include <cmath>
#include <map>
#include <vector>

// Solve linear systems of equations where the matrix A is NxN. The return
// value of a function is 'true' when A is invertible. In this case the
// solution X and the solution is valid.  If the return value is 'false',
// A is not invertible and X and Y are invalid, so do not use them.
//
// The linear solvers that use the conjugate gradient algorithm are based
// on the discussion in "Matrix Computations, 2nd edition" by G. H. Golub
// and Charles F. Van Loan, The Johns Hopkins Press, Baltimore MD, Fourth
// Printing 1993.

namespace gtl
{
    template <typename T>
    class LinearSystem
    {
    public:
        // Solve 2x2, 3x3 and 4x4 systems by inverting the matrix directly.
        // This avoids the overhead of Gaussian elimination in small
        // dimensions.
        static bool Solve(Matrix2x2<T> const& A, Vector2<T> const& B, Vector2<T>& X)
        {
            T determinant = C_<T>(0);
            Matrix2x2<T> invA = GetInverse(A, &determinant);
            bool invertible = (determinant != C_<T>(0));
            if (invertible)
            {
                X = invA * B;
            }
            else
            {
                MakeZero(X);
            }
            return invertible;
        }

        static bool Solve(Matrix3x3<T> const& A, Vector3<T> const& B, Vector3<T>& X)
        {
            T determinant = C_<T>(0);
            Matrix3x3<T> invA = GetInverse(A, &determinant);
            bool invertible = (determinant != C_<T>(0));
            if (invertible)
            {
                X = invA * B;
            }
            else
            {
                MakeZero(X);
            }
            return invertible;
        }

        static bool Solve(Matrix4x4<T> const& A, Vector4<T> const& B, Vector4<T>& X)
        {
            T determinant = C_<T>(0);
            Matrix4x4<T> invA = GetInverse(A, &determinant);
            bool invertible = (determinant != C_<T>(0));
            if (invertible)
            {
                X = invA * B;
            }
            else
            {
                MakeZero(X);
            }
            return invertible;
        }

        static bool Solve(Matrix<T> const& A, Vector<T> const& B, Vector<T>& X)
        {
            return GaussianElimination<T>::SolveSystem(A.GetNumRows(), 1, A.data(),
                B.data(), X.data());
        }

        // Solve A*X = B, where A is numRows-by-numRows and is specified by the
        // caller, where B is numRows-by-numCols and is specified by the caller,
        // and where the solution X is numRows-by-numCols.
        static bool Solve(size_t numRows, size_t numCols, T const* A, T const* B,
            T* X, bool rowMajor = true)
        {
            return GaussianElimination<T>::SolveSystem(numRows, numCols, A, B, X, rowMajor);
        }

        // Solve A*X = B, where A is tridiagonal. The function expects the
        // subdiagonal, diagonal and superdiagonal of A. The diagonal input
        // must have N elements. The subdiagonal and superdiagonal inputs
        // must have N-1 elements.
        static bool SolveTridiagonal(size_t N, T const* subdiagonal,
            T const* diagonal, T const* superdiagonal, T const* B, T* X)
        {
            if (diagonal[0] == C_<T>(0))
            {
                return false;
            }

            std::vector<T> tmp(N - 1);
            T expr = diagonal[0];
            X[0] = B[0] / expr;

            for (size_t i0 = 0, i1 = 1; i1 < N; ++i0, ++i1)
            {
                tmp[i0] = superdiagonal[i0] / expr;
                expr = diagonal[i1] - subdiagonal[i0] * tmp[i0];
                if (expr == C_<T>(0))
                {
                    return false;
                }
                X[i1] = (B[i1] - subdiagonal[i0] * X[i0]) / expr;
            }

            for (size_t k = 1, i0 = N - 1, i1 = N - 2; k < N; ++k, --i0, --i1)
            {
                X[i1] -= tmp[i1] * X[i0];
            }
            return true;
        }

        // Solve A*X = B, where A is tridiagonal. The function expects the
        // subdiagonal, diagonal, and superdiagonal of A. Moreover, the
        // subdiagonal elements are a constant, the diagonal elements are a
        // constant, and the superdiagonal elements are a constant.
        static bool SolveConstantTridiagonal(size_t N, T const& subdiagonal,
            T const& diagonal, T const& superdiagonal, T const* B, T* X)
        {
            if (diagonal == C_<T>(0))
            {
                return false;
            }

            std::vector<T> tmp(N - 1);
            T expr = diagonal;
            X[0] = B[0] / expr;

            for (size_t i0 = 0, i1 = 1; i1 < N; ++i0, ++i1)
            {
                tmp[i0] = superdiagonal / expr;
                expr = diagonal - subdiagonal * tmp[i0];
                if (expr == C_<T>(0))
                {
                    return false;
                }
                X[i1] = (B[i1] - subdiagonal * X[i0]) / expr;
            }

            for (size_t k = 1, i0 = N - 1, i1 = N - 2; k < N; ++k, --i0, --i1)
            {
                X[i1] -= tmp[i1] * X[i0];
            }
            return true;
        }

        // Solve A*X = B using the conjugate gradient method, where A is
        // symmetric. You must specify the maximum number of iterations and a
        // tolerance for terminating the iterations. Reasonable choices for
        // tolerance are 1e-06f for 'float' or 1e-08 for 'double'.
        static size_t SolveSymmetricCG(size_t N, T const* A, T const* B,
            T* X, size_t maxIterations, T const& tolerance)
        {
            // The first iteration.
            std::vector<T> R(N);
            for (size_t i = 0; i < N; ++i)
            {
                X[i] = C_<T>(0);
                R[i] = B[i];
            }
            T rho0 = Dot(N, R.data(), R.data());
            std::vector<T> P = R, W(N);
            Mul(N, A, P.data(), W.data());
            T alpha = rho0 / Dot(N, P.data(), W.data());
            UpdateX(N, X, alpha, P.data());
            UpdateR(N, R.data(), alpha, W.data());
            T rho1 = Dot(N, R.data(), R.data());

            // The remaining iterations.
            size_t iteration{};
            for (iteration = 1; iteration <= maxIterations; ++iteration)
            {
                T root0 = std::sqrt(rho1);
                T norm = Dot(N, B, B);
                T root1 = std::sqrt(norm);
                T cutoff = tolerance * root1;
                if (root0 <= cutoff)
                {
                    break;
                }

                T beta = rho1 / rho0;
                UpdateP(N, P.data(), beta, R.data());
                Mul(N, A, P.data(), W.data());
                alpha = rho1 / Dot(N, P.data(), W.data());
                UpdateX(N, X, alpha, P.data());
                UpdateR(N, R.data(), alpha, W.data());
                rho0 = rho1;
                rho1 = Dot(N, R.data(), R.data());
            }
            return iteration;
        }

        // Solve A*X = B using the conjugate gradient method, where A is 
        // sparse and symmetric. The nonzero entries of the symmetrix matrix
        // A are stored in a map whose keys are pairs (i,j) and whose values
        // are real numbers. The pair (i,j) is the location of the value in
        // the array. Only one of (i,j) and (j,i) should be stored since A is
        // symmetric. The column vector B is stored as an array of contiguous
        // values. You must specify the maximum number of iterations and a
        // tolerance for terminating the iterations. Reasonable choices for
        // tolerance are 1e-06f for 'float' or 1e-08 for 'double'.
        typedef std::map<std::array<size_t, 2>, T> SparseMatrix;
        static size_t SolveSymmetricCG(size_t N, SparseMatrix const& A,
            T const* B, T* X, size_t maxIterations, T const& tolerance)
        {
            // The first iteration.
            std::vector<T> R(N);
            for (size_t i = 0; i < N; ++i)
            {
                X[i] = C_<T>(0);
                R[i] = B[i];
            }
            T rho0 = Dot(N, R.data(), R.data());
            std::vector<T> P = R, W(N);
            Mul(N, A, P.data(), W.data());
            T alpha = rho0 / Dot(N, P.data(), W.data());
            UpdateX(N, X, alpha, P.data());
            UpdateR(N, R.data(), alpha, W.data());
            T rho1 = Dot(N, R.data(), R.data());

            // The remaining iterations.
            size_t iteration{};
            for (iteration = 1; iteration <= maxIterations; ++iteration)
            {
                T root0 = std::sqrt(rho1);
                T norm = Dot(N, B, B);
                T root1 = std::sqrt(norm);
                T cutoff = tolerance * root1;
                if (root0 <= cutoff)
                {
                    break;
                }

                T beta = rho1 / rho0;
                UpdateP(N, P.data(), beta, R.data());
                Mul(N, A, P.data(), W.data());
                alpha = rho1 / Dot(N, P.data(), W.data());
                UpdateX(N, X, alpha, P.data());
                UpdateR(N, R.data(), alpha, W.data());
                rho0 = rho1;
                rho1 = Dot(N, R.data(), R.data());
            }
            return iteration;
        }

    private:
        // Support for the conjugate gradient method.
        static T Dot(size_t N, T const* U, T const* V)
        {
            T dot = C_<T>(0);
            for (size_t i = 0; i < N; ++i)
            {
                dot += U[i] * V[i];
            }
            return dot;
        }

        // A is symmetric, so the matrix storage order is irrelevant.
        static void Mul(size_t N, T const* A, T const* X, T* P)
        {
            for (size_t i = 0; i < N; ++i)
            {
                P[i] = C_<T>(0);
            }

            for (size_t row = 0; row < N; ++row)
            {
                for (size_t col = 0; col < N; ++col)
                {
                    P[row] += A[col + N * row] * X[col];
                }
            }
        }

        static void Mul(size_t N, SparseMatrix const& A, T const* X, T* P)
        {
            for (size_t i = 0; i < N; ++i)
            {
                P[i] = C_<T>(0);
            }

            for (auto const& element : A)
            {
                size_t j0 = element.first[0];
                size_t j1 = element.first[1];
                T const& value = element.second;
                P[j0] += value * X[j1];
                if (j0 != j1)
                {
                    P[j1] += value * X[j0];
                }
            }
        }

        static void UpdateX(size_t N, T* X, T alpha, T const* P)
        {
            for (size_t i = 0; i < N; ++i)
            {
                X[i] += alpha * P[i];
            }
        }

        static void UpdateR(size_t N, T* R, T alpha, T const* W)
        {
            for (size_t i = 0; i < N; ++i)
            {
                R[i] -= alpha * W[i];
            }
        }

        static void UpdateP(size_t N, T* P, T beta, T const* R)
        {
            for (size_t i = 0; i < N; ++i)
            {
                P[i] = R[i] + beta * P[i];
            }
        }
    };
}
