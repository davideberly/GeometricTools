// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/SingularValueDecomposition.h>

// The plane is represented as Dot(U,X) = c where U is a unit-length normal
// vector, c is the plane constant, and X is any point on the plane.  The user
// must ensure that the normal vector is unit length.

namespace gte
{
    template <int N, typename Real>
    class Hyperplane
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // normal to (0,...,0,1) and the constant to zero (plane z = 0).
        Hyperplane()
            :
            constant((Real)0)
        {
            normal.MakeUnit(N - 1);
        }

        // Specify U and c directly.
        Hyperplane(Vector<N, Real> const& inNormal, Real inConstant)
            :
            normal(inNormal),
            constant(inConstant)
        {
        }

        // U is specified, c = Dot(U,p) where p is a point on the hyperplane.
        Hyperplane(Vector<N, Real> const& inNormal, Vector<N, Real> const& p)
            :
            normal(inNormal),
            constant(Dot(inNormal, p))
        {
        }

        // U is a unit-length vector in the orthogonal complement of the set
        // {p[1]-p[0],...,p[n-1]-p[0]} and c = Dot(U,p[0]), where the p[i] are
        // pointson the hyperplane.
        Hyperplane(std::array<Vector<N, Real>, N> const& p)
        {
            Matrix<N, N - 1, Real> edge;
            for (int i = 0; i < N - 1; ++i)
            {
                edge.SetCol(i, p[i + 1] - p[0]);
            }

            // Compute the 1-dimensional orthogonal complement of the edges of
            // the simplex formed by the points p[].
            SingularValueDecomposition<Real> svd(N, N - 1, 32);
            svd.Solve(&edge[0], -1);
            svd.GetUColumn(N - 1, &normal[0]);

            constant = Dot(normal, p[0]);
        }

        // Public member access.
        Vector<N, Real> normal;
        Real constant;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Hyperplane const& hyperplane) const
        {
            return normal == hyperplane.normal && constant == hyperplane.constant;
        }

        bool operator!=(Hyperplane const& hyperplane) const
        {
            return !operator==(hyperplane);
        }

        bool operator< (Hyperplane const& hyperplane) const
        {
            if (normal < hyperplane.normal)
            {
                return true;
            }

            if (normal > hyperplane.normal)
            {
                return false;
            }

            return constant < hyperplane.constant;
        }

        bool operator<=(Hyperplane const& hyperplane) const
        {
            return !hyperplane.operator<(*this);
        }

        bool operator> (Hyperplane const& hyperplane) const
        {
            return hyperplane.operator<(*this);
        }

        bool operator>=(Hyperplane const& hyperplane) const
        {
            return !operator<(hyperplane);
        }
    };

    // Template alias for convenience.
    template <typename Real>
    using Plane3 = Hyperplane<3, Real>;
}
