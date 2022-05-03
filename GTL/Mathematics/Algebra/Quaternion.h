// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// A quaternion is of the form
//   q = x * i + y * j + z * k + w * 1 = x * i + y * j + z * k + w
// where w, x, y, and z are real numbers. The scalar and vector parts are
//   Vector(q) = x * i + y * j + z * k
//   Scalar(q) = w
//   q = Vector(q) + Scalar(q)
// A document describing the arithmetic and algebraic properties of
// quaternions is
// https://www.geometrictools.com/Documentation/Quaternions.pdf

#include <GTL/Mathematics/Algebra/Vector.h>
#include <GTL/Mathematics/Functions/ChebyshevRatio.h>
#include <array>
#include <cmath>

namespace gtl
{
    // The type T must be a numeric type that represents real numbers.
    template <typename T>
    class Quaternion
    {
    public:
        using value_type = T;

        // The default constructor initializes its members to zero.
        Quaternion()
        {
            mElements.fill(C_<T>(0));
        }

        // The quaternion is q = x * i + y * j + z * k + w.
        Quaternion(T const& x, T const& y, T const& z, T const& w)
        {
            mElements[0] = x;
            mElements[1] = y;
            mElements[2] = z;
            mElements[3] = w;
        }

        // Member access.
        inline T const& operator[](size_t i) const
        {
            return mElements[i];
        }

        inline T& operator[](size_t i)
        {
            return mElements[i];
        }

        // Special quaternions.

        // z = 0*i + 0*j + 0*k + 0
        inline static Quaternion Zero()
        {
            return Quaternion{};
        }

        // i = 1*i + 0*j + 0*k + 0
        inline static Quaternion I()
        {
            return Quaternion(C_<T>(1), C_<T>(0), C_<T>(0), C_<T>(0));
        }

        // j = 0*i + 1*j + 0*k + 0
        inline static Quaternion J()
        {
            return Quaternion(C_<T>(0), C_<T>(1), C_<T>(0), C_<T>(0));
        }

        // k = 0*i + 0*j + 1*k + 0
        inline static Quaternion K()
        {
            return Quaternion(C_<T>(0), C_<T>(0), C_<T>(1), C_<T>(0));
        }

        // 1 = 0*i + 0*j + 0*k + 1
        inline static Quaternion Identity()
        {
            return Quaternion(C_<T>(0), C_<T>(0), C_<T>(0), C_<T>(1));
        }

    private:
        std::array<T, 4> mElements;

        friend class UnitTestQuaternion;
    };


