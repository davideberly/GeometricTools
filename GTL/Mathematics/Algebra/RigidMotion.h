// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Conversions among representations of rotations and rigid motions. Rotation
// axes must be unit length. The angles are in units of radians.

#include <GTL/Mathematics/Algebra/AxisAngle.h>
#include <GTL/Mathematics/Algebra/DualQuaternion.h>
#include <GTL/Mathematics/Algebra/EulerAngles.h>
#include <GTL/Mathematics/Algebra/Matrix.h>
#include <GTL/Mathematics/Algebra/Quaternion.h>
#include <algorithm>
#include <cmath>

namespace gtl
{
    template <typename T>
    class RigidMotion
    {
    public:
        // ROTATION CONVERSIONS
        using value_type = T;

        // Create a rotation matrix from an angle (in radians). The matrix is
        // R(t) = {{c,-s},{s,c}}, where c = cos(angle), s = sin(angle), and the
        // inner-brace pairs are rows of the matrix.
        static void Convert(T const& angle, Matrix2x2<T>& r)
        {
            T cs = std::cos(angle);
            T sn = std::sin(angle);
            r(0, 0) = cs;
            r(0, 1) = -sn;
            r(1, 0) = sn;
            r(1, 1) = cs;
        }

        // Get the angle (radians) from a rotation matrix. The caller is
        // responsible for ensuring the matrix is a rotation.
        static void Convert(Matrix2x2<T> const& r, T& angle)
        {
            angle = std::atan2(r(1, 0), r(0, 0));
        }

        // Convert a rotation matrix to a quaternion.
        //   x^2 = (+r00 - r11 - r22 + 1)/4
        //   y^2 = (-r00 + r11 - r22 + 1)/4
        //   z^2 = (-r00 - r11 + r22 + 1)/4
        //   w^2 = (+r00 + r11 + r22 + 1)/4
        //   x^2 + y^2 = (1 - r22)/2
        //   z^2 + w^2 = (1 + r22)/2
        //   y^2 - x^2 = (r11 - r00)/2
        //   w^2 - z^2 = (r11 + r00)/2
        //   x*y = (r01 + r10)/4
        //   x*z = (r02 + r20)/4
        //   y*z = (r12 + r21)/4
        //   x*w = (r21 - r12)/4
        //   y*w = (r02 - r20)/4
        //   z*w = (r10 - r01)/4
        // If Q is the 4x1 column vector that represents the tuple (x,y,z,w),
        // the previous equations imply
        //         +-                  -+
        //         | x*x  x*y  x*z  x*w |
        // Q*Q^T = | y*x  y*y  y*z  y*w |
        //         | z*x  z*y  z*z  z*w |
        //         | w*x  w*y  w*z  w*w |
        //         +-                  -+
        // The code extracts the row of maximum length, normalizing it to
        // obtain the result q.
        static void Convert(Matrix3x3<T> const& r, Quaternion<T>& q)
        {
            T r22 = r(2, 2);
            if (r22 <= C_<T>(0))  // x^2 + y^2 >= z^2 + w^2
            {
                T dif10 = r(1, 1) - r(0, 0);
                T omr22 = C_<T>(1) - r22;
                if (dif10 <= C_<T>(0))  // x^2 >= y^2
                {
                    T fourXSqr = omr22 - dif10;
                    T inv4x = C_<T>(1, 2) / std::sqrt(fourXSqr);
                    q[0] = fourXSqr * inv4x;
                    q[1] = (r(0, 1) + r(1, 0)) * inv4x;
                    q[2] = (r(0, 2) + r(2, 0)) * inv4x;
                    q[3] = (r(2, 1) - r(1, 2)) * inv4x;
                }
                else  // y^2 >= x^2
                {
                    T fourYSqr = omr22 + dif10;
                    T inv4y = C_<T>(1, 2) / std::sqrt(fourYSqr);
                    q[0] = (r(0, 1) + r(1, 0)) * inv4y;
                    q[1] = fourYSqr * inv4y;
                    q[2] = (r(1, 2) + r(2, 1)) * inv4y;
                    q[3] = (r(0, 2) - r(2, 0)) * inv4y;
                }
            }
            else  // z^2 + w^2 >= x^2 + y^2
            {
                T sum10 = r(1, 1) + r(0, 0);
                T opr22 = C_<T>(1) + r22;
                if (sum10 <= C_<T>(0))  // z^2 >= w^2
                {
                    T fourZSqr = opr22 - sum10;
                    T inv4z = C_<T>(1, 2) / std::sqrt(fourZSqr);
                    q[0] = (r(0, 2) + r(2, 0)) * inv4z;
                    q[1] = (r(1, 2) + r(2, 1)) * inv4z;
                    q[2] = fourZSqr * inv4z;
                    q[3] = (r(1, 0) - r(0, 1)) * inv4z;
                }
                else  // w^2 >= z^2
                {
                    T fourWSqr = opr22 + sum10;
                    T inv4w = C_<T>(1, 2) / std::sqrt(fourWSqr);
                    q[0] = (r(2, 1) - r(1, 2)) * inv4w;
                    q[1] = (r(0, 2) - r(2, 0)) * inv4w;
                    q[2] = (r(1, 0) - r(0, 1)) * inv4w;
                    q[3] = fourWSqr*inv4w;
                }
            }
        }

