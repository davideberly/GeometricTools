// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Lie groups and Lie algebras are useful for representing special classes of
// matrices found in applications. In particular, implementations of the
// exponential map and logarithm function are provided. If M is a matrix in a
// Lie group and if G is a generator matrix for the corresponding Lie algebra
// element L, then M = exp(G) and G = log(M). The exp(G) is computed using the
// Taylor series exp(x) = sum_{n=0}^infinity x^n/n! for a real variable x but
// with G formally substituted for x in the series.
//
// The adjoint for a Lie group element M linearly transforms a Lie algebra
// element L that is a tangent vector in one space to a Lie algebra element L'
// that is a tangent vector in another space. Let G[i] for 0 <= i < k be the
// k linearly independent generators of the matrix representation for the Lie
// algebra, each generator an n-by-n matrix. A Lie algebra element L can be
// written as a linear combination of the G[i]. Similarly, let W be the matrix
// representation of the Lie algebra element for which M = exp(W). The adjoint
// has a k-by-k matrix representation, say, Adjoint(W), where the Lie algebra
// elements are transformed as matrices, L' = Adjoint(W)*L. The i-th column of
// the matrix Adjoint(W) is the k-by-1 vector of coefficients for the Lie
// bracket [W,G[i]] written as a linear combination of the generators. The
// matrix is sometimes also represented as an n-by-n Lie group element,
// Adjoint(M) = exp(Adjoint(W)).
//
// Construction of geodesic paths between two group elements is included. Such
// a path necessarily lives on the manifold of the group. The prototypical
// case is the geodesic path connecting two rotation matrices, which is
// effectively captured by the slerp (spherical linear interpolation)
// operation for quaternions. Generally, if M0 and M1 are Lie group elements,
// the geodesic path connecting M0 and M1 is parameterized by
//   F(t;M0,M1) = exp(t * log(M1 * M0^{-1})) * M0
// for t in [0,1]. Observe that F(0;M0,M1) = M0 and F(1;M0,M1) = M1.
//
// In the source code, a Lie algebra element is L, the corresponding generator
// is G = ToGenerator(L) with inverse L = ToAlgebra(G) and the corresponding
// Lie group element is M. The exponential map is M = exp(G) = exp(Alg(L))
// and the logarithm map is G = log(M) [as a generator] or L = InvAlg(log(M))
// [as a Lie algebra element].
//
// TODO: For small angles, use minimax approximations for sin(z)/z,
// (1 - cos(z))/z^2 and other such trigonometric expressions that have a
// removable singularity at z = 0.

#include <GTL/Mathematics/Algebra/Matrix.h>
#include <cmath>

// SO(2): Rotations in 2-dimensional space.
namespace gtl
{
    // S0(2) is the Lie group for rotations in 2D. so(2) is the corresponding
    // Lie algebra for SO(2) and is a 1D quantity c. The 2x2 rotation matrix M
    // is generated from c by constructing a 2x2 generator G = c*G0, where
    //   G0 = {{ 0, -1 },{ 1, 0 }}
    // and then computing the power series M = exp(G(c)). For the sake of
    // notation, exp(c) is used to denote exp(G(c)). The 2x2 rotation matrix
    // is
    //   M = {{ cos(c), -sin(c) }, { sin(c), cos(c) }}
    // The adjoint matrix is
    //   Adjoint(M) = I
    // where I is the 2x2 identity matrix.

    template <typename T>
    class LieSO2
    {
    public:
        using value_type = T;

        // n = 2, k = 1, c = (angle)
        using AlgebraType = T;
        using AdjointType = Matrix2x2<T>;
        using GeneratorType = Matrix2x2<T>;
        using GroupType = GeneratorType;

        // Compute a generator G from the Lie algebra element c.
        static GeneratorType ToGenerator(AlgebraType const& c)
        {
            GeneratorType G{};
            G(0, 0) = C_<T>(0);
            G(0, 1) = -c;
            G(1, 0) = c;
            G(1, 1) = C_<T>(0);
            return G;
        }

