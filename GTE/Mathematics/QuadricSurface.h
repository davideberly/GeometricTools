// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// A quadric surface is defined implicitly by
//
//   0 = q0 + q1 * x[0] + q2 * x[1] + q3 * x[2] +
//       q4 * x[0]^2 + q5 * x[0] * x[1] + q6 * x[0] * x[2] +
//       q7 * x[1]^2 + q8 * x[1] * x[2] + q9 * x[2]^2
//
//                                   +-              -+
//                                   | q4   q5/2 q6/2 |
//     = q0 + [q1 q2 q3] * X + X^T * | q5/2 q7   q8/2 | * X
//                                   | q6/2 q8/2 q9   |
//                                   +-              -+
//
//     = C + B^T*X + X^T*A*X
// 
// A document describing the classification of the solution set is
// https://www.geometrictools.com/Documentation/ClassifyingQuadrics.pdf

#include <Mathematics/Logger.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Matrix2x2.h>
#include <Mathematics/Matrix3x3.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class QuadricSurface
    {
    public:
        QuadricSurface()
            :
            mClassification(Classification::UNKNOWN),
            mA{},
            mB{},
            mC(static_cast<T>(0))
        {
        }

        QuadricSurface(Matrix3x3<T> const& A, Vector3<T> const& b, T const& c)
            :
            mClassification(Classification::UNKNOWN),
            mA(A),
            mB(b),
            mC(c)
        {
        }

        QuadricSurface(std::array<T, 10> const& q)
            :
            mClassification(Classification::UNKNOWN),
            mA{
                q[4], static_cast<T>(0.5) * q[5], static_cast<T>(0.5) * q[6],
                static_cast<T>(0.5) * q[5], q[7], static_cast<T>(0.5) * q[8],
                static_cast<T>(0.5) * q[6], static_cast<T>(0.5) * q[8], q[9] },
                mB{ q[1], q[2], q[3] },
                mC(q[0])
        {
        }

        // Member access.
        inline Matrix3x3<T> const& GetA() const
        {
            return mA;
        }

        inline Vector3<T> const& GetB() const
        {
            return mB;
        }

        inline T const& GetC() const
        {
            return mC;
        }

        inline std::array<T, 10> GetQ() const
        {
            std::array<T, 10> q{};
            q[0] = mC;
            q[1] = mB[0];
            q[2] = mB[1];
            q[3] = mB[2];
            q[4] = mA(0, 0);
            q[5] = static_cast<T>(2) * mA(0, 1);
            q[6] = static_cast<T>(2) * mA(0, 2);
            q[7] = mA(1, 1);
            q[8] = static_cast<T>(2) * mA(1, 2);
            q[9] = mA(2, 2);
            return q;
        }

        // Evaluate the function.
        T F(Vector3<T> const& position) const
        {
            T f = Dot(position, mA * position + mB) + mC;
            return f;
        }

        // Evaluate the first-order partial derivatives (gradient).
        T FX(Vector3<T> const& position) const
        {
            T sum = mA(0, 0) * position[0] + mA(0, 1) * position[1] + mA(0, 2) * position[2];
            T fx = static_cast<T>(2) * sum + mB[0];
            return fx;
        }

        T FY(Vector3<T> const& position) const
        {
            T sum = mA(1, 0) * position[0] + mA(1, 1) * position[1] + mA(1, 2) * position[2];
            T fy = static_cast<T>(2) * sum + mB[1];
            return fy;
        }

        T FZ(Vector3<T> const& position) const
        {
            T sum = mA(2, 0) * position[0] + mA(2, 1) * position[1] + mA(2, 2) * position[2];
            T fz = static_cast<T>(2) * sum + mB[2];
            return fz;
        }

        // Evaluate the second-order partial derivatives (Hessian).
        T FXX(Vector3<T> const&) const
        {
            T fxx = static_cast<T>(2) * mA(0, 0);
            return fxx;
        }

        T FXY(Vector3<T> const&) const
        {
            T fxy = static_cast<T>(2) * mA(0, 1);
            return fxy;
        }

        T FXZ(Vector3<T> const&) const
        {
            T fxz = static_cast<T>(2) * mA(0, 2);
            return fxz;
        }

        T FYY(Vector3<T> const&) const
        {
            T fyy = static_cast<T>(2) * mA(1, 1);
            return fyy;
        }

        T FYZ(Vector3<T> const&) const
        {
            T fyz = static_cast<T>(2) * mA(1, 2);
            return fyz;
        }

        T FZZ(Vector3<T> const&) const
        {
            T fzz = static_cast<T>(2) * mA(2, 2);
            return fzz;
        }

        // Classification of the quadric. The implementation uses exact
        // rational arithmetic to avoid misclassification due to
        // floating-point rounding errors.
        enum class Classification
        {
            NO_SOLUTION,
            POINT,
            LINE,
            PLANE,
            TWO_PLANES,
            PARABOLIC_CYLINDER,
            ELLIPTIC_CYLINDER,
            HYPERBOLIC_CYLINDER,
            ELLIPTIC_PARABOLOID,
            HYPERBOLIC_PARABOLOID,
            ELLIPTIC_CONE,
            HYPERBOLOID_ONE_SHEET,
            HYPERBOLOID_TWO_SHEETS,
            ELLIPSOID,
            ENTIRE_SPACE,
            UNKNOWN
        };

        Classification GetClassification() const
        {
            if (mClassification != Classification::UNKNOWN)
            {
                return mClassification;
            }

            // Convert the coefficients to their rational representations and
            // compute various derived quantities.
            Matrix3x3<Rational> rA{};
            Vector3<Rational> rB{};
            Rational rC{};
            rA(0, 0) = mA(0, 0);
            rA(0, 1) = mA(0, 1);
            rA(0, 2) = mA(0, 2);
            rA(1, 0) = rA(0, 1);
            rA(1, 1) = mA(1, 1);
            rA(1, 2) = mA(1, 2);
            rA(2, 0) = rA(0, 2);
            rA(2, 1) = rA(1, 2);
            rA(2, 2) = mA(2, 2);
            rB[0] = mB[0];
            rB[1] = mB[1];
            rB[2] = mB[2];
            rC = mC;

            // Compute the polynomial det(lambda * I - A) with rational
            // coefficients.
            Rational rS00 = rA(1, 1) * rA(2, 2) - rA(1, 2) * rA(1, 2);
            Rational rS01 = rA(0, 1) * rA(2, 2) - rA(1, 2) * rA(0, 2);
            Rational rS02 = rA(0, 1) * rA(1, 2) - rA(0, 2) * rA(1, 1);
            Rational rS11 = rA(0, 0) * rA(2, 2) - rA(0, 2) * rA(0, 2);
            Rational rS12 = rA(0, 0) * rA(1, 2) - rA(0, 2) * rA(0, 1);
            Rational rS22 = rA(0, 0) * rA(1, 1) - rA(0, 1) * rA(0, 1);
            std::array<Rational, 4> rP
            {
                -(rA(0, 0) * rS00 - rA(0, 1) * rS01 + rA(0, 2) * rS02),
                rS00 + rS11 + rS22,
                -(rA(0, 0) + rA(1, 1) + rA(2, 2)),
                Rational(1)
            };

            // Use Sturm sequences to determine the signs of the roots.
            size_t numPositive = 0, numNegative = 0, numZero = 0;
            ComputeRootSigns(rP, numPositive, numNegative, numZero);

            // Classify the solution set to the equation.
            if (numZero == 0)
            {
                mClassification = AllNonzero(rA, rB, rC, numPositive);
            }
            else if (numZero == 1)
            {
                mClassification = TwoNonzero(rA, rB, rC, numPositive, numNegative);
            }
            else if (numZero == 2)
            {
                mClassification = OneNonzero(rA, rB, rC, numPositive);
            }
            else  // numZero = 3
            {
                mClassification = AllZero(rB, rC);
            }
            return mClassification;
        }

    private:
        using Rational = BSRational<UIntegerAP32>;

        // Use Descartes' rule of signs to determine the root signs.
        static void ComputeRootSigns(std::array<Rational, 4> const& rP,
            size_t& numPositive, size_t& numNegative, size_t& numZero)
        {
            // Collect the nonzero signs of the rP[0], rP[1] and rP[2].
            int32_t constexpr degree = 3;
            std::array<int32_t, degree + 1> signs{};
            for (size_t i = 0; i <= degree; ++i)
            {
                signs[i] = rP[i].GetSign();
            }

            // Compute the number of positive roots of p(lambda).
            int32_t currentSign = signs[degree];
            numPositive = 0;
            for (int32_t i = degree - 1; i >= 0; --i)
            {
                if (signs[i] == -currentSign)
                {
                    currentSign = signs[i];
                    ++numPositive;
                }
            }

            // Compute the signs of the coefficients of p(-lambda).
            for (int32_t i = 1; i <= degree; i += 2)
            {
                signs[i] = -signs[i];
            }

            // Compute the number of positive roots of p(-lambda).
            currentSign = signs[degree];
            numNegative = 0;
            for (int32_t i = degree - 1; i >= 0; --i)
            {
                if (signs[i] == -currentSign)
                {
                    currentSign = signs[i];
                    ++numNegative;
                }
            }

            // Compute the number of zero roots of p(lambda).
            numZero = 3 - numPositive - numNegative;
        }

        static Classification AllNonzero(Matrix3x3<Rational> const& A,
            Vector3<Rational> const& b, Rational const& c, size_t numPositiveRoots)
        {
            Rational r = Dot(b, Inverse(A) * b) / Rational(4) - c;

            if (r > Rational(0))
            {
                if (numPositiveRoots == 3)
                {
                    return Classification::ELLIPSOID;
                }
                else if (numPositiveRoots == 2)
                {
                    return Classification::HYPERBOLOID_ONE_SHEET;
                }
                else if (numPositiveRoots == 1)
                {
                    return Classification::HYPERBOLOID_TWO_SHEETS;
                }
                else
                {
                    return Classification::NO_SOLUTION;
                }
            }
            else if (r < Rational(0))
            {
                if (numPositiveRoots == 3)
                {
                    return Classification::NO_SOLUTION;
                }
                else if (numPositiveRoots == 2)
                {
                    return Classification::HYPERBOLOID_TWO_SHEETS;
                }
                else if (numPositiveRoots == 1)
                {
                    return Classification::HYPERBOLOID_ONE_SHEET;
                }
                else
                {
                    return Classification::ELLIPSOID;
                }
            }
            else // sign == 0
            {
                if (numPositiveRoots == 3 || numPositiveRoots == 0)
                {
                    return Classification::POINT;
                }
                else
                {
                    return Classification::ELLIPTIC_CONE;
                }
            }
        }

        static void ComputeOrthogonalSetTwoNonzero(Matrix3x3<Rational> const& A,
            Vector3<Rational>& w0, Vector3<Rational>& w1, Vector3<Rational>& w2)
        {
            Vector3<Rational> vzero{}; // zero vector
            w1 = { A(0, 0), A(0, 1), A(0, 2) };
            if (w1 != vzero)
            {
                w2 = { A(1, 0), A(1, 1), A(1, 2) };
                w0 = Cross(w1, w2);
                if (w0 == vzero)
                {
                    w2 = { A(2, 0), A(2, 1), A(2, 2) };
                    w0 = Cross(w1, w2);
                }
            }
            else
            {
                w1 = { A(1, 0), A(1, 1), A(1, 2) };
                w2 = { A(2, 0), A(2, 1), A(2, 2) };
                w0 = Cross(w1, w2);
            }
            w2 = Cross(w0, w1);
        }

        static Classification TwoNonzero(Matrix3x3<Rational> const& A,
            Vector3<Rational> const& b, Rational const& c, size_t numPositive, size_t numNegative)
        {
            Vector3<Rational> w0{}, w1{}, w2{};
            ComputeOrthogonalSetTwoNonzero(A, w0, w1, w2);
            Rational d0 = Dot(w0, b);
            if (d0 != Rational(0))
            {
                if (numPositive == numNegative)
                {
                    return Classification::HYPERBOLIC_PARABOLOID;
                }
                else
                {
                    return Classification::ELLIPTIC_PARABOLOID;
                }
            }

            Vector3<Rational> Aw1 = A * w1;
            Vector3<Rational> Aw2 = A * w2;
            Matrix2x2<Rational> E{};
            E(0, 0) = Dot(w1, Aw1);
            E(0, 1) = Dot(w1, Aw2);
            E(1, 0) = E(0, 1);
            E(1, 1) = Dot(w2, Aw2);
            Vector2<Rational> f{ Dot(w1, b) , Dot(w2, b) };
            Rational r = Dot(f, Inverse(E) * f) / Rational(4) - c;

            if (numPositive == 2)
            {
                if (r > Rational(0))
                {
                    return Classification::ELLIPTIC_CYLINDER;
                }
                else if (r < Rational(0))
                {
                    return Classification::NO_SOLUTION;
                }
                else
                {
                    return Classification::LINE;
                }
            }
            else if (numNegative == 2)
            {
                if (r < Rational(0))
                {
                    return Classification::ELLIPTIC_CYLINDER;
                }
                else if (r > Rational(0))
                {
                    return Classification::NO_SOLUTION;
                }
                else
                {
                    return Classification::LINE;
                }
            }
            else  // numPositive = numNegative = 1
            {
                if (r != Rational(0))
                {
                    return Classification::HYPERBOLIC_CYLINDER;
                }
                else
                {
                    return Classification::TWO_PLANES;
                }
            }
        }

        static void ComputeOrthogonalSetOneNonzero(Matrix3x3<Rational> const& A,
            Vector3<Rational>& w0, Vector3<Rational>& w1, Vector3<Rational>& w2)
        {
            Vector3<Rational> vzero{}; // zero vector
            w2 = { A(0, 0), A(0, 1), A(0, 2) };
            if (w2 == vzero)
            {
                w2 = { A(1, 0), A(1, 1), A(1, 2) };
                if (w2 == vzero)
                {
                    w2 = { A(2, 0), A(2, 1), A(2, 2) };
                }
            }

            if (std::fabs(w2[0]) > std::fabs(w2[1]))
            {
                w0 = { -w2[2], Rational(0), w2[0] };
            }
            else
            {
                w0 = { Rational(0), w2[2], -w2[1] };
            }
            w1 = Cross(w2, w0);
        }

        static Classification OneNonzero(Matrix3x3<Rational> const& A,
            Vector3<Rational> const& b, Rational const& c, size_t numPositive)
        {
            Vector3<Rational> w0{}, w1{}, w2{};
            ComputeOrthogonalSetOneNonzero(A, w0, w1, w2);
            Rational d0 = Dot(w0, b);
            Rational d1 = Dot(w1, b);
            if (d0 != Rational(0) || d1 != Rational(0))
            {
                return Classification::PARABOLIC_CYLINDER;
            }

            Rational E = Dot(w2, A * w2);
            Rational f = Dot(w2, b);
            Rational r = f * f / (Rational(4) * E) - c;

            if (numPositive == 1)  // numNegative = 0
            {
                if (r > Rational(0))
                {
                    return Classification::TWO_PLANES;
                }
                else if (r < Rational(0))
                {
                    return Classification::NO_SOLUTION;
                }
                else
                {
                    return Classification::PLANE;
                }
            }
            else  // numPositive = 0, numNegative = 1
            {
                if (r < Rational(0))
                {
                    return Classification::TWO_PLANES;
                }
                else if (r > Rational(0))
                {
                    return Classification::NO_SOLUTION;
                }
                else
                {
                    return Classification::PLANE;
                }
            }
        }

        static Classification AllZero(Vector3<Rational> const& b, Rational const& c)
        {
            Vector3<Rational> vzero{}; // zero vector
            if (b != vzero)
            {
                return Classification::PLANE;
            }
            else
            {
                if (c == Rational(0))
                {
                    return Classification::ENTIRE_SPACE;
                }
                else
                {
                    return Classification::NO_SOLUTION;
                }
            }
        }

        // Floating-point quadratic coefficients.
        mutable Classification mClassification;
        Matrix3x3<T> mA;
        Vector3<T> mB;
        T mC;
    };
}
