// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// The SymmetricEigensolver class is an implementation of Algorithm 8.2.3
// (Symmetric QR Algorithm) described in "Matrix Computations, 2nd edition"
// by G. H. Golub and C. F. Van Loan, The Johns Hopkins University Press,
// Baltimore MD, Fourth Printing 1993. Algorithm 8.2.1 (Householder
// Tridiagonalization) is used to reduce symmetric matrix A to tridiagonal T.
// Algorithm 8.2.2 (Implicit Symmetric QR Step with Wilkinson Shift) is
// used for the iterative reduction from tridiagonal to diagonal. If A is
// the original matrix, D is the diagonal matrix of eigenvalues and R is
// the rotation matrix whose columns are eigenvectors, then theoretically
// A = R * D * R^T. Numerically, we have errors E = R^T * A * Q - D.
// Algorithm 8.2.3 mentions that one expects |E| is approximately u*|A|,
// where |M| denotes the Frobenius norm of M and where u is the unit roundoff
// for the floating-point arithmetic: 2^{-23} for 'float', which is
// FLT_EPSILON = 1.192092896e-7f, and 2^{-52} for'double', which is
// DBL_EPSILON = 2.2204460492503131e-16.
//
// The condition |a(i,i+1)| <= epsilon*(|a(i,i) + a(i+1,i+1)|) used to
// determine when the reduction decouples to smaller problems is implemented
// as: sum = |a(i,i)| + |a(i+1,i+1)|; sum + |a(i,i+1)| == sum. The idea is
// that the superdiagonal term is small relative to its diagonal neighbors,
// and so it is effectively zero. The unit tests have shown that this
// interpretation of decoupling is effective.
//
// The authors suggest that once you have the tridiagonal matrix, a practical
// implementation will store the diagonal and superdiagonal entries in linear
// arrays, ignoring the theoretically zero values not in the 3-band. This is
// good for cache coherence. The authors also suggest storing the Householder
// vectors in the lower-triangular portion of the matrix to save memory. The
// implementation uses both suggestions.
//
// The construction of the full eigenvector matrix is relatively expensive.
// If you need only a small number of eigenvectors, use the member function
// GetEigenvector(size_t) for the desired eigenvectors.
//
// The list of returned eigenvalues is non-increasing (smallest to largest).
// The eigenvectors are ordered accordingly.