        // Compute the Lie algebra element c from a generator G.
        static AlgebraType ToAlgebra(GeneratorType const& G)
        {
            return G(1, 0);
        }
    
        // Compute the Lie group element M from the Lie algebra element c.
        static GroupType Exp(AlgebraType const& c)
        {
            GroupType M{};
            T sn = std::sin(c);
            T cs = std::cos(c);
            M(0, 0) = cs;
            M(0, 1) = -sn;
            M(1, 0) = sn;
            M(1, 1) = cs;
            return M;
        }

        // Compute the Lie algebra element c from the Lie group element M.
        static AlgebraType Log(GroupType const& M)
        {
            return std::atan2(M(1, 0), M(0, 0));
        }

        // Compute the adjoint matrix Adjoint(M) from the Lie algebra
        // element c. In this class, the adjoint is always the identity
        // matrix regardless of the value of c.
        static AdjointType Adjoint(AlgebraType const&)
        {
            return AdjointType{ { C_<T>(1), C_<T>(0) }, { C_<T>(0), C_<T>(1) }};
        }

        // Helper function to compute log(M1*M0^{-1}).
        static AlgebraType LogM1M0Inv(GroupType const& M0, GroupType const& M1)
        {
            return Log(MultiplyABT(M1, M0));
        }

        // Compute a point on the geodesic path from M0 to M1. The expression
        // log(M1*M0^{-1}) is computed for each call to the function.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, GroupType const& M1)
        {
            return Exp(t * LogM1M0Inv(M0, M1)) * M0;
        }

        // Compute a point on the geodesic path from M0 to M1. The Lie algebra
        // element log(M1*M0^{-1}) must be precomputed by the caller.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, AlgebraType const& logM1M0Inv)
        {
            return Exp(t * logM1M0Inv) * M0;
        }

    private:
        friend class UnitTestLieGroupsAlgebras;
    };
}

// SE(2): Rigid motions (rotation and translation) in 2-dimensional space.
namespace gtl
{
    // SE(2) is the Lie group for rigid motions in 2D. se(2) is the
    // corresponding Lie algebra for SE(2) and is a 3D quantity
    // c = (angle; u0, u1), where the angle is for the rotation matrix and
    // (u0, u1) is for the translation vector. The 3x3 rigid motion M is
    // generated from c by constructing a 3x3 generator G = c0*G0+c1*G1+c2*G2,
    // where
    //   G0 = {{ 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 0 }}
    //   G1 = {{ 0, 0, 1 }, { 0, 0, 0 }, { 0, 0, 0 }}
    //   G2 = {{ 0, 0, 0 }, { 0, 0, 1 }, { 0, 0, 0 }}
    // and then computing the power series M = exp(G(c)). For the sake of
    // notation, exp(c) is used to denote exp(G(c)). The motion matrix is
    //   M = {{ R, T }, { 0, 1 }}
    // where R is the 2x2 rotation matrix, T is the 2x1 translation vector,
    // 0 is the 1x2 zero vector and 1 is a scalar. The adjoint matrix is
    //   Adjoint(M) = {{ R, Perp(T) }, { 0, 1 }}
    // where Perp(x,y) = (y,-x).

    template <typename T>
    class LieSE2
    {
    public:
        using value_type = T;

        // n = 3, k = 3, c = (angle; u0, u1)
        using AlgebraType = Vector3<T>;
        using AdjointType = Matrix3x3<T>;
        using GeneratorType = Matrix3x3<T>;
        using GroupType = GeneratorType;

        // Compute a generator G from the Lie algebra element c.
        static GeneratorType ToGenerator(AlgebraType const& c)
        {
            GeneratorType G{};
            G(0, 0) = C_<T>(0);
            G(0, 1) = -c[0];
            G(0, 2) = c[1];
            G(1, 0) = c[0];
            G(1, 1) = C_<T>(0);
            G(1, 2) = c[2];
            G(2, 0) = C_<T>(0);
            G(2, 1) = C_<T>(0);
            G(2, 2) = C_<T>(0);
            return G;
        }

