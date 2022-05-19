// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The hyperplane is represented as Dot(U, X - P) = 0 where U is a unit-length
// normal vector, P is the hyperplane origin, and X is any point on the
// hyperplane. The user must ensure that the normal vector is unit length. The
// hyperplane constant is c = Dot(U, P) so that Dot(U, X) = c. If P is not
// specified when constructing a hyperplane, it is chosen to be the point on
// the plane closest to the origin, P = c * U.
//
// NOTE: You cannot set 'origin' and 'constant' independently. Use the
// constructors instead.
//
// // Construct from normal N and constant c.
// Plane3<T> plane(N, c);  // plane.origin = c * N
// 
// // Construct from normal N and origin P.
// Plane3<T> plane(N, P);  // plane.constant = Dot(N, P)
//
// Plane3<T> plane{};  // N = (0,0,0), P = (0,0,0), c = 0 [invalid]
// plane.normal = (0,0,1);
// plane.constant = 3;
// // If you consume plane now, the origin and constant are inconsistent
// // because P = (0,0,0) but Dot(N,P) = 0 != 3 = c. Instead use
// plane = Plane3<T>({ 0, 0, 1 }, 3);

#include <GTL/Mathematics/Algebra/Matrix.h>
#include <GTL/Mathematics/MatrixAnalysis/SingularValueDecomposition.h>
#include <GTL/Utility/TypeTraits.h>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class Hyperplane
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all member to zero.
        Hyperplane()
            :
            normal{},
            origin{},
            constant(C_<T>(0))
        {
        }

        Hyperplane(Vector<T, N> const& inNormal, T const& inConstant)
            :
            normal(inNormal),
            origin(inConstant * inNormal),
            constant(inConstant)
        {
        }

        Hyperplane(Vector<T, N> const& inNormal, Vector<T, N> const& inOrigin)
            :
            normal(inNormal),
            origin(inOrigin),
            constant(Dot(inNormal, inOrigin))
        {
        }

        // U is a unit-length vector in the orthogonal complement of the set
        // {p[1]-p[0],...,p[n-1]-p[0]} and c = Dot(U,p[0]), where the p[i] are
        // points on the hyperplane.
        Hyperplane(std::array<Vector<T, N>, N> const& p)
            :
            normal{},
            origin{},
            constant(C_<T>(0))
        {
            ComputeFromPoints<N>(p);
        }

        // Public member access.
        Vector<T, N> normal;
        Vector<T, N> origin;
        T constant;

    private:
        // For use in the Hyperplane(std::array<*>) constructor when N == 2.
        template <size_t _N = N, TraitSelector<_N == 2> = 0>
        void ComputeFromPoints(std::array<Vector<T, N>, N> const& p)
        {
            Vector<T, N> edge = p[1] - p[0];
            normal = UnitPerp(edge);
            constant = Dot(normal, p[0]);
            origin = constant * normal;
        }

        // For use in the Hyperplane(std::array<*>) constructor when N == 3.
        template <size_t _N = N, TraitSelector<_N == 3> = 0>
        void ComputeFromPoints(std::array<Vector<T, N>, N> const& p)
        {
            Vector<T, N> edge0 = p[1] - p[0];
            Vector<T, N> edge1 = p[2] - p[0];
            normal = UnitCross(edge0, edge1);
            constant = Dot(normal, p[0]);
            origin = constant * normal;
        }

        // For use in the Hyperplane(std::array<*>) constructor when N > 3.
        template <size_t _N = N, TraitSelector<(_N > 3)> = 0>
        void ComputeFromPoints(std::array<Vector<T, N>, N> const& p)
        {
            Matrix<T, N, N - 1> edge;
            for (size_t i0 = 0, i1 = 1; i1 < N; i0 = i1++)
            {
                edge.SetCol(i0, p[i1] - p[0]);
            }

            // Compute the 1-dimensional orthogonal complement of the edges of
            // the simplex formed by the points p[].
            size_t const maxIterations = 32;
            SingularValueDecomposition<T> svd(N - 1, N - 1, maxIterations);
            svd.Solve(edge.data());
            svd.GetUColumn(N - 1, normal.data());
            constant = Dot(normal, p[0]);
            origin = constant * normal;
        }

        friend class UnitTestHyperplane;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(Hyperplane<T, N> const& hyperplane0, Hyperplane<T, N> const& hyperplane1)
    {
        return hyperplane0.normal == hyperplane1.normal
            && hyperplane0.origin == hyperplane1.origin
            && hyperplane0.constant == hyperplane1.constant;
    }

    template <typename T, size_t N>
    bool operator!=(Hyperplane<T, N> const& hyperplane0, Hyperplane<T, N> const& hyperplane1)
    {
        return !operator==(hyperplane0, hyperplane1);
    }

    template <typename T, size_t N>
    bool operator<(Hyperplane<T, N> const& hyperplane0, Hyperplane<T, N> const& hyperplane1)
    {
        if (hyperplane0.normal < hyperplane1.normal)
        {
            return true;
        }

        if (hyperplane0.normal > hyperplane1.normal)
        {
            return false;
        }

        if (hyperplane0.origin < hyperplane1.origin)
        {
            return true;
        }

        if (hyperplane0.origin > hyperplane1.origin)
        {
            return false;
        }

        return hyperplane0.constant < hyperplane1.constant;
    }

    template <typename T, size_t N>
    bool operator<=(Hyperplane<T, N> const& hyperplane0, Hyperplane<T, N> const& hyperplane1)
    {
        return !operator<(hyperplane1, hyperplane0);
    }

    template <typename T, size_t N>
    bool operator>(Hyperplane<T, N> const& hyperplane0, Hyperplane<T, N> const& hyperplane1)
    {
        return operator<(hyperplane1, hyperplane0);
    }

    template <typename T, size_t N>
    bool operator>=(Hyperplane<T, N> const& hyperplane0, Hyperplane<T, N> const& hyperplane1)
    {
        return !operator<(hyperplane0, hyperplane1);
    }

    // Template alias for convenience.
    template <typename T> using Plane3 = Hyperplane<T, 3>;
}
