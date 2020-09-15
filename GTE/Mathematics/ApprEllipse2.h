// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

// The ellipse in general form is  X^t A X + B^t X + C = 0 where A is a
// positive definite 2x2 matrix, B is a 2x1 vector, C is a scalar, and X is
// a 2x1 vector X.  Completing the square, (X-U)^t A (X-U) = U^t A U - C
// where U = -0.5 A^{-1} B.  Define M = A/(U^t A U - C).  The ellipse is
// (X-U)^t M (X-U) = 1.  Factor M = R^t D R where R is orthonormal and D is
// diagonal with positive diagonal terms.  The ellipse in factored form is
// (X-U)^t R^t D^t R (X-U) = 1.  Find the least squares fit of a set of N
// points P[0] through P[N-1].  The return value is the least-squares energy
// function at (U,R,D).

#include <Mathematics/ContOrientedBox2.h>
#include <Mathematics/DistPointHyperellipsoid.h>
#include <Mathematics/Matrix2x2.h>
#include <Mathematics/MinimizeN.h>

namespace gte
{
    template <typename Real>
    class ApprEllipse2
    {
    public:
        Real operator()(int numPoints, Vector2<Real> const* points,
            Vector2<Real>& center, Matrix2x2<Real>& rotate, Real diagonal[2]) const
        {
            // Energy function is E : R^5 -> R where
            //   V = (V0, V1, V2, V3, V4)
            //     = (D[0], D[1], U.x, U.y, atan2(R(0,1),R(0,0))).
            std::function<Real(Real const*)> energy =
                [numPoints, points](Real const* input)
            {
                return Energy(numPoints, points, input);
            };

            MinimizeN<Real> minimizer(5, energy, 8, 8, 32);

            // The initial guess for the minimizer is based on an oriented box
            // that contains the points.
            OrientedBox2<Real> box;
            GetContainer(numPoints, points, box);
            center = box.center;
            for (int i = 0; i < 2; ++i)
            {
                rotate.SetRow(i, box.axis[i]);
                diagonal[i] = box.extent[i];
            }

            Real angle = std::atan2(rotate(0, 1), rotate(0, 0));
            Real e0 =
                diagonal[0] * std::fabs(rotate(0, 0)) +
                diagonal[1] * std::fabs(rotate(1, 0));
            Real e1 =
                diagonal[0] * std::fabs(rotate(0, 1)) +
                diagonal[1] * std::fabs(rotate(1, 1));

            Real v0[5] =
            {
                (Real)0.5 * diagonal[0],
                (Real)0.5 * diagonal[1],
                center[0] - e0,
                center[1] - e1,
                -(Real)GTE_C_PI
            };

            Real v1[5] =
            {
                (Real)2 * diagonal[0],
                (Real)2 * diagonal[1],
                center[0] + e0,
                center[1] + e1,
                (Real)GTE_C_PI
            };

            Real vInitial[5] =
            {
                diagonal[0],
                diagonal[1],
                center[0],
                center[1],
                angle
            };

            Real vMin[5], error;
            minimizer.GetMinimum(v0, v1, vInitial, vMin, error);

            diagonal[0] = vMin[0];
            diagonal[1] = vMin[1];
            center[0] = vMin[2];
            center[1] = vMin[3];
            MakeRotation(-vMin[4], rotate);

            return error;
        }

    private:
        static Real Energy(int numPoints, Vector2<Real> const* points, Real const* input)
        {
            // Build rotation matrix.
            Matrix2x2<Real> rotate;
            MakeRotation(-input[4], rotate);

            Ellipse2<Real> ellipse(Vector2<Real>::Zero(), { Vector2<Real>::Unit(0),
                Vector2<Real>::Unit(1) }, { input[0], input[1] });

            // Transform the points to the coordinate system of center C and
            // columns of rotation R.
            DCPQuery<Real, Vector2<Real>, Ellipse2<Real>> peQuery;
            Real energy = (Real)0;
            for (int i = 0; i < numPoints; ++i)
            {
                Vector2<Real> diff = points[i] - Vector2<Real>{ input[2], input[3] };
                Vector2<Real> prod = rotate * diff;
                Real dist = peQuery(prod, ellipse).distance;
                energy += dist;
            }

            return energy;
        }
    };
}