        // Compute the Lie algebra element c from a generator G.
        static AlgebraType ToAlgebra(GeneratorType const& G)
        {
            return AlgebraType{ G(1, 0), G(0, 2), G(1, 2) };
        }

        // Compute the Lie group element M from the Lie algebra element c.
        static GroupType Exp(AlgebraType const& c)
        {
            GroupType M{};

            if (std::fabs(c[0]) > C_<T>(0))
            {
                T sn = std::sin(c[0]);
                T cs = std::cos(c[0]);
                T omcs = C_<T>(1) - cs;
                T trn0 = (sn * c[1] - omcs * c[2]) / c[0];
                T trn1 = (omcs * c[1] + sn * c[2]) / c[0];
                M(0, 0) = cs;
                M(0, 1) = -sn;
                M(0, 2) = trn0;
                M(1, 0) = sn;
                M(1, 1) = cs;
                M(1, 2) = trn1;
                M(2, 0) = C_<T>(0);
                M(2, 1) = C_<T>(0);
                M(2, 2) = C_<T>(1);
            }
            else
            {
                MakeIdentity(M);
                M(0, 2) = c[1];
                M(1, 2) = c[2];
            }

            return M;
        }

        // Compute the Lie algebra element c from the Lie group element M.
        static AlgebraType Log(GroupType const& M)
        {
            AlgebraType c{};

            c[0] = std::atan2(M(1, 0), M(0, 0));
            if (std::fabs(c[0]) > C_<T>(0))
            {
                T omm00 = C_<T>(1) - M(0, 0);
                c[1] = C_<T>(1, 2) * (M(1, 0) * M(0, 2) + omm00 * M(1, 2)) / omm00;
                c[2] = C_<T>(1, 2) * (M(1, 0) * M(1, 2) - omm00 * M(0, 2)) / omm00;
            }
            else
            {
                c[1] = M(0, 2);
                c[2] = M(1, 2);
            }

            return c;
        }

        // Compute the adjoint matrix Adjoint(M) from the Lie algebra
        // element c.
        static AdjointType Adjoint(AlgebraType const& c)
        {
            auto R = LieSO2<T>::Exp(c[0]);
            AdjointType adjoint{};
            adjoint(0, 0) = R(0, 0);
            adjoint(0, 1) = R(0, 1);
            adjoint(0, 2) = c[2];
            adjoint(1, 0) = R(1, 0);
            adjoint(1, 1) = R(1, 1);
            adjoint(1, 2) = -c[1];
            adjoint(2, 0) = C_<T>(0);
            adjoint(2, 1) = C_<T>(0);
            adjoint(2, 2) = C_<T>(1);
            return adjoint;
        }

        // Helper function to compute log(M1*M0^{-1}).
        static AlgebraType LogM1M0Inv(GroupType const& M0, GroupType const& M1)
        {
            Matrix2x2<T> rot0 = HProject(M0);
            Vector2<T> trn0 = { M0(0, 2), M0(1, 2) };
            Matrix2x2<T> rot1 = HProject(M1);
            Vector2<T> trn1 = { M1(0, 2), M1(1, 2) };
            Matrix2x2<T> rot = MultiplyABT(rot1, rot0);
            Vector2<T> trn = trn1 - rot * trn0;
            Matrix3x3<T> M1M0Inv = HLift(rot);
            M1M0Inv(0, 2) = trn[0];
            M1M0Inv(1, 2) = trn[1];
            return Log(M1M0Inv);
        }

        // Compute a point on the geodesic path from M0 to M1. The expression
        // log(M1*M0^{-1}) is computed for each call to the function.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, GroupType const& M1)
        {
            return Exp(t * LogM1M0Inv(M0, M1)) * M0;
        }

        // Compute a point on the geodesic path from M0 to M1. The Lie
        // algebra element log(M1*M0^{-1}) must be precomputed by the caller.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, AlgebraType const& logM1M0Inv)
        {
            return Exp(t * logM1M0Inv) * M0;
        }