        // Convert a quaterion q = x*i + y*j + z*k + w to a rotation matrix.
        //     +-           -+   +-                                     -+
        // R = | r00 r01 r02 | = | 1-2y^2-2z^2  2(xy-zw)     2(xz+yw)    |
        //     | r10 r11 r12 |   | 2(xy+zw)     1-2x^2-2z^2  2(yz-xw)    |
        //     | r20 r21 r22 |   | 2(xz-yw)     2(yz+xw)     1-2x^2-2y^2 |
        //     +-           -+   +-                                     -+
        static void Convert(Quaternion<T> const& q, Matrix3x3<T>& r)
        {
            T twoX = C_<T>(2) * q[0];
            T twoY = C_<T>(2) * q[1];
            T twoZ = C_<T>(2) * q[2];
            T twoXX = twoX * q[0];
            T twoXY = twoX * q[1];
            T twoXZ = twoX * q[2];
            T twoXW = twoX * q[3];
            T twoYY = twoY * q[1];
            T twoYZ = twoY * q[2];
            T twoYW = twoY * q[3];
            T twoZZ = twoZ * q[2];
            T twoZW = twoZ * q[3];
            r(0, 0) = C_<T>(1) - twoYY - twoZZ;
            r(0, 1) = twoXY - twoZW;
            r(0, 2) = twoXZ + twoYW;
            r(1, 0) = twoXY + twoZW;
            r(1, 1) = C_<T>(1) - twoXX - twoZZ;
            r(1, 2) = twoYZ - twoXW;
            r(2, 0) = twoXZ - twoYW;
            r(2, 1) = twoYZ + twoXW;
            r(2, 2) = C_<T>(1) - twoXX - twoYY;
        }

