// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The algorithm for the perspective projection of an ellipsoid onto a
// plane is described in
// https://www.geometrictools.com/Documentation/PerspectiveProjectionEllipsoid.pdf

#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/Line.h>
#include <Mathematics/Matrix2x2.h>
#include <Mathematics/Matrix3x3.h>
#include <array>
#include <cmath>

namespace gte
{
    // Orthogonally project an ellipse onto a line. The projection interval
    // is [smin,smax] and corresponds to the line segment P + s * D, where
    // smin <= s <= smax.
    template <typename Real>
    void Project(Ellipse2<Real> const& ellipse, Line2<Real> const& line,
        Real& smin, Real& smax)
    {
        // Center of projection interval.
        Real center = Dot(line.direction, ellipse.center - line.origin);

        // Radius of projection interval.
        Real tmp[2] =
        {
            ellipse.extent[0] * Dot(line.direction, ellipse.axis[0]),
            ellipse.extent[1] * Dot(line.direction, ellipse.axis[1])
        };
        Real rSqr = tmp[0] * tmp[0] + tmp[1] * tmp[1];
        Real radius = std::sqrt(rSqr);

        smin = center - radius;
        smax = center + radius;
    }

    // Orthogonally project an ellipsoid onto a line. The projection interval
    // is [smin,smax] and corresponds to the line segment P + s * D, where
    // smin <= s <= smax.
    template <typename Real>
    void Project(Ellipsoid3<Real> const& ellipsoid, Line3<Real> const& line,
        Real& smin, Real& smax)
    {
        // Center of projection interval.
        Real center = Dot(line.direction, ellipsoid.center - line.origin);

        // Radius of projection interval.
        Real tmp[3] =
        {
            ellipsoid.extent[0] * Dot(line.direction, ellipsoid.axis[0]),
            ellipsoid.extent[1] * Dot(line.direction, ellipsoid.axis[1]),
            ellipsoid.extent[2] * Dot(line.direction, ellipsoid.axis[2])
        };
        Real rSqr = tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2];
        Real radius = std::sqrt(rSqr);

        smin = center - radius;
        smax = center + radius;
    }

    // Perspectively project an ellipsoid onto a plane.
    //
    // The ellipsoid has center C, axes A[i] and extents e[i] for 0 <= i <= 2.
    //
    // The eyepoint is E.
    // 
    // The view plane is Dot(N,X) = d, where N is a unit-length normal vector.
    // Choose U and V so that {U,V,N} is a right-handed orthonormal set; that
    // is, the vectors are unit length, mutually perpendicular and
    // N = Cross(U,V). N must be directed away from E in the sense that the
    // point K on the plane closest to E is K = E + n * N with n > 0. When
    // using a view frustum, n is the 'near' distance (from the eyepoint to
    // the view plane). The plane equation is then
    //   0 = Dot(N,X-K) = Dot(N,X) - Dot(N,E) - n = d - Dot(N,E) - n
    // so that n = d - Dot(N,E).
    //
    // The ellipsoid must be between the eyepoint and the view plane in the
    // sense that all rays from the eyepoint that intersect the ellipsoid must
    // also intersect the view plane. The precondition test is to project the
    // ellipsoid onto the line E + s * N to obtain interval [smin,smax] where
    // smin > 0. The function Project(ellipsoid, line, smin, smax) defined
    // previously in this file can be used to verify the precondition. If the
    // precondition is satisfied, the projection is an ellipse in the plane.
    // If the precondition is not satisfied, the projection is a conic section
    // that is not an ellipse or it is the empty set.
    //
    // The output is the equation of the ellipse in 2D. The projected ellipse
    // coordinates Y = (y0,y1) are the view plane coordinates of the actual 3D
    // ellipse points X = K + y0 * U + y1 * V = K + J * Y, where J is a 3x2
    // matrix whose columns are U and V.

    // Use this query when you have a single plane and a single ellipsoid to
    // project onto the plane.
    template <typename Real>
    void PerspectiveProject(Ellipsoid3<Real> const& ellipsoid, Vector3<Real> const& E,
        Plane3<Real> const& plane, Ellipse2<Real>& ellipse)
    {
        std::array<Vector3<Real>, 3> basis{};
        basis[0] = plane.normal;
        ComputeOrthogonalComplement(1, basis.data());
        auto const& N = plane.normal;
        auto const& U = basis[1];
        auto const& V = basis[2];
        Real n = plane.constant - Dot(N, E);
        PerspectiveProject(ellipsoid, E, N, U, V, n, ellipse);
    }

    // Use this query when you have a single plane and multiple ellipsoids to
    // project onto the plane. The vectors U and V and the near value n are
    // precomputed.
    template <typename Real>
    void PerspectiveProject(Ellipsoid3<Real> const& ellipsoid, Vector3<Real> const& E,
        Vector3<Real> const& N, Vector3<Real> const& U, Vector3<Real> const& V,
        Real const& n, Ellipse2<Real>& ellipse)
    {
        Real const two = static_cast<Real>(2);
        Real const four = static_cast<Real>(4);

        // Compute the coefficients for the ellipsoid represented by the
        // quadratic equation X^T*A*X + B^T*X + C = 0.
        Matrix3x3<Real> A{};
        Vector3<Real> B{};
        Real C{};
        ellipsoid.ToCoefficients(A, B, C);

        // Compute the matrix M; see PerspectiveProjectionEllipsoid.pdf for
        // the mathematical details.
        Vector3<Real> AE = A * E;
        Real qformEAE = Dot(E, AE);
        Real dotBE = Dot(B, E);
        Real quadE = four * (qformEAE + dotBE + C);
        Vector3<Real> Bp2AE = B + two * AE;
        Matrix3x3<Real> M = OuterProduct(Bp2AE, Bp2AE) - quadE * A;

        // Compute the coefficients for the projected ellipse.
        Vector3<Real> MU = M * U;
        Vector3<Real> MV = M * V;
        Vector3<Real> MN = M * N;
        Real twoN = two * n;
        Matrix2x2<Real> AOut;
        Vector2<Real> BOut;
        Real COut;
        AOut(0, 0) = Dot(U, MU);
        AOut(0, 1) = Dot(U, MV);
        AOut(1, 0) = AOut(0, 1);
        AOut(1, 1) = Dot(V, MV);
        BOut[0] = twoN * (Dot(U, MN));
        BOut[1] = twoN * (Dot(V, MN));
        COut = n * n * Dot(N, MN);

        // Extract the ellipse center, axis directions and extents.
        ellipse.FromCoefficients(AOut, BOut, COut);
    }
}