    private:
        friend class UnitTestLieGroupsAlgebras;
    };
}

// SO(3): Rotations in 3-dimensional space.
namespace gtl
{
    // S0(3) is the Lie group for rotations in 3D. so(3) is the corresponding
    // Lie algebra for SO(3) and is a 3D quantity c = (c0,c1,c2). The 3x3
    // rotation matrix M is generated from c by constructing a 3x3 generator
    // G = c0*G0 + c1*G1 + c2*G2, where
    //   G0 = {{ 0, 0, 0 }, { 0, 0, -1 }, { 0, 1, 0 }}
    //   G1 = {{ 0, 0, 1 }, { 0, 0, 0 }, { -1, 0, 0 }}
    //   G2 = {{ 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 0 }}
    // and then computing the power series M = exp(G(c)). For the sake of
    // notation, exp(c) is used to denote exp(G(c)). The rotation matrix is
    //   M = I + (sin(angle)/angle) * G + ((1 - cos(angle))/angle^2) * G^2
    // where angle is the length of c. The adjoint matrix is
    //   Adjoint(M) = M

    template <typename T>
    class LieSO3
    {
    public:
        using value_type = T;
        
        // n = 3, k = 3, c = (s0,s1,s2)
        using AlgebraType = Vector3<T>;
        using AdjointType = Matrix3x3<T>;
        using GeneratorType = Matrix3x3<T>;
        using GroupType = GeneratorType;

        // Compute a generator G from the Lie algebra element c.
        static GeneratorType ToGenerator(AlgebraType const& c)
        {
            GeneratorType G{};
            G(0, 0) = C_<T>(0);
            G(0, 1) = -c[2];
            G(0, 2) = c[1];
            G(1, 0) = c[2];
            G(1, 1) = C_<T>(0);
            G(1, 2) = -c[0];
            G(2, 0) = -c[1];
            G(2, 1) = c[0];
            G(2, 2) = C_<T>(0);
            return G;
        }

        // Compute the Lie algebra element c from a generator G.
        static AlgebraType ToAlgebra(GeneratorType const& G)
        {
            return AlgebraType{ G(2, 1), G(0, 2), G(1, 0) };
        }

        // Compute the Lie group element M from the Lie algebra element c.
        static GroupType Exp(AlgebraType const& c)
        {
            GroupType M{};
            MakeIdentity(M);

            T sqrAngle = Dot(c, c);
            T angle = std::sqrt(sqrAngle);
            if (angle > C_<T>(0))
            {
                GeneratorType G = ToGenerator(c);
                GeneratorType Gsqr = G * G;
                T sinAngle = std::sin(angle);
                T cosAngle = std::cos(angle);
                T c0 = sinAngle / angle;
                T c1 = (C_<T>(1) - cosAngle) / sqrAngle;
                M += c0 * G + c1 * Gsqr;
            }

            return M;
        }