        // Convert a rotation matrix to an axis-angle pair. Let (x0,x1,x2) be
        // the axis and let t be an angle of rotation. The rotation matrix is
        // R = I + sin(t)*S + (1-cos(t))*S^2, where I is the identity and S =
        // {{0,-x2,x1},{x2,0,-x0},{-x1,x0,0}} where the inner-brace triples
        // are the rows of the matrix. If t > 0, R represents a
        // counterclockwise rotation. It may be shown that cos(t) =
        // (trace(R)-1)/2 and R - Transpose(R) = 2*sin(t)*S. As long as
        // sin(t) is not zero, we may solve for S in the second equation,
        // which produces the axis direction U = (S21,S02,S10). When t = 0,
        // the rotation is the identity, in which case any axis direction is
        // valid; we choose (1,0,0). When t = pi, it must be that
        // R - Transpose(R) = 0, which prevents us from extracting the axis.
        // Instead, note that (R+I)/2 = I+S^2 = U*U^T, where U is a
        // unit-length axis direction.
        static void Convert(Matrix3x3<T> const& r, AxisAngle<T>& a)
        {
            T trace = r(0, 0) + r(1, 1) + r(2, 2);
            T cs = C_<T>(1, 2) * (trace - C_<T>(1));
            cs = std::max(std::min(cs, C_<T>(1)), -C_<T>(1));
            a.angle = std::acos(cs);  // The angle is in [0,pi].

            if (a.angle > C_<T>(0))
            {
                if (a.angle <= C_PI_DIV_2<T>)
                {
                    // The angle is in (0,pi/2].
                    a.axis[0] = r(2, 1) - r(1, 2);
                    a.axis[1] = r(0, 2) - r(2, 0);
                    a.axis[2] = r(1, 0) - r(0, 1);
                    Normalize(a.axis);
                }
                else
                {
                    // The angle is in (pi/2,pi), in which case
                    //   R + R^T = 2*cos(angle) + 2*(1-cos(angle))*U*U^T
                    // where U = (u0,u1,u2) is the unit-length direction of
                    // the rotation axis. Therefore,
                    //   U*U^T = ((R+R^T-2*cos(angle)*I)/(2*(1-cos(angle)))
                    // The angle constraint guarantees that 1-cos(angle) is
                    // in [1,2], so numerical rounding errors caused by the
                    // division are not problematic. Define
                    //   M = (R + R^T) / 2 - cos(angle) * I = K * U * U^T
                    // for some scalar K. The largest diagonal entry of M is
                    // the largest diagonal entry of R. Determine which is the
                    // largest diagonal entry and normalize the corresponding
                    // row of M to obtain U.
                    if (r(0, 0) >= r(1, 1))
                    {
                        if (r(0, 0) >= r(2, 2))
                        {
                            // r00 is maximum diagonal term
                            a.axis[0] = r(0, 0) - cs;
                            a.axis[1] = C_<T>(1, 2) * (r(0, 1) + r(1, 0));
                            a.axis[2] = C_<T>(1, 2) * (r(0, 2) + r(2, 0));
                        }
                        else
                        {
                            // r22 is maximum diagonal term
                            a.axis[0] = C_<T>(1, 2) * (r(2, 0) + r(0, 2));
                            a.axis[1] = C_<T>(1, 2) * (r(2, 1) + r(1, 2));
                            a.axis[2] = r(2, 2) - cs;
                        }
                    }
                    else
                    {
                        if (r(1, 1) >= r(2, 2))
                        {
                            // r11 is maximum diagonal term
                            a.axis[0] = C_<T>(1, 2) * (r(1, 0) + r(0, 1));
                            a.axis[1] = r(1, 1) - cs;
                            a.axis[2] = C_<T>(1, 2) * (r(1, 2) + r(2, 1));
                        }
                        else
                        {
                            // r22 is maximum diagonal term
                            a.axis[0] = C_<T>(1, 2) * (r(2, 0) + r(0, 2));
                            a.axis[1] = C_<T>(1, 2) * (r(2, 1) + r(1, 2));
                            a.axis[2] = r(2, 2) - cs;
                        }
                    }
                    Normalize(a.axis);

                    // The equation for U*U^T has two unit-length vector
                    // solutions (U or -U). Determine which should be used.
                    // When the angle is exactly pi, rotation(U,pi) and
                    // rotation(-U,pi) are the same, but this is not true for
                    // angles smaller than pi.
                    T omcs = C_<T>(1) - cs;
                    T sn = a.axis[0] * r(2, 1) + a.axis[1] * r(0, 2) + a.axis[2] * r(1, 0)
                        - C_<T>(3) * omcs * a.axis[0] * a.axis[1] * a.axis[2];
                    if (sn < C_<T>(0))
                    {
                        // The angle from std::acos is in [0,pi], so
                        // sin(angle) needs to be nonnegative. When it is not,
                        // the axis direction must be negated.
                        a.axis = -a.axis;
                    }
                }
            }
            else
            {
                // The angle is 0 and the matrix is the identity. Any axis will
                // work, so choose the x-axis axis.
                MakeBasis(0, a.axis);
            }
        }

