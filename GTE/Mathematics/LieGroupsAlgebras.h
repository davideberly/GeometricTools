// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.1.2024.05.21

#pragma once

// Lie groups and Lie algebras are useful for representing special classes of
// matrices found in applications. Implementations are provided for rotations
// in 2D and 3D and for rigid motions (rotations and translations) in 2D and
// 3D. The mathematical details are found in
//   https://www.geometrictools.com/Documentation/LieGroupsAlgebras.pdf

#include <Mathematics/Matrix2x2.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/Matrix4x4.h>
#include <Mathematics/RotationEstimate.h>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    // S0(2) is the Lie group for rotations in 2D. so(2) is the corresponding
    // Lie algebra for SO(2) and is a 1D quantity x = (angle). The 2x2
    // rotation matrix M is generated from x by constructing a 2x2 generator
    // G = x*G0, where
    //   G0 = {{ 0, -1 },{ 1, 0 }}
    // and then computing the power series M = exp(L(x)). For the sake of
    // notation, exp(x) is used to denote exp(L(x)). The 2x2 rotation matrix
    // is
    //   M = {{ cos(x), -sin(x) }, { sin(x), cos(x) }}
    // The adjoint matrix is the 1x1 identity matrix
    //   A(M) = 1

    template <typename T>
    class LieSO2
    {
    public:
        // n = 2, k = 1, x = (angle)
        using AlgebraType = T;          // kx1
        using AdjointType = T;          // kxk
        using GroupType = Matrix2x2<T>; // nxn

        // Compute the Lie group element X from the Lie algebra element x
        // using X = L(x).
        static GroupType ToGroup(AlgebraType const& x)
        {
            T const zero = static_cast<T>(0);
            GroupType X{};
            X(0, 0) = zero;
            X(0, 1) = -x;
            X(1, 0) = x;
            X(1, 1) = zero;
            return X;
        }

        // Compute the Lie algebra element x from the Lie group element X
        // using x = L^{-1}(X).
        static AlgebraType ToAlgebra(GroupType const& X)
        {
            return X(1, 0);
        }
    
        // Compute the exponential map of the Lie algebra element x to produce
        // the Lie group element Y = exp(X) = exp(L(x)).
        static GroupType Exp(AlgebraType const& x)
        {
            GroupType Y{};
            T sn = std::sin(x);
            T cs = std::cos(x);
            Y(0, 0) = cs;
            Y(0, 1) = -sn;
            Y(1, 0) = sn;
            Y(1, 1) = cs;
            return Y;
        }

        // Compute the logarithm map of the Lie group element Y to produce the
        // Lie algebra element x corresponding to the Lie group element X.
        static AlgebraType Log(GroupType const& Y)
        {
            AlgebraType x = std::atan2(Y(1, 0), Y(0, 0));
            return x;
        }

        // Compute the adjoint matrix A(M) from the Lie group element M.
        static AdjointType Adjoint(GroupType const&)
        {
            AdjointType A = static_cast<T>(1);
            return A;
        }

        // Compute log(M1*Inverse(M0)) to reduce computation time when you
        // want to evaluate GeodesicPath for multiple values of t. For a
        // rotation matrix M0, Inverse(M0) is equal to Transpose(M0), which
        // avoids a general inversion of M0.
        static AlgebraType LogM1M0Inv(GroupType const& M0, GroupType const& M1)
        {
            return Log(MultiplyABT(M1, M0));
        }

        // Compute a point on the geodesic path from M0 to M1. The expression
        // log(M1*Inverse(M0)) is computed for each call to the function. Use
        // this GeodesicPath when it is needed for only a single value of t.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, GroupType const& M1)
        {
            return Exp(t * LogM1M0Inv(M0, M1)) * M0;
        }

        // Compute a point on the geodesic path from M0 to M1. The Lie algebra
        // element log(M1*Inverse(M0)) must be precomputed by the caller. Use
        // this GeodesicPath when it is needed for multiple values of t.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, AlgebraType const& logM1M0Inv)
        {
            return Exp(t * logM1M0Inv) * M0;
        }
    };


    // SE(2) is the Lie group for rigid motions in 2D. se(2) is the
    // corresponding Lie algebra for SE(2) and is a 3D quantity
    // x = (angle; u0, u1), where (angle) is for the rotation matrix and
    // (u0, u1) is for the translation vector. The 3x3 rigid motion M is
    // generated from x by constructing a 3x3 generator
    // G = x0 * G0 + x1 * G1 + x2 * G2, where
    //   G0 = {{ 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 0 }}
    //   G1 = {{ 0, 0, 1 }, { 0, 0, 0 }, { 0, 0, 0 }}
    //   G2 = {{ 0, 0, 0 }, { 0, 0, 1 }, { 0, 0, 0 }}
    // and then computing the power series M = exp(L(x)). For the sake of
    // notation, exp(x) is used to denote exp(L(x)). The rigid motion
    // matrix is
    //   M = {{ R, T }, { 0, 1 }}
    // where R is the 2x2 rotation matrix, T is the 2x1 translation vector,
    // 0 is the 1x2 zero vector and 1 is a scalar. The adjoint matrix is
    //   A(M) = {{ 1, 0 }, { Perp(T), R }}
    // where T = (t0,t1) and Perp(T) = (t1,-t0), both 2x1 vectors but written
    // as 2-tuples.

    template <typename T>
    class LieSE2
    {
    public:
        // n = 3, k = 3, x = (angle; u0, u1)
        using AlgebraType = Vector3<T>;     // kx1
        using AdjointType = Matrix3x3<T>;   // kxk
        using GroupType = Matrix3x3<T>;     // nxn

        // Compute the Lie group element X from the Lie algebra element x
        // using X = L(x).
        static GroupType ToGroup(AlgebraType const& x)
        {
            T const zero = static_cast<T>(0);
            GroupType X{};
            X(0, 0) = zero;
            X(0, 1) = -x[0];
            X(0, 2) = x[1];
            X(1, 0) = x[0];
            X(1, 1) = zero;
            X(1, 2) = x[2];
            X(2, 0) = zero;
            X(2, 1) = zero;
            X(2, 2) = zero;
            return X;
        }

        // Compute the Lie algebra element x from the Lie group element X
        // using x = L^{-1}(X).
        static AlgebraType ToAlgebra(GroupType const& X)
        {
            AlgebraType A{ X(1, 0), X(0, 2), X(1, 2) };
            return A;
        }

        // Compute the exponential map of the Lie algebra element x to produce
        // the Lie group element Y = exp(X) = exp(L(x)).
        static GroupType Exp(AlgebraType const& x)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T sn = std::sin(x[0]);
            T cs = std::cos(x[0]);

            // Compute sin(t)/t.
            T a0 = F0(x[0]);

            // Compute (1 - cos(t))/t = t * (1-cos(t))/t^2.
            T a1 = x[0] * F1(x[0]);

            T trn0 = a0 * x[1] - a1 * x[2];
            T trn1 = a1 * x[1] + a0 * x[2];

            GroupType Y{};
            Y(0, 0) = cs;
            Y(0, 1) = -sn;
            Y(0, 2) = trn0;
            Y(1, 0) = sn;
            Y(1, 1) = cs;
            Y(1, 2) = trn1;
            Y(2, 0) = zero;
            Y(2, 1) = zero;
            Y(2, 2) = one;
            return Y;
        }

        // Compute the logarithm map of the Lie group element Y to produce the
        // Lie algebra element x corresponding to the Lie group element X.
        static AlgebraType Log(GroupType const& Y)
        {
            AlgebraType x{};
            x[0] = std::atan2(Y(1, 0), Y(0, 0));

            // Compute sin(t)/t.
            T a0 = F0(x[0]);

            // Compute (1 - cos(t))/t = t * (1-cos(t))/t^2.
            T a1 = x[0] * F1(x[0]);

            Matrix2x2<T> V{};
            V(0, 0) = a0;
            V(1, 0) = a1;
            V(0, 1) = -a1;
            V(1, 1) = a0;

            Matrix2x2<T> inverseV = Inverse(V);
            x[1] = inverseV(0, 0) * Y(0, 2) + inverseV(0, 1) * Y(1, 2);
            x[2] = inverseV(1, 0) * Y(0, 2) + inverseV(1, 1) * Y(1, 2);
            return x;
        }

        // Compute the adjoint matrix A(M) from the Lie group element M.
        static AdjointType Adjoint(GroupType const& M)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            AdjointType A{};
            A(0, 0) = one;
            A(0, 1) = zero;
            A(0, 2) = zero;
            A(1, 0) = M(1, 2);
            A(1, 1) = M(0, 0);
            A(1, 2) = M(0, 1);
            A(2, 0) = -M(0, 2);
            A(2, 1) = M(1, 0);
            A(2, 2) = M(1, 1);
            return A;
        }

        // Compute log(M1*Inverse(M0)) to reduce computation time when you
        // want to evaluate GeodesicPath for multiple values of t.
        static AlgebraType LogM1M0Inv(GroupType const& M0, GroupType const& M1)
        {
            return Log(M1 * Inverse(M0));
        }

        // Compute a point on the geodesic path from M0 to M1. The expression
        // log(M1*Inverse(M0)) is computed for each call to the function. Use
        // this GeodesicPath when it is needed for only a single value of t.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, GroupType const& M1)
        {
            return Exp(t * LogM1M0Inv(M0, M1)) * M0;
        }

        // Compute a point on the geodesic path from M0 to M1. The Lie algebra
        // element log(M1*Inverse(M0)) must be precomputed by the caller. Use
        // this GeodesicPath when it is needed for multiple values of t.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, AlgebraType const& logM1M0Inv)
        {
            return Exp(t * logM1M0Inv) * M0;
        }

    private:
        inline static T F0(T const& t)
        {
            if (std::fabs(t) > static_cast<T>(0.0625))
            {
                // Compute sin(t)/t.
                return std::sin(t) / t;
            }
            else
            {
                // Estimate sin(t)/t.
                return RotC0Estimate<T, 16>(t);
            }
        }

        inline static T F1(T const& t)
        {
            if (std::fabs(t) > static_cast<T>(0.0625))
            {
                // Compute (1 - cos(t))/t^2.
                return (static_cast<T>(1) - std::cos(t)) / t / t;
            }
            else
            {
                // Estimate (1 - cos(t))/t^2.
                return RotC1Estimate<T, 16>(t);
            }
        }
    };


    // S0(3) is the Lie group for rotations in 3D. so(3) is the corresponding
    // Lie algebra for SO(3) and is a 3D quantity x = (x0,x1,x2). The 3x3
    // rotation matrix M is generated from x by constructing a 3x3 generator
    // G = x0*G0 + x1*G1 + x2*G2, where
    //   G0 = {{ 0, 0, 0 }, { 0, 0, -1 }, { 0, 1, 0 }}
    //   G1 = {{ 0, 0, 1 }, { 0, 0, 0 }, { -1, 0, 0 }}
    //   G2 = {{ 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 0 }}
    // and then computing the power series M = exp(L(x)). For the sake of
    // notation, exp(x) is used to denote exp(L(x)). The rotation matrix is
    //   M = I + (sin(angle)/angle) * S + ((1 - cos(angle))/angle^2) * S^2
    // where angle is the length of c. The adjoint matrix is
    //   A(M) = M

    template <typename T>
    class LieSO3
    {
    public:
        // n = 3, k = 3, c = (s0,s1,s2)
        using AlgebraType = Vector3<T>;
        using AdjointType = Matrix3x3<T>;
        using GroupType = Matrix3x3<T>;

        // Compute the Lie group element X from the Lie algebra element x
        // using X = L(x).
        static GroupType ToGroup(AlgebraType const& x)
        {
            T const zero = static_cast<T>(0);
            GroupType X{};
            X(0, 0) = zero;
            X(0, 1) = -x[2];
            X(0, 2) = x[1];
            X(1, 0) = x[2];
            X(1, 1) = zero;
            X(1, 2) = -x[0];
            X(2, 0) = -x[1];
            X(2, 1) = x[0];
            X(2, 2) = zero;
            return X;
        }

        // Compute the Lie algebra element x from the Lie group element X
        // using x = L^{-1}(X).
        static AlgebraType ToAlgebra(GroupType const& X)
        {
            return AlgebraType{ X(2, 1), X(0, 2), X(1, 0) };
        }

        // Compute the exponential map of the Lie algebra element x to produce
        // the Lie group element Y = exp(X) = exp(L(x)).
        static GroupType Exp(AlgebraType const& x)
        {
            T const zero = static_cast<T>(0);
            T sqrAngle = Dot(x, x);
            T angle = std::sqrt(sqrAngle);
            if (angle > zero)
            {
                GroupType Y = ToGroup(x);
                GroupType Ysqr = Y * Y;

                // Compute sin(t)/t.
                T a0 = F0(angle);

                // Compute (1 - cos(t))/t^2.
                T a1 = F1(angle);

                GroupType M = GroupType::Identity() + a0 * Y + a1 * Ysqr;
                return M;
            }
            else
            {
                return GroupType::Identity();
            }
        }

        // Compute the logarithm map of the Lie group element Y to produce the
        // Lie algebra element x corresponding to the Lie group element X.
        static AlgebraType Log(GroupType const& Y)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const half = static_cast<T>(0.5);
            AlgebraType x{};

            T arg = half * (Trace(Y) - one);  // in [-1,1]
            if (arg > -one)
            {
                if (arg < one)
                {
                    // 0 < angle < pi
                    T angle = std::acos(arg);
                    // G = (angle / (2*sin(angle)) * (Y - Y^T)
                    T multiplier = half / F0(angle);
                    x[0] = multiplier * (Y(2, 1) - Y(1, 2));
                    x[1] = multiplier * (Y(0, 2) - Y(2, 0));
                    x[2] = multiplier * (Y(1, 0) - Y(0, 1));
                }
                else // arg = 1, angle = 0, Y is the identity, G is zero
                {
                    x.MakeZero();
                }
            }
            else  // arg = -1, angle = pi
            {
                // Observe that Y = I + (2/pi^2) * G^2. Consider x as a 3x1
                // vector; then x * x^T = (pi^2/2)*(Y + I). The right-hand
                // side is a symmetric matrix with positive diagonal entries
                // and rank 1. Choose the row of Y + I that has the largest
                // diagonal term and normalize that row. Multiply it by
                // pi/sqrt(2) to obtain x from which G = ToGenerator(x). The
                // vector -x is also a candidate but irrelevant here because
                // x and -x produce the same rotation matrix. Knowing Y+I is
                // symmetric and wanting to avoid bias, use (Y(i,j)+Y(j,i))/2
                // for the off-diagonal entries rather than Y(i,j).
                if (Y(0, 0) >= Y(1, 1))
                {
                    if (Y(0, 0) >= Y(2, 2))
                    {
                        // r00 is maximum diagonal term
                        x[0] = Y(0, 0) + one;
                        x[1] = half * (Y(0, 1) + Y(1, 0));
                        x[2] = half * (Y(0, 2) + Y(2, 0));
                    }
                    else
                    {
                        // r22 is maximum diagonal term
                        x[0] = half * (Y(2, 0) + Y(0, 2));
                        x[1] = half * (Y(2, 1) + Y(1, 2));
                        x[2] = Y(2, 2) + one;
                    }
                }
                else
                {
                    if (Y(1, 1) >= Y(2, 2))
                    {
                        // r11 is maximum diagonal term
                        x[0] = half * (Y(1, 0) + Y(0, 1));
                        x[1] = Y(1, 1) + one;
                        x[2] = half * (Y(1, 2) + Y(2, 1));
                    }
                    else
                    {
                        // r22 is maximum diagonal term
                        x[0] = half * (Y(2, 0) + Y(0, 2));
                        x[1] = half * (Y(2, 1) + Y(1, 2));
                        x[2] = Y(2, 2) + one;
                    }
                }

                if (Normalize(x) > zero)
                {
                    T angle = static_cast<T>(GTE_C_PI * GTE_C_INV_SQRT_2);
                    for (int32_t i = 0; i < 3; ++i)
                    {
                        x[i] *= angle;
                    }
                }
                else
                {
                    x.MakeZero();
                }
            }

            return x;
        }

        // Compute the adjoint matrix A(M) from the Lie group element M.
        static AdjointType Adjoint(GroupType const& M)
        {
            return M;
        }

        // Compute log(M1*Inverse(M0)) to reduce computation time when you
        // want to evaluate GeodesicPath for multiple values of t. For a
        // rotation matrix M0, Inverse(M0) is equal to Transpose(M0), which
        // avoids a general inversion of M0.
        static AlgebraType LogM1M0Inv(GroupType const& M0, GroupType const& M1)
        {
            return Log(MultiplyABT(M1, M0));
        }

        // Compute a point on the geodesic path from M0 to M1. The expression
        // log(M1*Inverse(M0)) is computed for each call to the function. Use
        // this GeodesicPath when it is needed for only a single value of t.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, GroupType const& M1)
        {
            return Exp(t * LogM1M0Inv(M0, M1)) * M0;
        }

        // Compute a point on the geodesic path from M0 to M1. The Lie algebra
        // element log(M1*Inverse(M0)) must be precomputed by the caller. Use
        // this GeodesicPath when it is needed for multiple values of t.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, AlgebraType const& logM1M0Inv)
        {
            return Exp(t * logM1M0Inv) * M0;
        }

    private:
        inline static T F0(T const& t)
        {
            if (std::fabs(t) > static_cast<T>(0.0625))
            {
                // Compute sin(t)/t.
                return std::sin(t) / t;
            }
            else
            {
                // Estimate sin(t)/t.
                return RotC0Estimate<T, 16>(t);
            }
        }

        inline static T F1(T const& t)
        {
            if (std::fabs(t) > static_cast<T>(0.0625))
            {
                // Compute (1 - cos(t))/t^2.
                return (static_cast<T>(1) - std::cos(t)) / t / t;
            }
            else
            {
                // Estimate (1 - cos(t))/t^2.
                return RotC1Estimate<T, 16>(t);
            }
        }
    };


    // SE(3) is the Lie group for rigid motions in 3D. se(3) is the
    // corresponding Lie algebra for SE(3) and is a 6D quantity
    // x = (s0,s1,s2;u0,u1,u2), where (s0,s1,s2) corresponds to the rotation
    // matrix and (u0,u1,u2) corresponds to the translation vector. The 3x3
    // rigid motion is generated from x by constructing a 6x6 generator
    // G = x0 * G0 + x1 * G1 + x2 * G2 + x3 * G3 + x4 * G4 + x5 * G5, where
    //   G0 = {{0,0,0,0},{0,0,-1,0},{0,1,0,0},{0,0,0,0}}
    //   G1 = {{0,0,1,0},{0,0,0,0},{-1,0,0,0},{0,0,0,0}}
    //   G2 = {{0,-1,0,0},{1,0,0,0},{0,0,0,0},{0,0,0,0}}
    //   G3 = {{0,0,0,1},{0,0,0,0},{0,0,0,0},{0,0,0,0}}
    //   G4 = {{0,0,0,0},{0,0,0,1},{0,0,0,0},{0,0,0,0}}
    //   G5 = {{0,0,0,0},{0,0,0,0},{0,0,0,1},{0,0,0,0}}
    // and then computing the power series M = exp(L(x)). For the sake of
    // notation, exp(x) is used to denote exp(L(x)). The rigit motion matrix
    // is
    //   M = {{ R, T }, { 0, 1 }}
    // where R is the 3x3 rotation matrix, T is the 3x1 translation vector,
    // 0 is the 1x3 zero vector and 1 is a scalar. The adjoint matrix is
    //   A(M) = {{ R, Skew(T)*R }, { 0, R }}
    // where Skew{T} = {{ 0, -T2, T1 }, { T2, 0, -T0 }, { -T1, T0, 0 }}
    // and 0 is the 3x3 zero matrix.

    template <typename T>
    class LieSE3
    {
    public:
        // n = 4, k = 6, c = (s0,s1,s2,u0,u1,u2)
        using AlgebraType = Vector<6, T>;
        using AdjointType = Matrix<6, 6, T>;
        using GroupType = Matrix4x4<T>;

        // Compute the Lie group element X from the Lie algebra element x
        // using X = L(x).
        static GroupType ToGroup(AlgebraType const& x)
        {
            T const zero = static_cast<T>(0);
            GroupType X{};
            X(0, 0) = zero;
            X(0, 1) = -x[2];
            X(0, 2) = x[1];
            X(0, 3) = x[3];
            X(1, 0) = x[2];
            X(1, 1) = zero;
            X(1, 2) = -x[0];
            X(1, 3) = x[4];
            X(2, 0) = -x[1];
            X(2, 1) = x[0];
            X(2, 2) = zero;
            X(2, 3) = x[5];
            X(3, 0) = zero;
            X(3, 1) = zero;
            X(3, 2) = zero;
            X(3, 3) = zero;
            return X;
        }

        // Compute the Lie algebra element x from the Lie group element X
        // using x = L^{-1}(X).
        static AlgebraType ToAlgebra(GroupType const& X)
        {
            return AlgebraType{ X(2, 1), X(0, 2), X(1, 0), X(0, 3), X(1, 3), X(2, 3) };
        }

        // Compute the exponential map of the Lie algebra element x to produce
        // the Lie group element Y = exp(X) = exp(L(x)).
        static GroupType Exp(AlgebraType const& x)
        {
            Vector3<T> s{ x[0], x[1], x[2] };
            Vector3<T> u{ x[3], x[4], x[5] };
            Matrix3x3<T> S = LieSO3<T>::ToGroup(s);
            Matrix3x3<T> Ssqr = S * S;
            T sqrAngle = Dot(s, s);
            T angle = std::sqrt(sqrAngle);
            T a0 = F0(angle);
            T a1 = F1(angle);
            T a2 = F2(angle);
            Matrix3x3<T> R = Matrix3x3<T>::Identity() + a0 * S + a1 * Ssqr;
            Matrix3x3<T> V = Matrix3x3<T>::Identity() + a1 * S + a2 * Ssqr;
            Vector3<T> trn = V * u;
            GroupType Y{};
            Y = HLift(R);
            Y(0, 3) = trn[0];
            Y(1, 3) = trn[1];
            Y(2, 3) = trn[2];
            return Y;
        }

        // Compute the logarithm map of the Lie group element Y to produce the
        // Lie algebra element x corresponding to the Lie group element X.
        static AlgebraType Log(GroupType const& Y)
        {
            Matrix3x3<T> R = HProject(Y);
            Vector3<T> s = LieSO3<T>::Log(R);
            Matrix3x3<T> S = LieSO3<T>::ToGroup(s);
            Matrix3x3<T> Ssqr = S * S;
            T sqrAngle = Dot(s, s);
            T angle = std::sqrt(sqrAngle);
            T a1 = F1(angle);
            T a2 = F2(angle);
            Matrix3x3<T> V = Matrix3x3<T>::Identity() + a1 * S + a2 * Ssqr;
            Matrix3x3<T> inverseV = Inverse(V);
            Vector3<T> trn{ Y(0, 3), Y(1, 3), Y(2, 3) };
            Vector3<T> u = inverseV * trn;
            return AlgebraType{ s[0], s[1], s[2], u[0], u[1], u[2] };
        }

        // Compute the adjoint matrix A(M) from the Lie group element M.
        static AdjointType Adjoint(GroupType const& M)
        {
            T const zero = static_cast<T>(0);
            Matrix3x3<T> R = HProject(M);
            Matrix3x3<T> skewT = LieSO3<T>::ToGroup(Vector3<T>{ M(0, 3), M(1, 3), M(2, 3) });
            Matrix3x3<T> skewTR = skewT * R;
            AdjointType A{};
            for (int32_t row = 0, rowp3 = 3; row < 3; ++row, ++rowp3)
            {
                for (int32_t col = 0, colp3 = 3; col < 3; ++col, ++colp3)
                {
                    A(row, col) = M(row, col);
                    A(row, colp3) = zero;
                    A(rowp3, col) = skewTR(row, col);
                    A(rowp3, colp3) = M(row, col);
                }
            }
            return A;
        }

        // Compute log(M1*Inverse(M0)) to reduce computation time when you
        // want to evaluate GeodesicPath for multiple values of t.
        static AlgebraType LogM1M0Inv(GroupType const& M0, GroupType const& M1)
        {
            return Log(M1 * Inverse(M0));
        }

        // Compute a point on the geodesic path from M0 to M1. The expression
        // log(M1*Inverse(M0)) is computed for each call to the function. Use
        // this GeodesicPath when it is needed for only a single value of t.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, GroupType const& M1)
        {
            return Exp(t * LogM1M0Inv(M0, M1)) * M0;
        };

        // Compute a point on the geodesic path from M0 to M1. The Lie algebra
        // element log(M1*Inverse(M0)) must be precomputed by the caller. Use
        // this GeodesicPath when it is needed for multiple values of t.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, AlgebraType const& logM1M0Inv)
        {
            return Exp(t * logM1M0Inv) * M0;
        }

    private:
        inline static T F0(T const& t)
        {
            if (std::fabs(t) > static_cast<T>(0.0625))
            {
                // Compute sin(t)/t.
                return std::sin(t) / t;
            }
            else
            {
                // Estimate sin(t)/t.
                return RotC0Estimate<T, 16>(t);
            }
        }

        inline static T F1(T const& t)
        {
            if (std::fabs(t) > static_cast<T>(0.0625))
            {
                // Compute (1 - cos(t))/t^2.
                return (static_cast<T>(1) - std::cos(t)) / t / t;
            }
            else
            {
                // Estimate (1 - cos(t))/t^2.
                return RotC1Estimate<T, 16>(t);
            }
        }

        inline static T F2(T const& t)
        {
            if (std::fabs(t) > static_cast<T>(0.0625))
            {
                // Compute (t - sin(t))/t^3.
                return (t - std::sin(t)) / t / t / t;
            }
            else
            {
                // Estimate (t - sin(t))/t^3.
                return RotC4Estimate<T, 16>(t);
            }
        }
    };
}