        // Compute the Lie algebra element c from the Lie group element M.
        static AlgebraType Log(GroupType const& M)
        {
            // The generator is G = {{0,-c2,c1},{c2,0,-c0},{-c1,c0,0}}.
            AlgebraType c{};

            T arg = C_<T>(1, 2) * (Trace(M) - C_<T>(1));  // in [-1,1]
            if (arg > -C_<T>(1))
            {
                if (arg < C_<T>(1))
                {
                    // 0 < angle < pi
                    T angle = std::acos(arg);
                    T sinAngle = std::sin(angle);
                    // G = (angle / (2*sin(angle)) * (M - M^T)
                    T multiplier = C_<T>(1, 2) * angle / sinAngle;
                    c[0] = multiplier * (M(2, 1) - M(1, 2));
                    c[1] = multiplier * (M(0, 2) - M(2, 0));
                    c[2] = multiplier * (M(1, 0) - M(0, 1));
                }
                else // arg = 1, angle = 0, M is the identity, G is zero
                {
                    MakeZero(c);
                }
            }
            else  // arg = -1, angle = pi
            {
                // Observe that M = I + (2/pi^2) * G^2. Consider c as a 3x1
                // vector; then c * c^T = (pi^2/2)*(M + I). The right-hand
                // side is a symmetric matrix with positive diagonal entries
                // and rank 1. Choose the row of M + I that has the largest
                // diagonal term and normalize that row. Multiply it by
                // pi/sqrt(2) to obtain c from which G = ToGenerator(c). The
                // vector -c is also a candidate but irrelevant here because
                // c and -c produce the same rotation matrix. Knowing M+I is
                // symmetric, and wanting to avoid bias, use
                // (M(i,j) + M(j,i)) / 2 for the off-diagonal entries rather
                // than M(i,j).
                if (M(0, 0) >= M(1, 1))
                {
                    if (M(0, 0) >= M(2, 2))
                    {
                        // r00 is maximum diagonal term
                        c[0] = M(0, 0) + C_<T>(1);
                        c[1] = C_<T>(1, 2) * (M(0, 1) + M(1, 0));
                        c[2] = C_<T>(1, 2) * (M(0, 2) + M(2, 0));
                    }
                    else
                    {
                        // r22 is maximum diagonal term
                        c[0] = C_<T>(1, 2) * (M(2, 0) + M(0, 2));
                        c[1] = C_<T>(1, 2) * (M(2, 1) + M(1, 2));
                        c[2] = M(2, 2) + C_<T>(1);
                    }
                }
                else
                {
                    if (M(1, 1) >= M(2, 2))
                    {
                        // r11 is maximum diagonal term
                        c[0] = C_<T>(1, 2) * (M(1, 0) + M(0, 1));
                        c[1] = M(1, 1) + C_<T>(1);
                        c[2] = C_<T>(1, 2) * (M(1, 2) + M(2, 1));
                    }
                    else
                    {
                        // r22 is maximum diagonal term
                        c[0] = C_<T>(1, 2) * (M(2, 0) + M(0, 2));
                        c[1] = C_<T>(1, 2) * (M(2, 1) + M(1, 2));
                        c[2] = M(2, 2) + C_<T>(1);
                    }
                }

                T sqrLength = Dot(c, c);
                T length = std::sqrt(sqrLength);
                if (length > C_<T>(0))
                {
                    T adjust = C_PI<T> * C_INV_SQRT_2<T> / length;
                    for (size_t i = 0; i < 3; ++i)
                    {
                        c[i] *= adjust;
                    }
                }
                else
                {
                    MakeZero(c);
                }
            }

            return c;
        }

        // Compute the adjoint matrix Adjoint(M) from the Lie algebra
        // element c.
        static AdjointType Adjoint(AlgebraType const& c)
        {
            return Exp(c);
        }

        // Helper function to compute log(M1*M0^{-1}).
        static AlgebraType LogM1M0Inv(GroupType const& M0, GroupType const& M1)
        {
            return Log(MultiplyABT(M1, M0));
        }

        // Compute a point on the geodesic path from M0 to M1. The expression
        // log(M1*M0^{-1}) is computed for each call to the function.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, GroupType const& M1)
        {
            return Exp(t * LogM1M0Inv(M0, M1)) * M0;
        }

        // Compute a point on the geodesic path from M0 to M1. The Lie
        // algebra element log(M1*M0^{-1}) must be precomputed by the caller.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, AlgebraType const& logM1M0Inv)
        {
            return Exp(t * logM1M0Inv) * M0;
        }

    private:
        friend class UnitTestLieGroupsAlgebras;
    };
}