        // Convert an axis-angle pair to a rotation matrix. Assuming
        // (x0,x1,x2) is for a right-handed world (x0 to right, x1 up, x2 out
        // of the plane of the page), a positive angle corresponds to a
        // counterclockwise rotation from the perspective of an observer
        // looking at the origin of the plane of rotation and having view
        // direction the negative of the axis direction. The coordinate-axis
        // rotations are the following, where unit(0) = (1,0,0), unit(1) =
        // (0,1,0), unit(2) = (0,0,1),
        //   R(unit(0),t) = {{ 1, 0, 0}, { 0, c,-s}, { 0, s, c}}
        //   R(unit(1),t) = {{ c, 0, s}, { 0, 1, 0}, {-s, 0, c}}
        //   R(unit(2),t) = {{ c,-s, 0}, { s, c, 0}, { 0, 0, 1}}
        // where c = cos(t), s = sin(t), and the inner-brace triples are rows
        // of the matrix. The general matrix is
        //     +-                                                          -+
        // R = | (1-c)*x0^2  + c     (1-c)*x0*x1 - s*x2  (1-c)*x0*x2 + s*x1 |
        //     | (1-c)*x0*x1 + s*x2  (1-c)*x1^2  + c     (1-c)*x1*x2 - s*x0 |
        //     | (1-c)*x0*x2 - s*x1  (1-c)*x1*x2 + s*x0  (1-c)*x2^2  + c    |
        //     +-                                                          -+
        static void Convert(AxisAngle<T> const& a, Matrix3x3<T>& r)
        {
            T cs = std::cos(a.angle);
            T sn = std::sin(a.angle);
            T oneMinusCos = C_<T>(1) - cs;
            T x0sqr = a.axis[0] * a.axis[0];
            T x1sqr = a.axis[1] * a.axis[1];
            T x2sqr = a.axis[2] * a.axis[2];
            T x0x1m = a.axis[0] * a.axis[1] * oneMinusCos;
            T x0x2m = a.axis[0] * a.axis[2] * oneMinusCos;
            T x1x2m = a.axis[1] * a.axis[2] * oneMinusCos;
            T x0Sin = a.axis[0] * sn;
            T x1Sin = a.axis[1] * sn;
            T x2Sin = a.axis[2] * sn;
            r(0, 0) = x0sqr * oneMinusCos + cs;
            r(0, 1) = x0x1m - x2Sin;
            r(0, 2) = x0x2m + x1Sin;
            r(1, 0) = x0x1m + x2Sin;
            r(1, 1) = x1sqr * oneMinusCos + cs;
            r(1, 2) = x1x2m - x0Sin;
            r(2, 0) = x0x2m - x1Sin;
            r(2, 1) = x1x2m + x0Sin;
            r(2, 2) = x2sqr * oneMinusCos + cs;
        }

        // Convert a rotation matrix to Euler angles. NOTE: You must set the
        // e.axis[] elements before calling this function; this specifies the
        // order you want for the coordinate-axis rotations. Factorization
        // into Euler angles is not necessarily unique. If the result is
        // nonUniqueSum, then multiple solutions occur because
        // angle[2]+angle[0] is constant. If the result is nonUniqueDifference,
        // then multiple solutions occur because angle[2]-angle[0] is
        // constant. In either case, the function returns angle[0]=0.
        static void Convert(Matrix3x3<T> const& r, EulerAngles<T>& e)
        {
            if (0 <= e.axis[0] && e.axis[0] < 3 &&
                0 <= e.axis[1] && e.axis[1] < 3 &&
                0 <= e.axis[2] && e.axis[2] < 3 &&
                e.axis[1] != e.axis[0] &&
                e.axis[1] != e.axis[2])
            {
                if (e.axis[0] != e.axis[2])
                {
                    // Map (0,1,2), (1,2,0), and (2,0,1) to +1.
                    // Map (0,2,1), (2,1,0), and (1,0,2) to -1.
                    size_t parity = (((e.axis[2] | (e.axis[1] << 2)) >> e.axis[0]) & 1);
                    T const sgn = (parity & 1 ? -C_<T>(1) : C_<T>(1));

                    if (r(e.axis[2], e.axis[0]) < C_<T>(1))
                    {
                        if (r(e.axis[2], e.axis[0]) > -C_<T>(1))
                        {
                            e.angle[2] = std::atan2(sgn * r(e.axis[1], e.axis[0]), r(e.axis[0], e.axis[0]));
                            e.angle[1] = std::asin(-sgn * r(e.axis[2], e.axis[0]));
                            e.angle[0] = std::atan2(sgn * r(e.axis[2], e.axis[1]), r(e.axis[2], e.axis[2]));
                            e.result = EulerAngles<T>::unique;
                        }
                        else
                        {
                            e.angle[2] = C_<T>(0);
                            e.angle[1] = sgn * C_PI_DIV_2<T>;
                            e.angle[0] = std::atan2(-sgn * r(e.axis[1], e.axis[2]), r(e.axis[1], e.axis[1]));
                            e.result = EulerAngles<T>::nonUniqueDifference;
                        }
                    }
                    else
                    {
                        e.angle[2] = C_<T>(0);
                        e.angle[1] = -sgn * C_PI_DIV_2<T>;
                        e.angle[0] = std::atan2(-sgn * r(e.axis[1], e.axis[2]), r(e.axis[1], e.axis[1]));
                        e.result = EulerAngles<T>::nonUniqueSum;
                    }
                }
                else
                {
                    // Map (0,2,0), (1,0,1), and (2,1,2) to +1.
                    // Map (0,1,0), (1,2,1), and (2,0,2) to -1.
                    size_t b0 = 3 - e.axis[1] - e.axis[2];
                    size_t parity = (((b0 | (e.axis[1] << 2)) >> e.axis[2]) & 1);
                    T const sgn = (parity & 1 ? C_<T>(1) : -C_<T>(1));

                    if (r(e.axis[2], e.axis[2]) < C_<T>(1))
                    {
                        if (r(e.axis[2], e.axis[2]) > -C_<T>(1))
                        {
                            e.angle[2] = std::atan2(r(e.axis[1], e.axis[2]), sgn * r(b0, e.axis[2]));
                            e.angle[1] = std::acos(r(e.axis[2], e.axis[2]));
                            e.angle[0] = std::atan2(r(e.axis[2], e.axis[1]), -sgn * r(e.axis[2], b0));
                            e.result = EulerAngles<T>::unique;
                        }
                        else
                        {
                            e.angle[2] = C_<T>(0);
                            e.angle[1] = C_PI<T>;
                            e.angle[0] = std::atan2(sgn * r(e.axis[1], b0), r(e.axis[1], e.axis[1]));
                            e.result = EulerAngles<T>::nonUniqueDifference;
                        }
                    }
                    else
                    {
                        e.angle[2] = C_<T>(0);
                        e.angle[1] = C_<T>(0);
                        e.angle[0] = std::atan2(sgn * r(e.axis[1], b0), r(e.axis[1], e.axis[1]));
                        e.result = EulerAngles<T>::nonUniqueSum;
                    }
                }
            }
            else
            {
                // Invalid axes.
                e.angle[0] = C_<T>(0);
                e.angle[1] = C_<T>(0);
                e.angle[2] = C_<T>(0);
                e.result = EulerAngles<T>::invalid;
            }
        }