    // Comparisons.
    template <typename T>
    bool operator==(Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            if (q0[i] != q1[i])
            {
                return false;
            }
        }
        return true;
    }

    template <typename T>
    bool operator!=(Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        return !operator==(q0, q1);
    }

    template <typename T>
    bool operator<(Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            if (q0[i] < q1[i])
            {
                return true;
            }
            if (q0[i] > q1[i])
            {
                return false;
            }
        }
        return false;
    }

    template <typename T>
    bool operator<=(Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        return !operator<(q1, q0);
    }

    template <typename T>
    bool operator>(Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        return operator<(q1, q0);
    }

    template <typename T>
    bool operator>=(Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        return !operator<(q0, q1);
    }

    // Unary operations.
    template <typename T>
    Quaternion<T> operator+(Quaternion<T> const& q)
    {
        return q;
    }

    template <typename T>
    Quaternion<T> operator-(Quaternion<T> const& q)
    {
        return Quaternion<T>{ -q[0], -q[1], -q[2], -q[3] };
    }

    // Linear algebraic operations.
    template <typename T>
    Quaternion<T> operator+(Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        Quaternion<T> result = q0;
        return result += q1;
    }

    template <typename T>
    Quaternion<T> operator-(Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        Quaternion<T> result = q0;
        return result -= q1;
    }

    template <typename T>
    Quaternion<T> operator*(Quaternion<T> const& q, T const& scalar)
    {
        Quaternion<T> result = q;
        return result *= scalar;
    }

    template <typename T>
    Quaternion<T> operator*(T const& scalar, Quaternion<T> const& q)
    {
        Quaternion<T> result = q;
        return result *= scalar;
    }

    template <typename T>
    Quaternion<T> operator/(Quaternion<T> const& q, T const& scalar)
    {
        Quaternion<T> result = q;
        return result /= scalar;
    }

    template <typename T>
    Quaternion<T>& operator+=(Quaternion<T>& q0, Quaternion<T> const& q1)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            q0[i] += q1[i];
        }
        return q0;
    }

    template <typename T>
    Quaternion<T>& operator-=(Quaternion<T>& q0, Quaternion<T> const& q1)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            q0[i] -= q1[i];
        }
        return q0;
    }

    template <typename T>
    Quaternion<T>& operator*=(Quaternion<T>& q, T const& scalar)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            q[i] *= scalar;
        }
        return q;
    }

    template <typename T>
    Quaternion<T>& operator/=(Quaternion<T>& q, T const& scalar)
    {
        for (size_t i = 0; i < 4; ++i)
        {
            q[i] /= scalar;
        }
        return q;
    }

    // Geometric operations.
    template <typename T>
    T Dot(Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        T dot = C_<T>(0);
        for (size_t i = 0; i < 4; ++i)
        {
            dot += q0[i] * q1[i];
        }
        return dot;
    }

    template <typename T>
    T Length(Quaternion<T> const& q)
    {
        return std::sqrt(Dot(q, q));
    }

    template <typename T>
    T Normalize(Quaternion<T>& q)
    {
        T length = std::sqrt(Dot(q, q));
        if (length > C_<T>(0))
        {
            q /= length;
        }
        else
        {
            for (size_t i = 0; i < 4; ++i)
            {
                q[i] = C_<T>(0);
            }
        }
        return length;
    }

    // Multiplication of quaternions. This operation is not generally
    // commutative; that is, q0*q1 and q1*q0 are not usually the same value.
    //   (x0*i + y0*j + z0*k + w0)*(x1*i + y1*j + z1*k + w1)
    //   =
    //   i*(+x0*w1 + y0*z1 - z0*y1 + w0*x1) +
    //   j*(-x0*z1 + y0*w1 + z0*x1 + w0*y1) +
    //   k*(+x0*y1 - y0*x1 + z0*w1 + w0*z1) +
    //   1*(-x0*x1 - y0*y1 - z0*z1 + w0*w1)
    template <typename T>
    Quaternion<T> operator*(Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        return Quaternion<T>
        (
            +q0[0] * q1[3] + q0[1] * q1[2] - q0[2] * q1[1] + q0[3] * q1[0],
            -q0[0] * q1[2] + q0[1] * q1[3] + q0[2] * q1[0] + q0[3] * q1[1],
            +q0[0] * q1[1] - q0[1] * q1[0] + q0[2] * q1[3] + q0[3] * q1[2],
            -q0[0] * q1[0] - q0[1] * q1[1] - q0[2] * q1[2] + q0[3] * q1[3]
        );
    }

    // For a nonzero quaternion q = (x,y,z,w), inv(q) = (-x,-y,-z,w)/|q|^2,
    // where |q| is the length of the quaternion. When q is zero, the function
    // returns zero, which is considered to be an improbable case.
    template <typename T>
    Quaternion<T> Inverse(Quaternion<T> const& q)
    {
        T sqrLen = Dot(q, q);
        if (sqrLen > C_<T>(0))
        {
            Quaternion<T> inverse = Conjugate(q) / sqrLen;
            return inverse;
        }
        else
        {
            return Quaternion<T>{};  // zero
        }
    }

    // The conjugate of q = (x,y,z,w) is conj(q) = (-x,-y,-z,w).
    template <typename T>
    Quaternion<T> Conjugate(Quaternion<T> const& q)
    {
        return Quaternion<T>(-q[0], -q[1], -q[2], +q[3]);
    }

    // If q = A*(x*i + y*j + z*k) where (x,y,z) is unit length, then
    // exp(q) = sin(A)*(x*i + y*j + z*k) + cos(A). Observe that the input
    // q is not generally a unit-length quaternion. It is the caller's
    // responsibility to ensure that the w-component of q is zero.
    template <typename T>
    Quaternion<T> Exp(Quaternion<T> const& q)
    {
        T angle = Length(q);
        if (angle != C_<T>(0))
        {
            T sn = std::sin(angle);
            T multiplier = sn / angle;
            Quaternion<T> result;
            for (size_t i = 0; i < 3; ++i)
            {
                result[i] = multiplier * q[i];
            }
            result[3] = std::cos(angle);
            return result;
        }
        else
        {
            return Quaternion<T>::Identity();
        }
    }

    // If q = sin(A)*(x*i + y*j + z*k) + cos(A) where (x,y,z) is unit
    // length, then log(q) = A*(x*i + y*j + z*k).
    template <typename T>
    Quaternion<T> Log(Quaternion<T> const& q)
    {
        if (std::fabs(q[3]) < C_<T>(1))
        {
            T angle = std::acos(q[3]);
            T sn = std::sin(angle);
            if (std::fabs(sn) > C_<T>(0))
            {
                T multiplier = angle / sn;
                Quaternion<T> result;
                for (size_t i = 0; i < 3; ++i)
                {
                    result[i] = multiplier * q[i];
                }
                result[3] = C_<T>(0);
                return result;
            }
        }

        // The angle A is zero (q = 1) or pi (q = -1). In either
        // case, the log(q) = 0.
        return Quaternion<T>{}; // zero
    }

    // Rotate a 3D vector v = (v0,v1,v2) using quaternion multiplication. The
    // input quaternion must be unit length. If R is the rotation matrix
    // corresponding to the quaternion q, the rotated vector u = R*v is
    // computed by this function.
    template <typename T>
    Vector3<T> Rotate(Quaternion<T> const& q, Vector3<T> const& v)
    {
        Quaternion<T> input(v[0], v[1], v[2], C_<T>(0));
        Quaternion<T> output = q * input * Conjugate(q);
        Vector3<T> u{ output[0], output[1], output[2] };
        return u;
    }

    // Rotate a 3D vector, represented as a homogeneous 4D vector
    // v = (v0,v1,v2,0), using quaternion multiplication. The input quaternion
    // must be unit length. If R is the homogeneous 4x4 rotation matrix whose
    // upper 3x3 is the 3D rotation matrix corresponding to the quaternion q,
    // the rotated vector u = R*v is computed by this function.
    template <typename T>
    Vector4<T> Rotate(Quaternion<T> const& q, Vector4<T> const& v)
    {
        Quaternion<T> input(v[0], v[1], v[2], C_<T>(0));
        Quaternion<T> output = q * input * Conjugate(q);
        Vector4<T> u{ output[0], output[1], output[2], C_<T>(0) };
        return u;
    }

    // The spherical linear interpolation (slerp) of unit-length quaternions
    // q0 and q1 for t in [0,1] is
    //     slerp(t,q0,q1) = [sin(t*theta)*q0 + sin((1-t)*theta)*q1]/sin(theta)
    // where theta is the angle between q0 and q1 [cos(theta) = Dot(q0,q1)].
    // This function is a parameterization of the great spherical arc between
    // q0 and q1 on the unit hypersphere. Moreover, the parameterization is
    // one of normalized arclength: a particle traveling along the arc through
    // time t does so with constant speed.
    //
    // When using slerp in animations involving sequences of quaternions, it
    // is typical that the quaternions are preprocessed so that consecutive
    // ones form an acute angle A in [0,pi/2]. Other preprocessing can help
    // with performance.  See the function comments below.
    //
    // See SlerpEstimate.h for approximations, including the function
    // SlerpEstimate::DegreeRPH that gives good performance and accurate
    // results for preprocessed quaternions.

    // The angle between q0 and q1 is in [0,pi). There are no angle
    // restrictions and nothing is precomputed.
    template <typename T>
    Quaternion<T> Slerp(T const& t, Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        T cosA = Dot(q0, q1);
        T sign{};
        if (cosA >= C_<T>(0))
        {
            sign = C_<T>(1);
        }
        else
        {
            cosA = -cosA;
            sign = -C_<T>(1);
        }

        std::array<T, 2> f = ChebyshevRatiosUsingCosAngle(t, cosA);
        return q0 * f[0] + q1 * (sign * f[1]);
    }

    // The angle between q0 and q1 must be in [0,pi/2]. The suffix R is for
    // 'Restricted'. The preprocessing code is
    //   Quaternion<T> q[n];  // assuming initialized
    //   for (i0 = 0, i1 = 1; i1 < n; i0 = i1++)
    //   {
    //       cosA = Dot(q[i0], q[i1]);
    //       if (cosA < 0)
    //       {
    //           q[i1] = -q[i1];  // now Dot(q[i0], q[i]1) >= 0
    //       }
    //   }
    template <typename T>
    Quaternion<T> SlerpR(T const& t, Quaternion<T> const& q0, Quaternion<T> const& q1)
    {
        T cosA = Dot(q0, q1);
        std::array<T, 2> f = ChebyshevRatiosUsingCosAngle(t, cosA);
        return q0 * f[0] + q1 * f[1];
    }

    // The angle between q0 and q1 must be in [0,pi/2]. The suffix R is for
    // 'Restricted' and the suffix P is for 'Preprocessed'. The preprocessing
    // code is
    //   Quaternion<T> q[n];  // assuming initialized
    //   T cosA[n-1];  // precomputed here
    //   for (i0 = 0, i1 = 1; i1 < n; i0 = i1++)
    //   {
    //       T cs = Dot(q[i0], q[i1]);
    //       if (cs < 0)
    //       {
    //           q[i1] = -q[i1];  // now Dot(q[i0], q[i]1) >= 0
    //           cs = -cs;
    //       }
    //       cosA[i0] = cs;
    //   }
    template <typename T>
    Quaternion<T> SlerpRP(T const& t, Quaternion<T> const& q0, Quaternion<T> const& q1,
        T const& cosA)
    {
        std::array<T, 2> f = ChebyshevRatiosUsingCosAngle(t, cosA);
        return q0 * f[0] + q1 * f[1];
    }

    // The angle between q0 and q1 is A and must be in [0,pi/2]. The suffic R
    // is for 'Restricted', the suffix P is for 'Preprocessed' and the suffix
    // H is for 'Half' (the quaternion qh halfway between q0 and q1 is
    // precomputed). Quaternion qh is slerp(1/2,q0,q1) = (q0+q1)/|q0+q1|, so
    // the angle between q0 and qh is A/2 and the angle between qh and q1 is
    // A/2. The preprocessing code is
    //   Quaternion<T> q[n];  // assuming initialized
    //   Quaternion<T> qh[n-1];  // precomputed here
    //   T omcosAH[n-1];  // precomputed here
    //   for (i0 = 0, i1 = 1; i1 < n; i0 = i1++)
    //   {
    //       T cosA = Dot(q[i0], q[i1]);
    //       if (cosA < 0)
    //       {
    //           q[i1] = -q[i1];  // now Dot(q[i0], q[i]1) >= 0
    //           cosA = -cosA;
    //       }
    //       cosAH[i0] = sqrt((1+cosA)/2);
    //       qh[i0] = (q0 + q1) / (2 * cosAH[i0]);
    //   }
    template <typename T>
    Quaternion<T> SlerpRPH(T const& t, Quaternion<T> const& q0, Quaternion<T> const& q1,
        Quaternion<T> const& qh, T const& cosAH)
    {
        std::array<T, 2> f;
        T twoT = C_<T>(2) * t;
        if (twoT <= C_<T>(1))
        {
            f = ChebyshevRatiosUsingCosAngle(twoT, cosAH);
            return q0 * f[0] + qh * f[1];
        }
        else
        {
            f = ChebyshevRatiosUsingCosAngle(twoT - C_<T>(1), cosAH);
            return qh * f[0] + q1 * f[1];
        }
    }
}
