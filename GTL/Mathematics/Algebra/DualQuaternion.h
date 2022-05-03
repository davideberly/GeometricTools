// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// DUAL NUMBERS
//
// A dual number (or dual scalar) is of the form
//   v = a + b * e
// where e is a symbol representing a nonzero quantity but e*e = 0 and where
// a and b are real numbers. Addition, subtraction, and scalar multiplication
// are performed componentwise,
//   (a0 + b0 * e) + (a1 + b1 * e) = (a0 + a1) + (b0 + b1) * e
//   (a0 + b0 * e) - (a1 + b1 * e) = (a0 - a1) + (b0 - b1) * e
//   c * (a + b * e) = (c * a) + (c * b) * e
// where c is a real number. Multiplication is
//   (a0 + b0 * e) * (a1 + b1 * e) = (a0 * a1) + (a0 * b1 + b0 * a1) * e
// Function evaluation for a differentiable function F is
//   F(a + b * e) = F(a) + [b * F'(a)] * e
// where F' is the derivative of F. This property is used for "automatic
// differentiation" of functions.
//
//
// DUAL QUATERNIONS
//
// Define p0 = x0*i + y0*j + z0*k + w0 and p1 = x1*i + y1*j + z1*k + w1.
// A dual quaternion is of the form
//   d = p0 + p1 * e
//     = (x0+x1*e) * i + (y0+y1*e) * j + (z0+z1*e) * k + (w0+w1*e)
// where the coefficients are dual numbers. This has the same appearance as
// quaternions except that the real-valued coefficients of the quaternions are
// replaced by dual-number-valued coefficients. By convention, the abstract
// symbol e commutes with i, j and k.
//
// Define the dual quaternions A = pa + qa * e and B = pb + qb * e, where
// pa, qa, pb and qb are quaternions.
//
// Scalar and vector parts.
//   Vector(A) = Vector(pa) + Vector(qa) * e
//   Scalar(A) = Scalar(pa) + Scalar(qa) * e
//   A = Vector(A) + Scalar(A)
//
// Addition, subtraction, scalar multiplication and multiplication of dual
// quaternions are
//   A + B = (pa + pb) + (qa + qb) * e
//   A - B = (pa - pb) + (qa - qb) * e
//   c * A = (c * pa) + (c * qa) * e, c is a real-valued scalar
//   A * B = (pa * pb) + (pa * qb + qa * pb) * e
// Dual quaternion multiplication is not generally commutative; that is,
// B * A and A * B are usually different, although in some special cases
// they can be the same.
//
// Conjugate operation:
//   Conjugate(A) = Conjugate(pa) + Conjugate(qa) * e = Scalar(A) - Vector(A)
//
// Scalar product of dual quaternions:
//   A = a0 + a1 * i + a2 * j + a3 * k, ai = ai0 + ai1 * e, aij real
//   B = b0 + b1 * i + b2 * j + b3 * k, bi = bi0 + bi1 * e, bij real
//   Dot(A, B) = a0 * b0 + a1 * b1 + a2 * b2 + a3 * b3
//             = (A * Conjugate(B) + B * Conjugate(A)) / 2
//             = Dot(B, A)
//             = c0 + c1 * e, c0 and c1 are real (no i, j or k terms)
//
// Vector product of dual quaternions:
//   Cross(A, B) = (A * B - B * A) / 2
//               = q0 + q1 * e, q0 and q1 have zero-valued w-terms
//
// Norm (squared magnitude):
//   Norm(A) = A * Conjugate(A) = Conjugate(A) * A = Dot(A, A)
//           = (pa * Conjugate(pa)) 
//             + (pa * Conjugate(qa) + qa * Conjugate(pa)) * e
//           = s0 + s1 * e
// where s0 and s1 are real-valued scalars.
//
// Function of a dual quaternion, which requires the function F to be
// differentiable
//   F(A) = F(pa + qa * e) = F(pa) + (qa * F'(pa)) * e
// This property is used in automatic differentiation.
//
// Length (magnitude): The squared magnitude is Norm(A) = s0 + s1 * e,
// so we must compute the square root of this,
//   Length(A) = sqrt(Norm(A)) = sqrt(s0) + (s1 / (2 * sqrt(s0))) * e
//
// A unit dual quaternion U = p + q * e has the property Norm(U) = 1.
// This happens when
//   p * Conjugate(p) = 1 [p is a unit quaternion] and
//   p * Conjugate(q) + q * Conjugate(p) = 0
//
// For a dual quaternion A = p + q * e where p is not zero, the inverse is
//   Inverse(A) = Inverse(p) - Inverse(p) * q * Inverse(p) * e
// Note that dual quaternions of the form 0 + q * e are not invertible.
//
//
// RIGID TRANSFORMATIONS
//
// Application to rigid transformations in 3D, Y = R * X + T, where
// R is a 3x3 rotation matrix, T is a 3x1 translation vector, X is the
// 3x1 input and Y is the 3x1 output.
//
// The rigid transformation is represented by the dual quaternion
//   d = r + (t * r / 2) * e
// where r is a unit quaternion representing the rotation and t is a
// quaternion of the form t = T0 * i + T1 * j + T2 * k + 0 * 1, where the
// translation as a 3-tuple is T = (T0,T1,T2). Given a dual quaternion
// d = r + v * e that represents a rigid transformation, the quaternion
// for the rotation is the scalar part r, which must be unit length. The
// scalar part v implicitly stores the translation quaternion t as
// v = (t * r / 2). Therefore, t = 2 * v * Conjugate(r).
//
// The product of rigid transformations d0 = r0 + (t0 * r0 / 2) * e and
// d1 = r1 + (t1 * r1 / 2) * e is
//   d0 * d1 = (r0 * r1) + ((r0 * t1 * r1 + t0 * r0 * r1) / 2) * e
// The quaternion for rotation is r0 * r1, as expected. The translation t'
// is extracted as
//   t' = 2 * ((r0 * t1 * r1 + t0 * r0 * r1) / 2) * Conjugate(r0 * r1)
//      = (r0 * t1 * r1 + t0 * r0 * r1) * (Conjugate(r1) * Conjugate(r0))
//      = r0 * t1 * Conjugate(r0) + t0
// This is consistent with the product of 4x4 affine matrices
//   +-     -+ +-     -+   +-              -+
//   | R0 T0 | | R1 T1 | = | R0*R1 R0*T1+T0 |
//   | 0  1  | | 0  1  |   | 0     1        |
//   +-     -+ +-     -+   +-              -+
// The upper left of the right-hand-side matrix is a rotation R0*R1 which is
// represented by quaternion r0*r1. The upper right of the right-hand-side
// matrix is a translation R0*T1+T0 which is represented by quaternion
// r0*t1*Conjugate(r0)+t0. The first term of this expression is a rotation
// of t1 by r0. The second term adds in the tranlation t0.
//
// To apply a dual quaternion representing a rigid transformation to a
// 3-tuple point X = (x0,x1,x2) represented as a quaternion x = (x0,x1,x2,0)
// with output 3-tuple point Y represented as a quaterion y = (y0,y1,y2,0),
// let d = d0 + d1 * e = r + (t * r / 2) * e; then
//   y = r * x * Conjugate(r) + t
//     = d0 * x * Conjugate(d0) + 2 * d1 * Conjugate(d0)
//     = (d0 * x + 2 * d1) * Conjugate(d0)