        // Convert Euler angles to a rotation matrix. The three integer
        // inputs are in {0,1,2} and correspond to world directions unit(0)
        // = (1,0,0), unit(1) = (0,1,0), or unit(2) = (0,0,1). The triples
        // (N0,N1,N2) must be in the following set,
        //   {(0,1,2),(0,2,1),(1,0,2),(1,2,0),(2,0,1),(2,1,0),
        //    (0,1,0),(0,2,0),(1,0,1),(1,2,1),(2,0,2),(2,1,2)}
        // The rotation matrix is
        //     R(unit(N2),angleN2) * R(unit(N1),angleN1) * R(unit(N0),angleN0)
        static void Convert(EulerAngles<T> const& e, Matrix3x3<T>& r)
        {
            if (0 <= e.axis[0] && e.axis[0] < 3 &&
                0 <= e.axis[1] && e.axis[1] < 3 &&
                0 <= e.axis[2] && e.axis[2] < 3 &&
                e.axis[1] != e.axis[0] &&
                e.axis[1] != e.axis[2])
            {
                Vector3<T> unit0{}, unit1{}, unit2{};
                MakeBasis(e.axis[0], unit0);
                MakeBasis(e.axis[1], unit1);
                MakeBasis(e.axis[2], unit2);

                Matrix3x3<T> r0{}, r1{}, r2{};
                Convert(AxisAngle<T>(unit0, e.angle[0]), r0);
                Convert(AxisAngle<T>(unit1, e.angle[1]), r1);
                Convert(AxisAngle<T>(unit2, e.angle[2]), r2);
                r = r2 * r1 * r0;
            }
            else
            {
                // Invalid axes.
                MakeIdentity(r);
            }
        }