// SE(3): Rigid motions (rotation and translation) in 3-dimensional space.
namespace gtl
{
    // SE(3) is the Lie group for rigid motions in 3D. se(3) is the
    // corresponding Lie algebra for SE(3) and is a 6D quantity
    // c = (s0,s1,s2;u0,u1,u2), where (s0,s1,s2) corresponds to the rotation
    // matrix and (u0,u1,u2) corresponds to the translation vector. The 3x3
    // rigid motion is generated from c by constructing a 6x6 generator
    // G = c0*G0+c1*G1+c2*G2+c3*G3+c4*G4+c5*G5, where
    //   G0 = {{0,0,0,0},{0,0,-1,0},{0,1,0,0},{0,0,0,0}}
    //   G1 = {{0,0,1,0},{0,0,0,0},{-1,0,0,0},{0,0,0,0}}
    //   G2 = {{0,-1,0,0},{1,0,0,0},{0,0,0,0},{0,0,0,0}}
    //   G3 = {{0,0,0,1},{0,0,0,0},{0,0,0,0},{0,0,0,0}}
    //   G4 = {{0,0,0,0},{0,0,0,1},{0,0,0,0},{0,0,0,0}}
    //   G5 = {{0,0,0,0},{0,0,0,0},{0,0,0,1},{0,0,0,0}}
    // and then computing the power series M = exp(G(c)). For the sake of
    // notation, exp(c) is used to denote exp(G(c)). The motion matrix is
    //   M = {{ R, T }, { 0, 1 }}
    // where R is the 3x3 rotation matrix, T is the 3x1 translation vector,
    // 0 is the 1x3 zero vector and 1 is a scalar. The adjoint matrix is
    //   Adjoint(M) = {{ R, Skew(T)*R }, { 0, R }}
    // where Skew{T} = {{ 0, -T2, T1 }, { T2, 0, -T0 }, { -T1, T0, 0 }}
    // and 0 is the 3x3 zero matrix.

    template <typename T>
    class LieSE3
    {
    public:
        using value_type = T;

        // n = 4, k = 6, c = (s0,s1,s2,u0,u1,u2)
        using AlgebraType = Vector<T, 6>;
        using AdjointType = Matrix<T, 6, 6>;
        using GeneratorType = Matrix4x4<T>;
        using GroupType = GeneratorType;

        // Compute a generator G from the Lie algebra element c.
        static GeneratorType ToGenerator(AlgebraType const& c)
        {
            GeneratorType G{};
            G(0, 0) = C_<T>(0);
            G(0, 1) = -c[2];
            G(0, 2) = c[1];
            G(0, 3) = c[3];
            G(1, 0) = c[2];
            G(1, 1) = C_<T>(0);
            G(1, 2) = -c[0];
            G(1, 3) = c[4];
            G(2, 0) = -c[1];
            G(2, 1) = c[0];
            G(2, 2) = C_<T>(0);
            G(2, 3) = c[5];
            G(3, 0) = C_<T>(0);
            G(3, 1) = C_<T>(0);
            G(3, 2) = C_<T>(0);
            G(3, 3) = C_<T>(0);
            return G;
        }

        // Compute the Lie algebra element c from a generator G.
        static AlgebraType ToAlgebra(GeneratorType const& G)
        {
            return AlgebraType{ G(2, 1), G(0, 2), G(1, 0), G(0, 3), G(1, 3), G(2, 3) };
        }

        // Compute the Lie group element M from the Lie algebra element c.
        static GroupType Exp(AlgebraType const& c)
        {
            GroupType M{};

            Vector3<T> s{ c[0], c[1], c[2] };
            Vector3<T> u{ c[3], c[4], c[5] };
            T sqrAngle = Dot(s, s);
            T angle = std::sqrt(sqrAngle);
            if (angle > C_<T>(0))
            {
                Matrix3x3<T> R{}, V{};
                MakeIdentity(R);
                MakeIdentity(V);
                Matrix3x3<T> G = LieSO3<T>::ToGenerator(s);
                Matrix3x3<T> Gsqr = G * G;
                T sinAngle = std::sin(angle);
                T cosAngle = std::cos(angle);
                T a0 = sinAngle / angle;
                T a1 = (C_<T>(1) - cosAngle) / sqrAngle;
                T a2 = (C_<T>(1) - a0) / sqrAngle;
                R += a0 * G + a1 * Gsqr;
                V += a1 * G + a2 * Gsqr;
                Vector3<T> trn = V * u;
                M = HLift(R);
                M(0, 3) = trn[0];
                M(1, 3) = trn[1];
                M(2, 3) = trn[2];
            }
            else
            {
                MakeIdentity(M);
                M(0, 3) = u[0];
                M(1, 3) = u[1];
                M(2, 3) = u[2];
            }

            return M;
        }

