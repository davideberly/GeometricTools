// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/Matrix.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/SymmetricEigensolver.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    // The quadratic fit is
    //   0 = C[0] + C[1]*x + C[2]*y + C[3]*x^2 + C[4]*x*y + C[5]*y^2
    // which has one degree of freedom in the coefficients. Eliminate the
    // degree of freedom by minimizing the quadratic form E(C) = C^T M C
    // subject to Length(C) = 1 with M = (sum_i V[i])(sum_i V[i])^T where
    //   V = (1, x, y, x^2, x*y, y^2)
    // The minimum value is the smallest eigenvalue of M and C is a
    // corresponding unit length eigenvector.
    //
    // Input:
    //   n = number of points to fit
    //   P[0..n-1] = array of points to fit
    //
    // Output:
    //   C[0..5] = coefficients of quadratic fit (the eigenvector)
    //   return value of function is nonnegative and a measure of the fit
    //   (the minimum eigenvalue; 0 = exact fit, positive otherwise)
    //
    // Canonical forms. The quadratic equation can be factored into
    // P^T A P + B^T P + K = 0 where P = (x,y), K = C[0],B = (C[1],C[2])
    // and A is a 2x2 symmetric matrix with A00 = C[3], A01 = C[4]/2 and
    // A11 = C[5]. Using an eigendecomposition, matrix A = R^T D R where
    // R is orthogonal and D is diagonal. Define V = R*P = (v0,v1),
    // E = R*B = (e0,e1), D = diag(d0,d1) and f = K to obtain
    //   d0 v0^2 + d1 v1^2 + e0 v0 + e1 v1 + f = 0
    // The classification depends on the signs of the d_i.

    template <typename Real>
    class ApprQuadratic2
    {
    public:
        Real operator()(int32_t numPoints, Vector2<Real> const* points,
            std::array<Real, 6>& coefficients)
        {
            Matrix<6, 6, Real> M{};  // constructor sets M to zero
            for (int32_t i = 0; i < numPoints; ++i)
            {
                Real x = points[i][0];
                Real y = points[i][1];
                Real x2 = x * x;
                Real y2 = y * y;
                Real xy = x * y;
                Real x3 = x * x2;
                Real xy2 = x * y2;
                Real x2y = x * xy;
                Real y3 = y * y2;
                Real x4 = x * x3;
                Real x2y2 = x * xy2;
                Real x3y = x * x2y;
                Real y4 = y * y3;
                Real xy3 = x * y3;

                // M(0, 0) += 1
                M(0, 1) += x;
                M(0, 2) += y;
                M(0, 3) += x2;
                M(0, 4) += xy;
                M(0, 5) += y2;

                // M(1, 1) += x2    [M(0,3)]
                // M(1, 2) += xy    [M(0,4)]
                M(1, 3) += x3;
                M(1, 4) += x2y;
                M(1, 5) += xy2;

                // M(2, 2) += y2    [M(0,5)]
                // M(2, 3) += x2y   [M(1,4)]
                // M(2, 4) += xy2   [M(1,5)]
                M(2, 5) += y3;

                M(3, 3) += x4;
                M(3, 4) += x3y;
                M(3, 5) += x2y2;

                // M(4, 4) += x2y2  [M(3,5)]
                M(4, 5) += xy3;

                M(5, 5) += y4;
            }

            Real const rNumPoints = static_cast<Real>(numPoints);
            M(0, 0) = rNumPoints;
            M(1, 1) = M(0, 3);  // x2
            M(1, 2) = M(0, 4);  // xy
            M(2, 2) = M(0, 5);  // y2
            M(2, 3) = M(1, 4);  // x2y
            M(2, 4) = M(1, 5);  // xy2
            M(4, 4) = M(3, 5);  // x2y2

            for (int32_t row = 0; row < 6; ++row)
            {
                for (int32_t col = 0; col < row; ++col)
                {
                    M(row, col) = M(col, row);
                }
            }

            for (int32_t row = 0; row < 6; ++row)
            {
                for (int32_t col = 0; col < 6; ++col)
                {
                    M(row, col) /= rNumPoints;
                }
            }

            M(0, 0) = static_cast<Real>(1);

            SymmetricEigensolver<Real> es(6, 1024);
            es.Solve(&M[0], +1);
            es.GetEigenvector(0, coefficients.data());

            // For an exact fit, numeric round-off errors might make the
            // minimum eigenvalue just slightly negative. Return the clamped
            // value because the application might rely on the return value
            // being nonnegative.
            return std::max(es.GetEigenvalue(0), static_cast<Real>(0));
        }
    };


    // If you believe your points are nearly circular, use this function. The
    // circle is of the form
    //   C'[0] + C'[1]*x + C'[2]*y + C'[3]*(x^2 + y^2) = 0
    // where Length(C') = 1. The function returns
    //   C = (C'[0] / C'[3], C'[1] / C'[3], C'[2] / C'[3])
    //     = (C[0], C[1], C[2])
    // so the fitted circle is
    //   C[0] + C[1]*x + C[2]*y + x^2 + y^2 = 0
    // The center is (xc,yc) = -(C[1],C[2])/2 and the radius is
    // r = sqrt(xc * xc + yc * yc - C[0]).

    template <typename Real>
    class ApprQuadraticCircle2
    {
    public:
        Real operator()(int32_t numPoints, Vector2<Real> const* points, Circle2<Real>& circle)
        {
            Matrix<4, 4, Real> M{};  // constructor sets M to zero
            for (int32_t i = 0; i < numPoints; ++i)
            {
                Real x = points[i][0];
                Real y = points[i][1];
                Real x2 = x * x;
                Real y2 = y * y;
                Real xy = x * y;
                Real r2 = x2 + y2;
                Real xr2 = x * r2;
                Real yr2 = y * r2;
                Real r4 = r2 * r2;

                // M(0, 0) += 1
                M(0, 1) += x;
                M(0, 2) += y;
                M(0, 3) += r2;

                M(1, 1) += x2;
                M(1, 2) += xy;
                M(1, 3) += xr2;

                M(2, 2) += y2;
                M(2, 3) += yr2;

                M(3, 3) += r4;
            }

            Real const rNumPoints = static_cast<Real>(numPoints);
            M(0, 0) = rNumPoints;

            for (int32_t row = 0; row < 4; ++row)
            {
                for (int32_t col = 0; col < row; ++col)
                {
                    M(row, col) = M(col, row);
                }
            }

            for (int32_t row = 0; row < 4; ++row)
            {
                for (int32_t col = 0; col < 4; ++col)
                {
                    M(row, col) /= rNumPoints;
                }
            }

            M(0, 0) = static_cast<Real>(1);

            SymmetricEigensolver<Real> es(4, 1024);
            es.Solve(&M[0], +1);
            Vector<4, Real> evector{};
            es.GetEigenvector(0, &evector[0]);

            std::array<Real, 3> coefficients{};
            for (int32_t row = 0; row < 3; ++row)
            {
                coefficients[row] = evector[row] / evector[3];
            }

            // Clamp the radius to nonnegative values in case rounding errors
            // cause sqrRadius to be slightly negative.
            Real const negHalf = static_cast<Real>(-0.5);
            circle.center[0] = negHalf * coefficients[1];
            circle.center[1] = negHalf * coefficients[2];
            Real sqrRadius = Dot(circle.center, circle.center) - coefficients[0];
            circle.radius = std::sqrt(std::max(sqrRadius, static_cast<Real>(0)));

            // For an exact fit, numeric round-off errors might make the
            // minimum eigenvalue just slightly negative. Return the clamped
            // value because the application might rely on the return value
            // being nonnegative.
            return std::max(es.GetEigenvalue(0), static_cast<Real>(0));
        }
    };
}