        // Convert a quaternion to an axis-angle pair, where
        // q = sin(angle/2)*(axis[0]*i + axis[1]*j + axis[2]*k) + cos(angle/2)
        static void Convert(Quaternion<T> const& q, AxisAngle<T>& a)
        {
            T length = std::sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2]);
            if (length > C_<T>(0))
            {
                a.axis[0] = q[0] / length;
                a.axis[1] = q[1] / length;
                a.axis[2] = q[2] / length;
                T cs = std::max(std::min(q[3], C_<T>(1)), -C_<T>(1));
                a.angle = C_<T>(2) * std::acos(cs);
            }
            else
            {
                // The angle is 0 (modulo 2*pi). Any axis will work, so choose
                // the x-axis.
                MakeBasis(0, a.axis);
                a.angle = C_<T>(0);
            }
        }

        // Convert an axis-angle pair to a quaternion, where
        // q = sin(angle/2)*(axis[0]*i + axis[1]*j + axis[2]*k) + cos(angle/2)
        static void Convert(AxisAngle<T> const& a, Quaternion<T>& q)
        {
            T halfAngle = C_<T>(1, 2) * a.angle;
            T sn = std::sin(halfAngle);
            q[0] = sn * a.axis[0];
            q[1] = sn * a.axis[1];
            q[2] = sn * a.axis[2];
            q[3] = std::cos(halfAngle);
        }

        // Convert a quaternion to Euler angles. The quaternion is converted
        // to a matrix which is then converted to Euler angles. NOTE: You
        // must set the e.axis[] elements before calling this function; this
        // specifies the order you want for the coordinate-axis rotations.
        static void Convert(Quaternion<T> const& q, EulerAngles<T>& e)
        {
            Matrix3x3<T> r{};
            Convert(q, r);
            Convert(r, e);
        }

        // Convert Euler angles to a quaternion. The Euler angles are
        // converted to a matrix which is then converted to a quaternion.
        static void Convert(EulerAngles<T> const& e, Quaternion<T>& q)
        {
            Matrix3x3<T> r{};
            Convert(e, r);
            Convert(r, q);
        }

        // Convert an axis-angle pair to Euler angles. The axis-angle pair is
        // converted to a quaternion which is then converted to Euler angles.
        // NOTE: You must set the e.axis[] elements before calling this
        // function; this specifies the order you want for the coordinate-axis
        // rotations.
        static void Convert(AxisAngle<T> const& a, EulerAngles<T>& e)
        {
            Quaternion<T> q{};
            Convert(a, q);
            Convert(q, e);
        }

        // Convert Euler angles to an axis-angle pair. The Euler angles are
        // converted to a quaternion which is then converted to an axis-angle
        // pair.
        static void Convert(EulerAngles<T> const& e, AxisAngle<T>& a)
        {
            Quaternion<T> q{};
            Convert(e, q);
            Convert(q, a);
        }


        // RIGID MOTION CONVERSIONS (rotations and translations)

        // Convert a dual quaternion to a rotation (as a quaternion) and a
        // translation.
        static void Convert(DualQuaternion<T> const& d, Quaternion<T>& q,
            Vector3<T>& t)
        {
            q = d[0];
            Quaternion<T> product = C_<T>(2) * d[1] * Conjugate(q);
            t = { product[0], product[1], product[2] };
        }

        // Convert a dual quaternion to a rotation (as a matrix) and a
        // translation.
        static void Convert(DualQuaternion<T> const& d, Matrix3x3<T>& r,
            Vector3<T>& t)
        {
            Quaternion<T> q{};
            Convert(d, q, t);
            Convert(q, r);
        }

        // Convert a rotation (as a quaternion) and a translation to a dual
        // quaternion.
        static void Convert(Quaternion<T> const& q, Vector3<T> const& t,
            DualQuaternion<T>& d)
        {
            d[0] = q;
            d[1] = C_<T>(1, 2) *
                Quaternion<T>(t[0], t[1], t[2], C_<T>(0)) * q;
        }

        // Convert a rotation (as a matrix) and a translation to a dual
        // quaternion.
        static void Convert(Matrix3x3<T> const& r, Vector3<T> const& t,
            DualQuaternion<T>& d)
        {
            Quaternion<T> q{};
            Convert(r, q);
            Convert(q, t, d);
        }


        // MIXED-DIMENSION CONVERSIONS. The caller is responsible for ensuring
        // the input 3x3 matrices are rotation and the input 4x4 matrices are
        // homogeneous that represent a rigid motion. The outputs use the
        // convention that R*U = V for 3x3 rotation matrix R and 3x1 vectors
        // U and V. They use the convention that H*U = V for 4x4 homogeneous
        // matrix H and 4x1 homogeneous vectors U and V.
        static void Convert(Matrix3x3<T> const& r, Vector3<T> const& t,
            Matrix4x4<T>& h)
        {
            for (size_t row = 0; row < 3; ++row)
            {
                for (size_t col = 0; col < 3; ++col)
                {
                    h(row, col) = r(row, col);
                }
                h(row, 3) = t[row];
            }
            h(3, 0) = C_<T>(0);
            h(3, 1) = C_<T>(0);
            h(3, 2) = C_<T>(0);
            h(3, 3) = C_<T>(1);
        }

        static void Convert(Matrix4x4<T> const& h, Matrix3x3<T>& r,
            Vector3<T>& t)
        {
            for (size_t row = 0; row < 3; ++row)
            {
                for (size_t col = 0; col < 3; ++col)
                {
                    r(row, col) = h(row, col);
                }
                t[row] = h(row, 3);
            }
        }

    private:
        friend class UnitTestRigidMotion;
    };
}