        // Compute the Lie algebra element c from the Lie group element M.
        static AlgebraType Log(GroupType const& M)
        {
            Matrix3x3<T> rot = HProject(M);
            Vector3<T> s = LieSO3<T>::Log(rot), u{};
            Vector3<T> trn{ M(0, 3), M(1, 3), M(2, 3) };

            T sqrAngle = Dot(s, s);
            T angle = std::sqrt(sqrAngle);
            if (angle > C_<T>(0))
            {
                Matrix3x3<T> G = LieSO3<T>::ToGenerator(s);
                Matrix3x3<T> Gsqr = G * G;
                T sinAngle = std::sin(angle);
                T cosAngle = std::cos(angle);
                T a3 = -C_<T>(1, 2);
                T a4 = (C_<T>(1) - C_<T>(1, 2) * angle * sinAngle / (C_<T>(1) - cosAngle)) / sqrAngle;
                Matrix3x3<T> invV{};
                MakeIdentity(invV);
                invV += a3 * G + a4 * Gsqr;
                u = invV * trn;
            }
            else
            {
                MakeZero(s);
                u = trn;
            }

            return AlgebraType{ s[0], s[1], s[2], u[0], u[1], u[2] };
        }

        // Compute the adjoint matrix Adjoint(M) from the Lie algebra
        // element c.
        static AdjointType Adjoint(AlgebraType const& c)
        {
            Vector3<T> s{ c[0], c[1], c[2] };
            Vector3<T> u{ c[3], c[4], c[5] };
            auto R = LieSO3<T>::Exp(s);
            auto skewT = LieSO3<T>::ToGenerator(u);
            auto product = skewT * R;
            AdjointType adjoint{};
            for (size_t row = 0, rowP3 = 3; row < 3; ++row, ++rowP3)
            {
                for (size_t col = 0, colP3 = 3; col < 3; ++col, ++colP3)
                {
                    adjoint(row, col) = R(row, col);
                    adjoint(rowP3, col) = C_<T>(0);
                    adjoint(row, colP3) = product(row, col);
                    adjoint(rowP3, colP3) = R(row, col);
                }
            }
            return adjoint;
        }

        // Helper function to compute log(M1*M0^{-1}).
        static AlgebraType LogM1M0Inv(GroupType const& M0, GroupType const& M1)
        {
            Matrix3x3<T> rot0 = HProject(M0);
            Vector3<T> trn0 = { M0(0, 3), M0(1, 3), M0(2, 3) };
            Matrix3x3<T> rot1 = HProject(M1);
            Vector3<T> trn1 = { M1(0, 3), M1(1, 3), M1(2, 3) };
            Matrix3x3<T> rot = MultiplyABT(rot1, rot0);
            Vector3<T> trn = trn1 - rot * trn0;
            Matrix4x4<T> M1M0Inv = HLift(rot);
            M1M0Inv(0, 3) = trn[0];
            M1M0Inv(1, 3) = trn[1];
            M1M0Inv(2, 3) = trn[2];
            return Log(M1M0Inv);
        }

        // Compute a point on the geodesic path from M0 to M1. The expression
        // log(M1*M0^{-1}) is computed for each call to the function.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, GroupType const& M1)
        {
            return Exp(t * LogM1M0Inv(M0, M1)) * M0;
        };

        // Compute a point on the geodesic path from M0 to M1. The expression
        // log(M1*M0^{-1}) must be precomputed by the caller.
        static GroupType GeodesicPath(T const& t, GroupType const& M0, AlgebraType const& logM1M0Inv)
        {
            return Exp(t * logM1M0Inv) * M0;
        }

    private:
        friend class UnitTestLieGroupsAlgebras;
    };
}