#include <GTL/Mathematics/Algebra/Quaternion.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class DualQuaternion
    {
    public:
        using value_type = T;

        // The default constructor creates the identiy dual
        // quaternion 1 + 0 * e.
        DualQuaternion()
            :
            mElement{ Quaternion<T>::Identity(), Quaternion<T>::Zero() }
        {
        }

        // Create the dual quaternion p + q * e.
        DualQuaternion(Quaternion<T> const& p, Quaternion<T> const& q)
            :
            mElement{ p, q }
        {
        }

        // Member access.
        inline Quaternion<T> const& operator[] (size_t i) const
        {
            return mElement[i];
        }

        inline Quaternion<T>& operator[] (size_t i)
        {
            return mElement[i];
        }

        // Special quaternions.

        // 0 + 0 * e
        static DualQuaternion Zero()
        {
            return DualQuaternion(Quaternion<T>::Zero(), Quaternion<T>::Zero());
        }

        // 1 + 0 * e
        static DualQuaternion Identity()
        {
            return DualQuaternion();
        }

    private:
        // The dual quaternion is mElement[0] + mElement[1] * e.
        std::array<Quaternion<T>, 2> mElement;

        friend class UnitTestDualQuaternion;
    };

    // Comparisons.
    template <typename T>
    bool operator==(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        return d0[0] == d1[0] && d0[1] == d1[1];
    }

    template <typename T>
    bool operator!=(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        return !operator==(d0, d1);
    }

    template <typename T>
    bool operator<(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        if (d0[0] < d1[0])
        {
            return true;
        }
        if (d0[0] > d1[0])
        {
            return false;
        }
        return d0[1] < d1[1];
    }

    template <typename T>
    bool operator<=(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        return !operator<(d1, d0);
    }

    template <typename T>
    bool operator>(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        return operator<(d1, d0);
    }

    template <typename T>
    bool operator>=(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        return !operator<(d0, d1);
    }

    // Unary operations.
    template <typename T>
    DualQuaternion<T> operator+(DualQuaternion<T> const& d)
    {
        return d;
    }

    template <typename T>
    DualQuaternion<T> operator-(DualQuaternion<T> const& d)
    {
        return DualQuaternion<T>(-d[0], -d[1]);
    }

    // Linear-algebraic operations.
    template <typename T>
    DualQuaternion<T> operator+(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        DualQuaternion<T> result = d0;
        return result += d1;
    }

    template <typename T>
    DualQuaternion<T> operator-(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        DualQuaternion<T> result = d0;
        return result -= d1;
    }

    template <typename T>
    DualQuaternion<T> operator*(DualQuaternion<T> const& d, T const& scalar)
    {
        DualQuaternion<T> result = d;
        return result *= scalar;
    }

    template <typename T>
    DualQuaternion<T> operator*(T const& scalar, DualQuaternion<T> const& d)
    {
        DualQuaternion<T> result = d;
        return result *= scalar;
    }

    template <typename T>
    DualQuaternion<T> operator/(DualQuaternion<T> const& d, T const& scalar)
    {
        DualQuaternion<T> result = d;
        return result /= scalar;
    }

    template <typename T>
    DualQuaternion<T>& operator+=(DualQuaternion<T>& d0, DualQuaternion<T> const& d1)
    {
        d0[0] += d1[0];
        d0[1] += d1[1];
        return d0;
    }

    template <typename T>
    DualQuaternion<T>& operator-=(DualQuaternion<T>& d0, DualQuaternion<T> const& d1)
    {
        d0[0] -= d1[0];
        d0[1] -= d1[1];
        return d0;
    }

    template <typename T>
    DualQuaternion<T>& operator*=(DualQuaternion<T>& d, T const& scalar)
    {
        d[0] *= scalar;
        d[1] *= scalar;
        return d;
    }

    template <typename T>
    DualQuaternion<T>& operator/=(DualQuaternion<T>& d, T const& scalar)
    {
        d[0] /= scalar;
        d[1] /= scalar;
        return d;
    }

    // Multiplication of dual quaternions.
    template <typename T>
    DualQuaternion<T> operator*(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        return DualQuaternion<T>(d0[0] * d1[0], d0[0] * d1[1] + d0[1] * d1[0]);
    }

    // The conjugate of a dual quaternion
    template <typename T>
    DualQuaternion<T> Conjugate(DualQuaternion<T> const& d)
    {
        return DualQuaternion<T>(Conjugate(d[0]), Conjugate(d[1]));
    }

    // Inverse of a dual quaternion p+q*e for which p is not zero. If p is
    // zero, the function returns the zero dual quaternion as a signal the
    // inversion failed. For a dual quaternion A = p + q * e where p is not
    // zero, the inverse is
    //   Inverse(A) = Inverse(p) - Inverse(p) * q * Inverse(p) * e
    // Note that dual quaternions of the form 0 + q * e are not invertible.
    template <typename T>
    DualQuaternion<T> Inverse(DualQuaternion<T> const& d)
    {
        Quaternion<T> invP = Inverse(d[0]);
        return DualQuaternion<T>(invP, -invP * d[1] * invP);
    }

    // Geometric operations.

    // Scalar product of dual quaternions:
    //   dot(A, B) = a0 * b0 + a1 * b1 + a2 * b2 + a3 * b3
    //             = (A * conj(B) + B * conj(A)) / 2
    //             = dot(B, A)
    //             = c0 + c1 * e
    // where c0 and c1 are real (no i, j or k terms).
    template <typename T>
    DualQuaternion<T> Dot(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        return (d0 * Conjugate(d1) + d1 * Conjugate(d0)) * C_<T>(1, 2);
    }

    // Cross product of dual quaternions.
    template <typename T>
    DualQuaternion<T> Cross(DualQuaternion<T> const& d0, DualQuaternion<T> const& d1)
    {
        return (d0 * d1 - d1 * d0) * C_<T>(1, 2);
    }

    // Norm of a dual quaternion, Norm(d) = s0 + s1 * e, where s0 and s1 are
    // scalars. In terms of quaternion, Norm(d) = (0,0,0,s0) + (0,0,0,s1) * e.
    template <typename T>
    DualQuaternion<T> Norm(DualQuaternion<T> const& d)
    {
        return d * Conjugate(d);
    }

    // Length of a dual quaternion,
    //   Length(A) = sqrt(Norm(A)) = sqrt(s0) + (s1 / (2 * sqrt(s0))) * e
    template <typename T>
    DualQuaternion<T> Length(DualQuaternion<T> const& d)
    {
        DualQuaternion<T> norm = Norm(d);
        T w0 = std::sqrt(norm[0][3]);
        T w1 = C_<T>(1, 2) * norm[1][3] / w0;
        Quaternion<T> q0(C_<T>(0), C_<T>(0), C_<T>(0), w0);
        Quaternion<T> q1(C_<T>(0), C_<T>(0), C_<T>(0), w1);
        return DualQuaternion<T>(q0, q1);
    }

    // Rotate and translate a point x = (x0,x1,x2,1) to y = (y0,y1,y2,1) by
    // a dual quaternion d = r + (t * r / 2) * e. The rigid transform is
    // Y = R*X + T, where X = (x0,x1,x2), Y = (y0,y1,y2), T = (t0,t1,t2) and
    // R is the rotation matrix represented by quaternion r. The translation
    // quaternion is t = (t0,t1,t2,0). The point input to the function is
    // (x0,x1,x2). The point output from the function is (y0,y1,y1).
    template <typename T>
    Vector3<T> RotateAndTranslate(DualQuaternion<T> const& d, Vector3<T> const& x)
    {
        Quaternion<T> xq(x[0], x[1], x[2], C_<T>(0));
        Quaternion<T> yq = (d[0] * xq + d[1] * C_<T>(2)) * Conjugate(d[0]);
        Vector3<T> y{ yq[0], yq[1], yq[2] };
        return y;
    }

    // Rotate and translate a point x = (x0,x1,x2,1) to y = (y0,y1,y2,1) by
    // a dual quaternion d = r + (t * r / 2) * e. The rigid transform is
    // Y = R*X + T, where X = (x0,x1,x2), Y = (y0,y1,y2), T = (t0,t1,t2) and
    // R is the rotation matrix represented by quaternion r. The translation
    // quaternion is t = (t0,t1,t2,0). The point input to the function is
    // (x0,x1,x2,1). The point output from the function is (y0,y1,y1,1).
    template <typename T>
    Vector4<T> RotateAndTranslate(DualQuaternion<T> const& d, Vector4<T> const& x)
    {
        Quaternion<T> xq(x[0], x[1], x[2], C_<T>(0));
        Quaternion<T> yq = (d[0] * xq + d[1] * C_<T>(2)) * Conjugate(d[0]);
        Vector4<T> y{ yq[0], yq[1], yq[2], C_<T>(1) };
        return y;
    }
}
