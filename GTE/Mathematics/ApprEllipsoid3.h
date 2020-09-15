// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

// The ellipsoid in general form is  X^t A X + B^t X + C = 0 where A is a
// positive definite 3x3 matrix, B is a 3x1 vector, C is a scalar, and X is a
// 3x1 vector.  Completing the square, (X-U)^t A (X-U) = U^t A U - C where
// U = -0.5 A^{-1} B.  Define M = A/(U^t A U - C).  The ellipsoid is
// (X-U)^t M (X-U) = 1.  Factor M = R^t D R where R is orthonormal and D is
// diagonal with positive diagonal terms.  The ellipsoid in factored form is
// (X-U)^t R^t D^t R (X-U) = 1.  Find the least squares fit of a set of N
// points P[0] through P[N-1].  The error return value is the least-squares
// energy function at (U,R,D).

#include <Mathematics/ContOrientedBox3.h>
#include <Mathematics/DistPointHyperellipsoid.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/MinimizeN.h>
#include <Mathematics/Rotation.h>

namespace gte
{
    template <typename Real>
    class ApprEllipsoid3
    {
    public:
        Real operator()(int numPoints, Vector3<Real> const* points,
            Vector3<Real>& center, Matrix3x3<Real>& rotate, Real diagonal[3]) const
        {
            // Energy function is E : R^9 -> R where
            //   V = (V0,V1,V2,V3,V4,V5,V6,V7,V8)
            //     = (D[0],D[1],D[2],U[0],U,y,U[2],A0,A1,A2). 
            std::function<Real(Real const*)> energy =
                [numPoints, points](Real const* input)
            {
                return Energy(numPoints, points, input);
            };

            MinimizeN<Real> minimizer(9, energy, 8, 8, 32);

            // The initial guess for the minimizer is based on an oriented box
            // that contains the points.
            OrientedBox3<Real> box;
            GetContainer(numPoints, points, box);
            center = box.center;
            for (int i = 0; i < 3; ++i)
            {
                rotate.SetRow(i, box.axis[i]);
                diagonal[i] = box.extent[i];
            }

            Real angle[3];
            MatrixToAngles(rotate, angle);

            Real extent[3] =
            {
                diagonal[0] * std::fabs(rotate(0, 0)) +
                diagonal[1] * std::fabs(rotate(0, 1)) +
                diagonal[2] * std::fabs(rotate(0, 2)),

                diagonal[0] * std::fabs(rotate(1, 0)) +
                diagonal[1] * std::fabs(rotate(1, 1)) +
                diagonal[2] * std::fabs(rotate(1, 2)),

                diagonal[0] * std::fabs(rotate(2, 0)) +
                diagonal[1] * std::fabs(rotate(2, 1)) +
                diagonal[2] * std::fabs(rotate(2, 2))
            };

            Real v0[9] =
            {
                (Real)0.5 * diagonal[0],
                (Real)0.5 * diagonal[1],
                (Real)0.5 * diagonal[2],
                center[0] - extent[0],
                center[1] - extent[1],
                center[2] - extent[2],
                -(Real)GTE_C_PI,
                (Real)0,
                (Real)0
            };

            Real v1[9] =
            {
                (Real)2 * diagonal[0],
                (Real)2 * diagonal[1],
                (Real)2 * diagonal[2],
                center[0] + extent[0],
                center[1] + extent[1],
                center[2] + extent[2],
                (Real)GTE_C_PI,
                (Real)GTE_C_PI,
                (Real)GTE_C_PI
            };

            Real vInitial[9] =
            {
                diagonal[0],
                diagonal[1],
                diagonal[2],
                center[0],
                center[1],
                center[2],
                angle[0],
                angle[1],
                angle[2]
            };

            Real vMin[9], error;
            minimizer.GetMinimum(v0, v1, vInitial, vMin, error);

            diagonal[0] = vMin[0];
            diagonal[1] = vMin[1];
            diagonal[2] = vMin[2];
            center[0] = vMin[3];
            center[1] = vMin[4];
            center[2] = vMin[5];
            AnglesToMatrix(&vMin[6], rotate);

            return error;
        }

    private:
        static void MatrixToAngles(Matrix3x3<Real> const& rotate, Real angle[3])
        {
            // rotation axis = (cos(a0)sin(a1),sin(a0)sin(a1),cos(a1))
            // a0 in [-pi,pi], a1 in [0,pi], a2 in [0,pi]

            Real const zero = (Real)0;
            Real const one = (Real)1;
            AxisAngle<3, Real> aa = Rotation<3, Real>(rotate);

            if (-one < aa.axis[2])
            {
                if (aa.axis[2] < one)
                {
                    angle[0] = std::atan2(aa.axis[1], aa.axis[0]);
                    angle[1] = std::acos(aa.axis[2]);
                }
                else
                {
                    angle[0] = zero;
                    angle[1] = zero;
                }
            }
            else
            {
                angle[0] = zero;
                angle[1] = (Real)GTE_C_PI;
            }
        }

        static void AnglesToMatrix(Real const angle[3], Matrix3x3<Real>& rotate)
        {
            // rotation axis = (cos(a0)sin(a1),sin(a0)sin(a1),cos(a1))
            // a0 in [-pi,pi], a1 in [0,pi], a2 in [0,pi]

            Real cs0 = std::cos(angle[0]);
            Real sn0 = std::sin(angle[0]);
            Real cs1 = std::cos(angle[1]);
            Real sn1 = std::sin(angle[1]);
            AxisAngle<3, Real> aa;
            aa.axis = { cs0 * sn1, sn0 * sn1, cs1 };
            aa.angle = angle[2];
            rotate = Rotation<3, Real>(aa);
        }

        static Real Energy(int numPoints, Vector3<Real> const* points, Real const* input)
        {
            // Build rotation matrix.
            Matrix3x3<Real> rotate;
            AnglesToMatrix(&input[6], rotate);

            // Uniformly scale the extents to keep reasonable floating point values
            // in the distance calculations.
            Real maxValue = std::max(std::max(input[0], input[1]), input[2]);
            Real invMax = (Real)1 / maxValue;
            Ellipsoid3<Real> ellipsoid(Vector3<Real>::Zero(), { Vector3<Real>::Unit(0),
                Vector3<Real>::Unit(1), Vector3<Real>::Unit(2) }, { invMax * input[0],
                invMax * input[1], invMax * input[2] });

            // Transform the points to the coordinate system of center C and columns
            // of rotation R.
            DCPQuery<Real, Vector3<Real>, Ellipsoid3<Real>> peQuery;
            Real energy = (Real)0;
            for (int i = 0; i < numPoints; ++i)
            {
                Vector3<Real> diff = points[i] -
                    Vector3<Real>{ input[3], input[4], input[5] };

                Vector3<Real> prod = invMax * (diff * rotate);
                Real dist = peQuery(prod, ellipsoid).distance;
                energy += maxValue * dist;
            }

            return energy;
        }
    };
}