#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <GTL/Utility/Exceptions.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace gtl
{
    template <typename T, size_t...> class SymmetricEigensolver;

    template <typename T>
    class SymmetricEigensolver<T, 2>
    {
    public:
        // The default constructor sets all members to zero.
        SymmetricEigensolver()
            :
            mEigenvalues{ C_<T>(0), C_<T>(0) },
            mEigenvectors
            {{
                { C_<T>(0), C_<T>(0) },
                { C_<T>(0), C_<T>(0) }
            }}
        {
        }

        // Solve the eigensystem.
        void operator()(T const& a00, T const& a01, T const& a11)
        {
            // Normalize (c2,s2) robustly, avoiding floating-point overflow
            // in the sqrt call.
            T c2 = C_<T>(1, 2) * (a00 - a11);
            T s2 = a01;
            T maxAbsComp = std::max(std::fabs(c2), std::fabs(s2));
            if (maxAbsComp > C_<T>(0))
            {
                c2 /= maxAbsComp;  // in [-1,1]
                s2 /= maxAbsComp;  // in [-1,1]
                T length = std::sqrt(c2 * c2 + s2 * s2);
                c2 /= length;
                s2 /= length;
                if (c2 > C_<T>(0))
                {
                    c2 = -c2;
                    s2 = -s2;
                }
            }
            else
            {
                c2 = -C_<T>(1);
                s2 = C_<T>(0);
            }

            T s = std::sqrt(C_<T>(1, 2) * (C_<T>(1) - c2));  // >= 1/sqrt(2)
            T c = C_<T>(1, 2) * s2 / s;

            T csqr = c * c;
            T ssqr = s * s;
            T mid = s2 * a01;
            T diagonal0 = csqr * a00 + mid + ssqr * a11;
            T diagonal1 = csqr * a11 - mid + ssqr * a00;

            if (diagonal0 <= diagonal1)
            {
                mEigenvalues = { diagonal0, diagonal1 };
                mEigenvectors = { { { c, s }, { -s, c } } };
            }
            else
            {
                mEigenvalues = { diagonal1, diagonal0 };
                mEigenvectors = { { { s, -c }, { c, s } } };
            }
        }


        // Get a single eigenvalue.
        T const& GetEigenvalue(size_t i) const
        {
            GTL_OUTOFRANGE_ASSERT(
                i < 2,
                "The index must not exceed the matrix size.");

            return mEigenvalues[i];
        }

        // Get all eigenvalues.
        std::array<T, 2> const& GetEigenvalues() const
        {
            return mEigenvalues;
        }

        // Get a single eigenvector.
        std::array<T, 2> const& GetEigenvector(size_t i) const
        {
            GTL_OUTOFRANGE_ASSERT(
                i < 2,
                "The index must not exceed the matrix size.");

            return mEigenvectors[i];
        }

        // Get all eigenvectors.
        std::array<std::array<T, 2>, 2> const& GetEigenvectors() const
        {
            return mEigenvectors;
        }

    private:
        std::array<T, 2> mEigenvalues;
        std::array<std::array<T, 2>, 2> mEigenvectors;

        friend class UnitTestSymmetricEigensolver2;
    };

    template <typename T>
    class SymmetricEigensolver<T, 3>
    {
    public:
        SymmetricEigensolver()
            :
            mEigenvalues{ C_<T>(0), C_<T>(0) },
            mEigenvectors
        { {
            { C_<T>(0), C_<T>(0), C_<T>(0) },
            { C_<T>(0), C_<T>(0), C_<T>(0) },
            { C_<T>(0), C_<T>(0), C_<T>(0) }
        } }
        {
        }

        // Solve the eigensystem.
        //
        // If 'noniterative' is set to 'true', the algorithm preconditions the
        // input matrix in order to solve robustly for the roots of a cubic
        // polynomial. See the comments at the beginning of the file for a
        // reference to the PDF that describes the algorithm. Set
        // 'noniterative' to 'false' for an iterative algorithm. This version
        // is more accurate when the matrix has (numerically nearly) repeated
        // eigenvalues.
        //
        // The 'aggressive' parameter is relevant only for the iterative
        // algorithm. If 'aggressive' is set to 'true', the iterations occur
        // until a superdiagonal entry is exactly zero. If 'aggressive' is
        // 'false', the iterations occur until a superdiagonal entry is
        // effectively zero compared to the sum of magnitudes of its diagonal
        // neighbors. Generally, the nonaggressive convergence is acceptable.
        //
        // The function size_t return value is the number of iterations used
        // by the iterative algorithm. The return value is 0 for the
        // noniterative algorithm.
        size_t operator()(T const& a00, T const& a01, T const& a02, T const& a11,
            T const& a12, T const& a22, bool noniterative, bool aggressive = false)
        {
            if (noniterative)
            {
                return SolveNoniterative(a00, a01, a02, a11, a12, a22);
            }
            else
            {
                return SolveIterative(a00, a01, a02, a11, a12, a22, aggressive);
            }
        }


        // Get a single eigenvalue.
        T const& GetEigenvalue(size_t i) const
        {
            GTL_ARGUMENT_ASSERT(
                i < 3,
                "The index must not exceed the matrix size.");

            return mEigenvalues[i];
        }

        // Get all eigenvalues.
        std::array<T, 3> const& GetEigenvalues() const
        {
            return mEigenvalues;
        }

        // Get a single eigenvector.
        std::array<T, 3> const& GetEigenvector(size_t i) const
        {
            GTL_ARGUMENT_ASSERT(
                i < 3,
                "The index must not exceed the matrix size.");

            return mEigenvectors[i];
        }

        // Get all eigenvectors.
        std::array<std::array<T, 3>, 3> const& GetEigenvectors() const
        {
            return mEigenvectors;
        }

    private:
        // Sorting code is shared by the iterative/noniterative algorithms.
        // Sort the eigenvalues to eval[0] <= eval[1] <= eval[2].
        void SortEigenstuff(bool isRotation)
        {
            std::array<size_t, 3> index = { 0, 0, 0 };
            if (mEigenvalues[0] < mEigenvalues[1])
            {
                if (mEigenvalues[2] < mEigenvalues[0])
                {
                    // even permutation
                    index[0] = 2;
                    index[1] = 0;
                    index[2] = 1;
                }
                else if (mEigenvalues[2] < mEigenvalues[1])
                {
                    // odd permutation
                    index[0] = 0;
                    index[1] = 2;
                    index[2] = 1;
                    isRotation = !isRotation;
                }
                else
                {
                    // even permutation
                    index[0] = 0;
                    index[1] = 1;
                    index[2] = 2;
                }
            }
            else
            {
                if (mEigenvalues[2] < mEigenvalues[1])
                {
                    // odd permutation
                    index[0] = 2;
                    index[1] = 1;
                    index[2] = 0;
                    isRotation = !isRotation;
                }
                else if (mEigenvalues[2] < mEigenvalues[0])
                {
                    // even permutation
                    index[0] = 1;
                    index[1] = 2;
                    index[2] = 0;
                }
                else
                {
                    // odd permutation
                    index[0] = 1;
                    index[1] = 0;
                    index[2] = 2;
                    isRotation = !isRotation;
                }
            }

            auto unorderedEigenValues = mEigenvalues;
            auto unorderedEigenvectors = mEigenvectors;
            for (size_t j = 0; j < 3; ++j)
            {
                size_t i = index[j];
                mEigenvalues[j] = unorderedEigenValues[i];
                mEigenvectors[j] = unorderedEigenvectors[i];
            }

            // Ensure the ordered eigenvectors form a right-handed basis.
            if (!isRotation)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    mEigenvectors[2][j] = -mEigenvectors[2][j];
                }
            }
        }

        // Code for the iterative algorithm.
        size_t SolveIterative(T const& a00, T const& a01, T const& a02, T const& a11,
            T const& a12, T const& a22, bool aggressive)
        {
            // Compute the Householder reflection H and B = H*A*H,
            // where b02 = 0.
            bool isRotation = false;
            T c = C_<T>(0), s = C_<T>(0);
            GetCosSin(a12, -a02, c, s);
            std::array<std::array<T, 3>, 3> Q =
            { {
                { c, s, C_<T>(0) },
                { s, -c, C_<T>(0) },
                { C_<T>(0), C_<T>(0), C_<T>(1) }
            } };
            T term0 = c * a00 + s * a01;
            T term1 = c * a01 + s * a11;
            T b00 = c * term0 + s * term1;
            T b01 = s * term0 - c * term1;
            term0 = s * a00 - c * a01;
            term1 = s * a01 - c * a11;
            T b11 = s * term0 - c * term1;
            T b12 = s * a02 - c * a12;
            T b22 = a22;

            // Givens reflections, B' = G^T*B*G, preserve tridiagonal
            // matrices.
            size_t const maxIteration = static_cast<size_t>(
                2 * (1 + std::numeric_limits<T>::digits -
                    std::numeric_limits<T>::min_exponent));
            size_t iteration = 0;
            T c2 = C_<T>(0), s2 = C_<T>(0);

            if (std::fabs(b12) <= std::fabs(b01))
            {
                T saveB00{}, saveB01{}, saveB11{};
                for (iteration = 0; iteration < maxIteration; ++iteration)
                {
                    // Compute the Givens reflection.
                    GetCosSin(C_<T>(1, 2) * (b00 - b11), b01, c2, s2);
                    s = std::sqrt(C_<T>(1, 2) * (C_<T>(1) - c2));  // >= 1/sqrt(2)
                    c = C_<T>(1, 2) * s2 / s;

                    // Update Q by the Givens reflection.
                    Update0(Q, c, s);
                    isRotation = !isRotation;

                    // Update B <- Q^T*B*Q, ensuring that b02 is zero and
                    // |b12| has strictly decreased.
                    saveB00 = b00;
                    saveB01 = b01;
                    saveB11 = b11;
                    term0 = c * saveB00 + s * saveB01;
                    term1 = c * saveB01 + s * saveB11;
                    b00 = c * term0 + s * term1;
                    b11 = b22;
                    term0 = c * saveB01 - s * saveB00;
                    term1 = c * saveB11 - s * saveB01;
                    b22 = c * term1 - s * term0;
                    b01 = s * b12;
                    b12 = c * b12;

                    if (Converged(aggressive, b00, b11, b01))
                    {
                        // Compute the Householder reflection.
                        GetCosSin(C_<T>(1, 2) * (b00 - b11), b01, c2, s2);
                        s = std::sqrt(C_<T>(1, 2) * (C_<T>(1) - c2));
                        c = C_<T>(1, 2) * s2 / s;  // >= 1/sqrt(2)

                        // Update Q by the Householder reflection.
                        Update2(Q, c, s);
                        isRotation = !isRotation;

                        // Update D = Q^T*B*Q.
                        saveB00 = b00;
                        saveB01 = b01;
                        saveB11 = b11;
                        term0 = c * saveB00 + s * saveB01;
                        term1 = c * saveB01 + s * saveB11;
                        b00 = c * term0 + s * term1;
                        term0 = s * saveB00 - c * saveB01;
                        term1 = s * saveB01 - c * saveB11;
                        b11 = s * term0 - c * term1;
                        break;
                    }
                }
            }
            else
            {
                T saveB11{}, saveB12{}, saveB22{};
                for (iteration = 0; iteration < maxIteration; ++iteration)
                {
                    // Compute the Givens reflection.
                    GetCosSin(C_<T>(1, 2) * (b22 - b11), b12, c2, s2);
                    s = std::sqrt(C_<T>(1, 2) * (C_<T>(1) - c2));  // >= 1/sqrt(2)
                    c = C_<T>(1, 2) * s2 / s;

                    // Update Q by the Givens reflection.
                    Update1(Q, c, s);
                    isRotation = !isRotation;

                    // Update B <- Q^T*B*Q, ensuring that b02 is zero and
                    // |b01| has strictly decreased.
                    saveB11 = b11;
                    saveB12 = b12;
                    saveB22 = b22;
                    term0 = c * saveB22 + s * saveB12;
                    term1 = c * saveB12 + s * saveB11;
                    b22 = c * term0 + s * term1;
                    b11 = b00;
                    term0 = c * saveB12 - s * saveB22;
                    term1 = c * saveB11 - s * saveB12;
                    b00 = c * term1 - s * term0;
                    b12 = s * b01;
                    b01 = c * b01;

                    if (Converged(aggressive, b11, b22, b12))
                    {
                        // Compute the Householder reflection.
                        GetCosSin(C_<T>(1, 2) * (b11 - b22), b12, c2, s2);
                        s = std::sqrt(C_<T>(1, 2) * (C_<T>(1) - c2));
                        c = C_<T>(1, 2) * s2 / s;  // >= 1/sqrt(2)

                        // Update Q by the Householder reflection.
                        Update3(Q, c, s);
                        isRotation = !isRotation;

                        // Update D = Q^T*B*Q.
                        saveB11 = b11;
                        saveB12 = b12;
                        saveB22 = b22;
                        term0 = c * saveB11 + s * saveB12;
                        term1 = c * saveB12 + s * saveB22;
                        b11 = c * term0 + s * term1;
                        term0 = s * saveB11 - c * saveB12;
                        term1 = s * saveB12 - c * saveB22;
                        b22 = s * term0 - c * term1;
                        break;
                    }
                }
            }

            mEigenvalues = { b00, b11, b22 };
            for (size_t row = 0; row < 3; ++row)
            {
                for (size_t col = 0; col < 3; ++col)
                {
                    mEigenvectors[row][col] = Q[col][row];
                }
            }

            SortEigenstuff(isRotation);
            return iteration;
        }

        // Update Q = Q*G in-place using G = {{c,0,-s},{s,0,c},{0,0,1}}.
        static void Update0(std::array<std::array<T, 3>, 3>& Q, T const& c, T const& s)
        {
            for (size_t r = 0; r < 3; ++r)
            {
                T tmp0 = c * Q[r][0] + s * Q[r][1];
                T tmp1 = Q[r][2];
                T tmp2 = c * Q[r][1] - s * Q[r][0];
                Q[r][0] = tmp0;
                Q[r][1] = tmp1;
                Q[r][2] = tmp2;
            }
        }

        // Update Q = Q*G in-place using G = {{0,1,0},{c,0,s},{-s,0,c}}.
        static void Update1(std::array<std::array<T, 3>, 3>& Q, T const& c, T const& s)
        {
            for (size_t r = 0; r < 3; ++r)
            {
                T tmp0 = c * Q[r][1] - s * Q[r][2];
                T tmp1 = Q[r][0];
                T tmp2 = c * Q[r][2] + s * Q[r][1];
                Q[r][0] = tmp0;
                Q[r][1] = tmp1;
                Q[r][2] = tmp2;
            }
        }

        // Update Q = Q*H in-place using H = {{c,s,0},{s,-c,0},{0,0,1}}.
        static void Update2(std::array<std::array<T, 3>, 3>& Q, T const& c, T const& s)
        {
            for (size_t r = 0; r < 3; ++r)
            {
                T tmp0 = c * Q[r][0] + s * Q[r][1];
                T tmp1 = s * Q[r][0] - c * Q[r][1];
                Q[r][0] = tmp0;
                Q[r][1] = tmp1;
            }
        }

        // Update Q = Q*H in-place using H = {{1,0,0},{0,c,s},{0,s,-c}}.
        static void Update3(std::array<std::array<T, 3>, 3>& Q, T const& c, T const& s)
        {
            for (size_t r = 0; r < 3; ++r)
            {
                T tmp0 = c * Q[r][1] + s * Q[r][2];
                T tmp1 = s * Q[r][1] - c * Q[r][2];
                Q[r][1] = tmp0;
                Q[r][2] = tmp1;
            }
        }

        // Normalize (u,v) robustly, avoiding floating-point overflow in the
        // sqrt call. The normalized pair is (cs,sn) with cs <= 0. If
        // (u,v) = (0,0), the function returns (cs,sn) = (-1,0). When used
        // to generate a Householder reflection, it does not matter whether
        // (cs,sn) or (-cs,-sn) is used. When generating a Givens reflection,
        // cs = cos(2*theta) and sn = sin(2*theta). Having a negative cosine
        // for the double-angle term ensures that the single-angle terms
        // c = cos(theta) and s = sin(theta) satisfy |c| <= |s|.
        static void GetCosSin(T const& u, T const& v, T& cs, T& sn)
        {
            T maxAbsComp = std::max(std::fabs(u), std::fabs(v));
            if (maxAbsComp > C_<T>(0))
            {
                T uScaled = u / maxAbsComp;  // in [-1,1]
                T vScaled = v / maxAbsComp;  // in [-1,1]
                T length = std::sqrt(uScaled * uScaled + vScaled * vScaled);
                cs = uScaled / length;
                sn = vScaled / length;
                if (cs > C_<T>(0))
                {
                    cs = -cs;
                    sn = -sn;
                }
            }
            else
            {
                cs = -C_<T>(1);
                sn = C_<T>(0);
            }
        }

        // The convergence test. When aggressive is 'true', the superdiagonal
        // test is "bSuper == 0". When aggressive is 'false', the
        // superdiagonal test is
        //   |bDiag0| + |bDiag1| + |bSuper| == |bDiag0| + |bDiag1|
        // which means bSuper is effectively zero compared to the sizes of the
        // diagonal entries.
        bool Converged(bool aggressive, T const& bDiag0, T const& bDiag1, T const& bSuper)
        {
            if (aggressive)
            {
                return bSuper == C_<T>(0);
            }
            else
            {
                T sum = std::fabs(bDiag0) + std::fabs(bDiag1);
                return sum + std::fabs(bSuper) == sum;
            }
        }

        // Code for the noniterative algorithm. The inputs are passed by value
        // because they are modified internally.
        size_t SolveNoniterative(T a00, T a01, T a02, T a11, T a12, T a22)
        {
            // Precondition the matrix by factoring out the maximum absolute
            // value of the components. This guards against floating-point
            // overflow when computing the eigenvalues.
            T max0 = std::max(std::fabs(a00), std::fabs(a01));
            T max1 = std::max(std::fabs(a02), std::fabs(a11));
            T max2 = std::max(std::fabs(a12), std::fabs(a22));
            T maxAbsElement = std::max(std::max(max0, max1), max2);
            if (maxAbsElement == C_<T>(0))
            {
                // A is the zero matrix.
                mEigenvalues[0] = C_<T>(0);
                mEigenvalues[1] = C_<T>(0);
                mEigenvalues[2] = C_<T>(0);
                mEigenvectors[0] = { C_<T>(1), C_<T>(0), C_<T>(0) };
                mEigenvectors[1] = { C_<T>(0), C_<T>(1), C_<T>(0) };
                mEigenvectors[2] = { C_<T>(0), C_<T>(0), C_<T>(1) };
                return 0;
            }

            T invMaxAbsElement = C_<T>(1) / maxAbsElement;
            a00 *= invMaxAbsElement;
            a01 *= invMaxAbsElement;
            a02 *= invMaxAbsElement;
            a11 *= invMaxAbsElement;
            a12 *= invMaxAbsElement;
            a22 *= invMaxAbsElement;

            T norm = a01 * a01 + a02 * a02 + a12 * a12;
            if (norm > C_<T>(0))
            {
                // Compute the eigenvalues of A.

                // In the PDF mentioned previously, B = (A - q*I)/p, where
                // q = tr(A)/3 with tr(A) the trace of A (sum of the diagonal
                // entries of A) and where p = sqrt(tr((A - q*I)^2)/6).
                T q = (a00 + a11 + a22) / C_<T>(3);

                // The matrix A - q*I is represented by the following, where
                // b00, b11 and b22 are computed after these comments,
                //   +-           -+
                //   | b00 a01 a02 |
                //   | a01 b11 a12 |
                //   | a02 a12 b22 |
                //   +-           -+
                T b00 = a00 - q;
                T b11 = a11 - q;
                T b22 = a22 - q;

                // The is the variable p mentioned in the PDF.
                T p = std::sqrt((b00 * b00 + b11 * b11 + b22 * b22 + norm * C_<T>(2)) / C_<T>(6));

                // We need det(B) = det((A - q*I)/p) = det(A - q*I)/p^3.  The
                // value det(A - q*I) is computed using a cofactor expansion
                // by the first row of A - q*I.  The cofactors are c00, c01
                // and c02 and the determinant is b00*c00 - a01*c01 + a02*c02.
                // The det(B) is then computed finally by the division
                // with p^3.
                T c00 = b11 * b22 - a12 * a12;
                T c01 = a01 * b22 - a12 * a02;
                T c02 = a01 * a12 - b11 * a02;
                T det = (b00 * c00 - a01 * c01 + a02 * c02) / (p * p * p);

                // The halfDet value is cos(3*theta) mentioned in the PDF. The
                // acos(z) function requires |z| <= 1, but will fail silently
                // and return NaN if the input is larger than 1 in magnitude.
                // To avoid this problem due to rounding errors, the halfDet
                // value is clamped to [-1,1].
                T halfDet = det * C_<T>(1, 2);
                halfDet = std::min(std::max(halfDet, -C_<T>(1)), C_<T>(1));

                // The eigenvalues of B are ordered beta0 <= beta1 <= beta2.
                T angle = std::acos(halfDet) / C_<T>(3);
                T beta2 = std::cos(angle) * C_<T>(2);
                T beta0 = std::cos(angle + C_<T>(2, 3) * C_PI<T>) * C_<T>(2);
                T beta1 = -(beta0 + beta2);

                // The eigenvalues of A are ordered as
                // alpha0 <= alpha1 <= alpha2.
                mEigenvalues[0] = q + p * beta0;
                mEigenvalues[1] = q + p * beta1;
                mEigenvalues[2] = q + p * beta2;

                // Compute the eigenvectors so that the set
                // {evec[0], evec[1], evec[2]} is right handed and
                // orthonormal.
                std::array<std::array<T, 3>, 3> evec{};
                if (halfDet >= C_<T>(0))
                {
                    ComputeEigenvector0(a00, a01, a02, a11, a12, a22,
                        mEigenvalues[2], evec[2]);

                    ComputeEigenvector1(a00, a01, a02, a11, a12, a22,
                        evec[2], mEigenvalues[1], evec[1]);

                    evec[0] = Cross(evec[1], evec[2]);
                }
                else
                {
                    ComputeEigenvector0(a00, a01, a02, a11, a12, a22,
                        mEigenvalues[0], evec[0]);

                    ComputeEigenvector1(a00, a01, a02, a11, a12, a22,
                        evec[0], mEigenvalues[1], evec[1]);

                    evec[2] = Cross(evec[0], evec[1]);
                }
                for (size_t i = 0; i < 3; ++i)
                {
                    mEigenvectors[i] = { evec[i][0], evec[i][1], evec[i][2] };
                }
            }
            else
            {
                // The matrix is diagonal.
                mEigenvalues[0] = a00;
                mEigenvalues[1] = a11;
                mEigenvalues[2] = a22;
                mEigenvectors[0] = { C_<T>(1), C_<T>(0), C_<T>(0) };
                mEigenvectors[1] = { C_<T>(0), C_<T>(1), C_<T>(0) };
                mEigenvectors[2] = { C_<T>(0), C_<T>(0), C_<T>(1) };
            }

            // The preconditioning scaled the matrix A, which scales the
            // eigenvalues.  Revert the scaling.
            mEigenvalues[0] *= maxAbsElement;
            mEigenvalues[1] *= maxAbsElement;
            mEigenvalues[2] *= maxAbsElement;

            SortEigenstuff(true);
            return 0;
        }

        // Vector algebra in 3D is implemented here using std::array to avoid the
        // dependency on the GTL Vector3 class.
        static std::array<T, 3> Multiply(T s, std::array<T, 3> const& U)
        {
            std::array<T, 3> product = { s * U[0], s * U[1], s * U[2] };
            return product;
        }

        static std::array<T, 3> Subtract(std::array<T, 3> const& U, std::array<T, 3> const& V)
        {
            std::array<T, 3> difference = { U[0] - V[0], U[1] - V[1], U[2] - V[2] };
            return difference;
        }

        static std::array<T, 3> Divide(std::array<T, 3> const& U, T s)
        {
            std::array<T, 3> division = { U[0] / s, U[1] / s, U[2] / s };
            return division;
        }

        static T Dot(std::array<T, 3> const& U, std::array<T, 3> const& V)
        {
            T dot = U[0] * V[0] + U[1] * V[1] + U[2] * V[2];
            return dot;
        }

        static std::array<T, 3> Cross(std::array<T, 3> const& U, std::array<T, 3> const& V)
        {
            std::array<T, 3> cross =
            {
                U[1] * V[2] - U[2] * V[1],
                U[2] * V[0] - U[0] * V[2],
                U[0] * V[1] - U[1] * V[0]
            };
            return cross;
        }

        void ComputeOrthogonalComplement(std::array<T, 3> const& W,
            std::array<T, 3>& U, std::array<T, 3>& V) const
        {
            // Robustly compute a right-handed orthonormal set { U, V, W }.
            // The vector W is guaranteed to be unit-length, in which case
            // there is no need to worry about a division by zero when
            // computing invLength.
            T invLength;
            if (std::fabs(W[0]) > std::fabs(W[1]))
            {
                // The component of maximum absolute value is either W[0]
                // or W[2].
                invLength = C_<T>(1) / std::sqrt(W[0] * W[0] + W[2] * W[2]);
                U = { -W[2] * invLength, C_<T>(0), +W[0] * invLength };
            }
            else
            {
                // The component of maximum absolute value is either W[1]
                // or W[2].
                invLength = C_<T>(1) / std::sqrt(W[1] * W[1] + W[2] * W[2]);
                U = { C_<T>(0), +W[2] * invLength, -W[1] * invLength };
            }
            V = Cross(W, U);
        }

        void ComputeEigenvector0(T a00, T a01, T a02, T a11, T a12, T a22,
            T eval0, std::array<T, 3>& evec0) const
        {
            // Compute a unit-length eigenvector for eigenvalue[i0]. The
            // matrix is rank 2, so two of the rows are linearly independent.
            // For a robust computation of the eigenvector, select the two
            // rows whose cross product has largest length of all pairs of
            // rows.
            std::array<T, 3> row0 = { a00 - eval0, a01, a02 };
            std::array<T, 3> row1 = { a01, a11 - eval0, a12 };
            std::array<T, 3> row2 = { a02, a12, a22 - eval0 };
            std::array<T, 3>  r0xr1 = Cross(row0, row1);
            std::array<T, 3>  r0xr2 = Cross(row0, row2);
            std::array<T, 3>  r1xr2 = Cross(row1, row2);
            T d0 = Dot(r0xr1, r0xr1);
            T d1 = Dot(r0xr2, r0xr2);
            T d2 = Dot(r1xr2, r1xr2);

            T dmax = d0;
            size_t imax = 0;
            if (d1 > dmax)
            {
                dmax = d1;
                imax = 1;
            }
            if (d2 > dmax)
            {
                imax = 2;
            }

            if (imax == 0)
            {
                evec0 = Divide(r0xr1, std::sqrt(d0));
            }
            else if (imax == 1)
            {
                evec0 = Divide(r0xr2, std::sqrt(d1));
            }
            else
            {
                evec0 = Divide(r1xr2, std::sqrt(d2));
            }
        }

        void ComputeEigenvector1(T a00, T a01, T a02, T a11, T a12, T a22,
            std::array<T, 3> const& evec0, T eval1, std::array<T, 3>& evec1) const
        {
            // Robustly compute a right-handed orthonormal set {U, V, evec0}.
            std::array<T, 3> U{}, V{};
            ComputeOrthogonalComplement(evec0, U, V);

            // Let e be eval1 and let E be a corresponding eigenvector which
            // is a solution to the linear system (A - e*I)*E = 0. The matrix
            // (A - e*I) is 3x3, not invertible (so infinitely many
            // solutions), and has rank 2 when eval1 and eval are different.
            // It has rank 1 when eval1 and eval2 are equal. Numerically, it
            // is difficult to compute robustly the rank of a matrix.  Instead,
            // the 3x3 linear system is reduced to a 2x2 system as follows.
            // Define the 3x2 matrix J = [U V] whose columns are the U and V
            // computed previously. Define the 2x1 vector X = J*E.  The 2x2
            // system is 0 = M * X = (J^T * (A - e*I) * J) * X where J^T is
            // the transpose of J and M = J^T * (A - e*I) * J is a 2x2 matrix.
            // The system may be written as
            //     +-                        -++-  -+       +-  -+
            //     | U^T*A*U - e  U^T*A*V     || x0 | = e * | x0 |
            //     | V^T*A*U      V^T*A*V - e || x1 |       | x1 |
            //     +-                        -++   -+       +-  -+
            // where X has row entries x0 and x1.

            std::array<T, 3> AU =
            {
                a00 * U[0] + a01 * U[1] + a02 * U[2],
                a01 * U[0] + a11 * U[1] + a12 * U[2],
                a02 * U[0] + a12 * U[1] + a22 * U[2]
            };

            std::array<T, 3> AV =
            {
                a00 * V[0] + a01 * V[1] + a02 * V[2],
                a01 * V[0] + a11 * V[1] + a12 * V[2],
                a02 * V[0] + a12 * V[1] + a22 * V[2]
            };

            T m00 = U[0] * AU[0] + U[1] * AU[1] + U[2] * AU[2] - eval1;
            T m01 = U[0] * AV[0] + U[1] * AV[1] + U[2] * AV[2];
            T m11 = V[0] * AV[0] + V[1] * AV[1] + V[2] * AV[2] - eval1;

            // For robustness, choose the largest-length row of M to compute
            // the eigenvector. The 2-tuple of coefficients of U and V in the
            // assignments to eigenvector[1] lies on a circle, and U and V are
            // unit length and perpendicular, so eigenvector[1] is unit length
            // (within numerical tolerance).
            T absM00 = std::fabs(m00);
            T absM01 = std::fabs(m01);
            T absM11 = std::fabs(m11);
            T maxAbsComp;
            if (absM00 >= absM11)
            {
                maxAbsComp = std::max(absM00, absM01);
                if (maxAbsComp > C_<T>(0))
                {
                    if (absM00 >= absM01)
                    {
                        m01 /= m00;
                        m00 = C_<T>(1) / std::sqrt(C_<T>(1) + m01 * m01);
                        m01 *= m00;
                    }
                    else
                    {
                        m00 /= m01;
                        m01 = C_<T>(1) / std::sqrt(C_<T>(1) + m00 * m00);
                        m00 *= m01;
                    }
                    evec1 = Subtract(Multiply(m01, U), Multiply(m00, V));
                }
                else
                {
                    evec1 = U;
                }
            }
            else
            {
                maxAbsComp = std::max(absM11, absM01);
                if (maxAbsComp > C_<T>(0))
                {
                    if (absM11 >= absM01)
                    {
                        m01 /= m11;
                        m11 = C_<T>(1) / std::sqrt(C_<T>(1) + m01 * m01);
                        m01 *= m11;
                    }
                    else
                    {
                        m11 /= m01;
                        m01 = C_<T>(1) / std::sqrt(C_<T>(1) + m11 * m11);
                        m11 *= m01;
                    }
                    evec1 = Subtract(Multiply(m11, U), Multiply(m01, V));
                }
                else
                {
                    evec1 = U;
                }
            }
        }

        std::array<T, 3> mEigenvalues;
        std::array<std::array<T, 3>, 3> mEigenvectors;

        friend class UnitTestSymmetricEigensolver3;
    };

    template <typename T>
    class SymmetricEigensolver<T>
    {
    public:
        static size_t constexpr invalid = std::numeric_limits<size_t>::max();

        SymmetricEigensolver()
            :
            mSize(0),
            mMatrix{},
            mDiagonal{},
            mSuperdiagonal{},
            mEigenvalues{},
            mEigenvectors{},
            mGivens{},
            mVVector{},
            mPVector{},
            mWVector{}
        {
        }

        // Solve the eigensystem. The input is size-by-size and is stored in
        // size*size contiguous elements. The input must be symmetric, in
        // which case it does not matter whether it is stored in row-major or
        // column-major order. The maxIterations is the maximum number of
        // iterations allowed by the solver. The function size_t return value
        // is the number of iterations used by the iterative algorithm.
        size_t operator()(size_t size, T const* input, size_t maxIterations)
        {
            GTL_ARGUMENT_ASSERT(
                size > 0 && input != nullptr && maxIterations > 0,
                "Invalid input.");

            // The algorithm is implemented using a copy of the input matrix but
            // stored in row-major order.
            mSize = size;
            mMatrix.resize(mSize * mSize);
            for (size_t i = 0; i < mMatrix.size(); ++i)
            {
                mMatrix[i] = input[i];
            }

            // Resize the members used in the decomposition.
            mEigenvalues.resize(mSize);
            mEigenvectors.resize(mSize);
            for (size_t i = 0; i < mSize; ++i)
            {
                mEigenvectors[i].resize(mSize);
            }

            mDiagonal.resize(mSize);
            mSuperdiagonal.resize(mSize - 1);
            mGivens.resize(maxIterations * (mSize - 1));
            mVVector.resize(mSize);
            mPVector.resize(mSize);
            mWVector.resize(mSize);

            Tridiagonalize();
            mGivens.clear();
            for (size_t j = 0; j < maxIterations; ++j)
            {
                size_t imin = invalid, imax = invalid;
                for (size_t k = 2, i = mSize - 2; k <= mSize; ++k, --i)
                {
                    // When a01 is much smaller than its diagonal
                    // neighbors, it is effectively zero.
                    T a00 = mDiagonal[i];
                    T a01 = mSuperdiagonal[i];
                    T a11 = mDiagonal[i + 1];
                    T sum = std::fabs(a00) + std::fabs(a11);
                    if (sum + std::fabs(a01) != sum)
                    {
                        if (imax == invalid)
                        {
                            imax = i;
                        }
                        imin = i;
                    }
                    else
                    {
                        // The superdiagonal term is effectively zero
                        // compared to the neighboring diagonal terms.
                        if (imin != invalid)
                        {
                            break;
                        }
                    }
                }

                if (imax == invalid)
                {
                    // The algorithm has converged. Store the eigenvalues.
                    // Compute the eigenvectors from the Householder and
                    // Givens rotations and store them.
                    RepackageEigenstuff();
                    return j;
                }

                // Process the lower-right-most unreduced tridiagonal
                // block.
                DoQRImplicitShift(imin, imax);
            }
            return invalid;
        }


        // Get a single eigenvalue.
        T const& GetEigenvalue(size_t i) const
        {
            GTL_OUTOFRANGE_ASSERT(
                i < mSize,
                "The index must not exceed the matrix size.");

            return mEigenvalues[i];
        }

        // Get the eigenvalues as a vector of numbers.
        inline std::vector<T> const& GetEigenvalues() const
        {
            return mEigenvalues;
        }

        // Get a single eigenvector.
        std::vector<T> const& GetEigenvector(size_t i) const
        {
            GTL_OUTOFRANGE_ASSERT(
                i < mSize,
                "The index must not exceed the matrix size.");

            return mEigenvectors[i];
        }

        // Get all the eigenvectors.
        inline std::vector<std::vector<T>> const& GetEigenvectors() const
        {
            return mEigenvectors;
        }

        // Support for move semantics to avoid copying costs.
        inline std::vector<T>& GetEigenvalues()
        {
            return mEigenvalues;
        }

        inline std::vector<std::vector<T>>& GetEigenvectors()
        {
            return mEigenvectors;
        }

    private:
        // Tridiagonalize using Householder reflections. On input, matrix is
        // a copy of the input to operator()(...). On output, the upper
        // triangular part of matrix including the diagonal stores the
        // tridiagonalization. The lower-triangular part contains 2/Dot(v,v)
        // that are used in computing eigenvectors and the part below the
        // subdiagonal stores the essential parts of the Householder vectors
        // v (the elements of v after the leading 1-valued component).
        void Tridiagonalize()
        {
            size_t r{}, c{};
            for (size_t i = 0, ip1 = 1; i < mSize - 2; ++i, ++ip1)
            {
                // Compute the Householder vector. Read the initial vector
                // from the row of the matrix.
                for (r = 0; r < ip1; ++r)
                {
                    mVVector[r] = C_<T>(0);
                }
                T length = C_<T>(0);
                for (r = ip1; r < mSize; ++r)
                {
                    T vr = mMatrix[r + mSize * i];
                    mVVector[r] = vr;
                    length += vr * vr;
                }
                T vdv = C_<T>(1);
                length = std::sqrt(length);
                if (length > C_<T>(0))
                {
                    T& v1 = mVVector[ip1];
                    T sgn = (v1 >= C_<T>(0) ? C_<T>(1) : -C_<T>(1));
                    T invDenom = C_<T>(1) / (v1 + sgn * length);
                    v1 = C_<T>(1);
                    for (r = ip1 + 1; r < mSize; ++r)
                    {
                        T& vr = mVVector[r];
                        vr *= invDenom;
                        vdv += vr * vr;
                    }
                }

                // Compute the rank-1 offsets v*w^T and w*v^T.
                T invvdv = C_<T>(1) / vdv;
                T twoinvvdv = invvdv * C_<T>(2);
                T pdvtvdv = C_<T>(0);
                for (r = i; r < mSize; ++r)
                {
                    mPVector[r] = C_<T>(0);
                    for (c = i; c < r; ++c)
                    {
                        mPVector[r] +=  mMatrix[r + mSize * c] * mVVector[c];
                    }
                    for (/**/; c < mSize; ++c)
                    {
                        mPVector[r] +=  mMatrix[c + mSize * r] * mVVector[c];
                    }
                    mPVector[r] *= twoinvvdv;
                    pdvtvdv += mPVector[r] * mVVector[r];
                }

                pdvtvdv *= invvdv;
                for (r = i; r < mSize; ++r)
                {
                    mWVector[r] = mPVector[r] - pdvtvdv * mVVector[r];
                }

                // Update the input matrix.
                for (r = i; r < mSize; ++r)
                {
                    T vr = mVVector[r];
                    T wr = mWVector[r];
                    T offset = vr * wr * C_<T>(2);
                    mMatrix[r + mSize * r] -= offset;
                    for (c = r + 1; c < mSize; ++c)
                    {
                        offset = vr * mWVector[c] + wr * mVVector[c];
                        mMatrix[c + mSize * r] -= offset;
                    }
                }

                // Copy the vector to column i of the matrix. The 0-valued
                // components at indices 0 through i are not stored. The
                // 1-valued component at index i+1 is also not stored;
                // instead, the quantity 2/Dot(v,v) is stored for use in
                // eigenvector construction. That construction must take
                // into account the implied components that are not stored.
                mMatrix[i + mSize * ip1] = twoinvvdv;
                for (r = ip1 + 1; r < mSize; ++r)
                {
                    mMatrix[i + mSize * r] = mVVector[r];
                }
            }

            // Copy the diagonal and subdiagonal entries for cache coherence
            // in the QR iterations.
            size_t k{}, ksup = mSize - 1, index = 0, delta = mSize + 1;
            for (k = 0; k < ksup; ++k, index += delta)
            {
                mDiagonal[k] = mMatrix[index];
                mSuperdiagonal[k] = mMatrix[index + 1];
            }
            mDiagonal[k] = mMatrix[index];
        }

        // A helper for generating Givens rotation sine and cosine robustly.
        void GetSinCos(T x, T y, T& cs, T& sn)
        {
            // Solves sn*x + cs*y = 0 robustly.
            if (y != C_<T>(0))
            {
                if (std::fabs(y) > std::fabs(x))
                {
                    T tau = -x / y;
                    sn = C_<T>(1) / std::sqrt(C_<T>(1) + tau * tau);
                    cs = sn * tau;
                }
                else
                {
                    T tau = -y / x;
                    cs = C_<T>(1) / std::sqrt(C_<T>(1) + tau * tau);
                    sn = cs * tau;
                }
            }
            else
            {
                cs = C_<T>(1);
                sn = C_<T>(0);
            }
        }

        // The QR step with implicit shift. Generally, the initial T is
        // unreduced tridiagonal (all subdiagonal entries are nonzero). If a
        // QR step causes a superdiagonal entry to become zero, the matrix
        // decouples into a block diagonal matrix with two tridiagonal blocks.
        // These blocks can be reduced independently of each other, which
        // allows for parallelization of the algorithm. The inputs imin and
        // imax identify the subblock of T to be processed. That block has
        // upper-left element T(imin,imin) and lower-right element
        // T(imax,imax).
        void DoQRImplicitShift(size_t imin, size_t imax)
        {
            // The implicit shift. Compute the eigenvalue u of the
            // lower-right 2x2 block that is closer to a11.
            T a00 = mDiagonal[imax];
            T a01 = mSuperdiagonal[imax];
            T a11 = mDiagonal[imax + 1];
            T dif = (a00 - a11) * C_<T>(1, 2);
            T sgn = (dif >= C_<T>(0) ? C_<T>(1) : -C_<T>(1));
            T a01sqr = a01 * a01;
            T u = a11 - a01sqr / (dif + sgn * std::sqrt(dif * dif + a01sqr));
            T x = mDiagonal[imin] - u;
            T y = mSuperdiagonal[imin];

            T a12{}, a22{}, a23{}, tmp11{}, tmp12{}, tmp21{}, tmp22{}, cs{}, sn{};
            T a02 = C_<T>(0);
            size_t i0 = imin - 1, i1 = imin, i2 = imin + 1;
            for (; i1 <= imax; ++i0, ++i1, ++i2)
            {
                // Compute the Givens rotation and save it for use in
                // computing the eigenvectors.
                GetSinCos(x, y, cs, sn);
                mGivens.push_back(GivensRotation(i1, cs, sn));

                // Update the tridiagonal matrix. This amounts to updating a
                // 4x4 subblock,
                //   b00 b01 b02 b03
                //   b01 b11 b12 b13
                //   b02 b12 b22 b23
                //   b03 b13 b23 b33
                // The four corners (b00, b03, b33) do not change values. The
                // The interior block {{b11,b12},{b12,b22}} is updated on each
                // pass. For the first pass, the b0c values are out of range,
                // so only the values (b13, b23) change. For the last pass,
                // the br3 values are out of range, so only the values
                // (b01, b02) change. For passes between first and last, the
                // values (b01, b02, b13, b23) change.
                if (i1 > imin)
                {
                    mSuperdiagonal[i0] = cs * mSuperdiagonal[i0] - sn * a02;
                }

                a11 = mDiagonal[i1];
                a12 = mSuperdiagonal[i1];
                a22 = mDiagonal[i2];
                tmp11 = cs * a11 - sn * a12;
                tmp12 = cs * a12 - sn * a22;
                tmp21 = sn * a11 + cs * a12;
                tmp22 = sn * a12 + cs * a22;
                mDiagonal[i1] = cs * tmp11 - sn * tmp12;
                mSuperdiagonal[i1] = sn * tmp11 + cs * tmp12;
                mDiagonal[i2] = sn * tmp21 + cs * tmp22;

                if (i1 < imax)
                {
                    a23 = mSuperdiagonal[i2];
                    a02 = -sn * a23;
                    mSuperdiagonal[i2] = cs * a23;

                    // Update the parameters for the next Givens rotation.
                    x = mSuperdiagonal[i1];
                    y = a02;
                }
            }
        }

        // Sort the eigenvalues and compute the corresponding permutation of
        // the indices of the array storing the eigenvalues.
        void RepackageEigenstuff()
        {
            // Compute the permutation induced by sorting. Initially, we
            // start with the identity permutation I = (0,1,...,N-1).
            struct SortItem
            {
                SortItem()
                    :
                    eigenvalue(C_<T>(0)),
                    index(0)
                {
                }

                T eigenvalue;
                size_t index;
            };

            std::vector<SortItem> items(mSize);
            size_t i{};
            for (i = 0; i < mSize; ++i)
            {
                items[i].eigenvalue = mDiagonal[i];
                items[i].index = i;
            }

            std::sort(items.begin(), items.end(),
                [](SortItem const& item0, SortItem const& item1)
                {
                    return item0.eigenvalue < item1.eigenvalue;
                }
            );

            std::vector<size_t> permutation(mSize);
            i = 0;
            for (auto const& item : items)
            {
                permutation[i++] = item.index;
            }

            for (i = 0; i < mSize; ++i)
            {
                mEigenvalues[i] = mDiagonal[permutation[i]];
            }

            ComputeEigenvectors(permutation);
        }

        void ComputeEigenvectors(std::vector<size_t> const& permutation)
        {
            // The number of Householder reflections is H = mSize - 2. If
            // H is even, the product of reflections is a rotation; otherwise,
            // H is odd and the product is a reflection. The number of Givens
            // rotations does not affect the type of the product of
            // reflections.
            bool isRotation = ((mSize & 1) == 0);

            // Start with the identity matrix.
            for (size_t i = 0; i < mSize; ++i)
            {
                std::fill(mEigenvectors[i].begin(), mEigenvectors[i].end(), C_<T>(0));
                mEigenvectors[i][i] = C_<T>(1);
            }

            // Multiply the Householder reflections using backward
            // accumulation.
            size_t r{}, c{};
            for (size_t k = 3, i = mSize - 3, rmin = i + 1; k <= mSize; ++k, --i, --rmin)
            {
                // Copy the v vector and 2/Dot(v,v) from the matrix.
                T const* column = &mMatrix[i];
                T twoinvvdv = column[mSize * (i + 1)];
                for (r = 0; r < i + 1; ++r)
                {
                    mVVector[r] = C_<T>(0);
                }
                mVVector[r] = C_<T>(1);
                for (++r; r < mSize; ++r)
                {
                    mVVector[r] = column[mSize * r];
                }

                // Compute the w vector.
                for (r = 0; r < mSize; ++r)
                {
                    mWVector[r] = C_<T>(0);
                    for (c = rmin; c < mSize; ++c)
                    {
                        mWVector[r] += mVVector[c] * mEigenvectors[r][c];
                    }
                    mWVector[r] *= twoinvvdv;
                }

                // Update the matrix, Q <- Q - v*w^T.
                for (r = rmin; r < mSize; ++r)
                {
                    for (c = 0; c < mSize; ++c)
                    {
                        mEigenvectors[c][r] -= mVVector[r] * mWVector[c];
                    }
                }
            }

            // Multiply the Givens rotations.
            for (auto const& givens : mGivens)
            {
                for (r = 0; r < mSize; ++r)
                {
                    T& q0 = mEigenvectors[givens.index][r];
                    T& q1 = mEigenvectors[givens.index + 1][r];
                    T prd0 = givens.cs * q0 - givens.sn * q1;
                    T prd1 = givens.sn * q0 + givens.cs * q1;
                    q0 = prd0;
                    q1 = prd1;
                }
            }

            std::vector<size_t> visited(mSize, 0);
            for (size_t i = 0; i < mSize; ++i)
            {
                if (visited[i] == 0 && permutation[i] != i)
                {
                    // The item starts a cycle with 2 or more elements.
                    for (size_t j = 0; j < mSize; ++j)
                    {
                        mPVector[j] = mEigenvectors[i][j];
                    }

                    size_t start = i, current = i, next{};
                    while ((next = permutation[current]) != start)
                    {
                        isRotation = !isRotation;
                        visited[current] = 1;
                        for (size_t j = 0; j < mSize; ++j)
                        {
                            mEigenvectors[current][j] = mEigenvectors[next][j];
                        }
                        current = next;
                    }
                    visited[current] = 1;

                    for (size_t j = 0; j < mSize; ++j)
                    {
                        mEigenvectors[current][j] = mPVector[j];
                    }
                }
            }

            if (!isRotation)
            {
                // The eigenvectors are the columns of a reflection matrix.
                // Change sign on the last column to convert to a rotation
                // matrix.
                r = mSize - 1;
                for (c = 0; c < mSize; ++c)
                {
                    mEigenvectors[r][c] = -mEigenvectors[r][c];
                }
            }
        }

        // The number N of rows and columns of the matrices to be processed.
        size_t mSize;

        // The internal copy of a matrix passed to the solver. See the
        // comments about function Tridiagonalize() about what is stored in
        // the matrix.
        std::vector<T> mMatrix;  // NxN elements

        // After the initial tridiagonalization by Householder reflections, we
        // no longer need the full mMatrix. Copy the diagonal and
        // superdiagonal entries to linear arrays in order to be cache
        // friendly.
        std::vector<T> mDiagonal;  // N elements
        std::vector<T> mSuperdiagonal;  // N-1 elements

        // Storage for eigenvalues and eigenvectors for concise member access.
        std::vector<T> mEigenvalues;
        std::vector<std::vector<T>> mEigenvectors;

        // The Givens rotations used to reduce the initial tridiagonal matrix
        // to a diagonal matrix. A rotation is the identity with the following
        // replacement entries: R(index,index) = cs, R(index,index+1) = sn,
        // R(index+1,index) = -sn and R(index+1,index+1) = cs. If N is the
        // matrix size and K is the maximum number of iterations, the maximum
        // number of Givens rotations is K*(N-1). The maximum amount of
        // memory is allocated to store these.
        struct GivensRotation
        {
            GivensRotation()
                :
                index(0),
                cs(C_<T>(0)),
                sn(C_<T>(0))
            {
            }

            GivensRotation(size_t inIndex, T inCs, T inSn)
                :
                index(inIndex),
                cs(inCs),
                sn(inSn)
            {
            }

            size_t index;
            T cs, sn;
        };

        std::vector<GivensRotation> mGivens;  // K*(N-1) elements

        // Temporary storage to compute Householder reflections and to
        // support sorting of eigenvectors.
        mutable std::vector<T> mVVector;  // N elements
        mutable std::vector<T> mPVector;  // N elements
        mutable std::vector<T> mWVector;  // N elements

        friend class UnitTestSymmetricEigensolver;
    };
}
