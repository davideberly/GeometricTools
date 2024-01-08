// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// The spherical linear interpolation (slerp) of unit-length vectors
// q0 and q1 for t in [0,1] and theta in (0,pi) is
//   slerp(t,q0,q1) = [sin((1-t)*theta)*q0 + sin(theta)*q1]/sin(theta)
// where theta is the angle between q0 and q1 [cos(theta) = Dot(q0,q1)]. This
// function is a parameterization of the great spherical arc between q0 and q1
// on the unit hypersphere. Moreover, the parameterization has the property
// that a particle traveling along the arc does so with constant speed, where
// t is time.
//
// When applying slerp to unit-length quaternions (N = 4) that represent
// rotations, q and -q represent the same rotation. It is typical that a
// quaternion sequence is preprocessed by
//   std::array<T, N> q[numElements];  // assuming initialized
//   for (i0 = 0, i1 = 1; i1 < numElements; i0 = i1++)
//   {
//       cosA = Dot(q[i0], q[i1]);
//       if (cosA < 0)
//       {
//           q[i1] = -q[i1];  // now Dot(q[i0], q[i]1) >= 0
//       }
//   }
// so that the angle between consecutive quaternions is in [0,pi/2].
//
// The cosines might also be precomputed,
//   std::array<T, N> q[numElements];  // assuming initialized
//   T cosA[numElements-1];  // to be precomputed
//   for (i0 = 0, i1 = 1; i1 < numElements; i0 = i1++)
//   {
//       cosA[i0] = Dot(q[i0], q[i1]);
//       if (cosA[i0] < 0)
//       {
//           q[i1] = -q[i1];
//           cosA[i0] = -cosA[i0];
//       }
//   }
//
// For numerical robustness of slerp, the quaternions can be preprocessed
// so that a quaternion is inserted between each pair of original quaternions.
// Given q0 and q1, the midpoint of the arc connecting them is qh so that
// A = Dot(q0,q1) and A/2 = Dot(q0,qh) = Dot(qh,q1). The midpoint is
// qh = Slerp(1/2,q0,q1) = (q0 + q1)/|q0 + q1|. The preprocessing is
//   std::array<T, N> q[numElements];  // assuming initialized
//   std::array<T, N> qh[numElements-1];  // to be precomputed
//   T cosAH[numElements-1];  // to be precomputed
//   for (i0 = 0, i1 = 1; i1 < numElements; i0 = i1++)
//   {
//       cosA = Dot(q[i0], q[i1]);
//       if (cosA < 0)
//       {
//           q[i1] = -q[i1];
//           cosA = -cosA;
//       }
//       cosAH[i0] = sqrt((1 + cosA) / 2);
//       qh[i0] = (q0 + q1) / (2 * cosAH[i0]);
//   }

#include <Mathematics/ChebyshevRatio.h>
#include <array>
#include <cstddef>

namespace gte
{
    // The angle between q0 and q1 is in [0,pi).
    template <typename T, size_t N>
    std::array<T, N> Slerp(T t,
        std::array<T, N> const& q0, std::array<T, N> const& q1)
    {
        static_assert(
            N >= 2,
            "Invalid dimension.");

        T const zero = static_cast<T>(0);
        T cosA = zero;
        for (size_t i = 0; i < N; ++i)
        {
            cosA += q0[i] * q1[i];
        }

        std::array<T, 2> f = ChebyshevRatiosUsingCosAngle<T>(t, cosA);
        std::array<T, N> result{};
        result.fill(zero);
        for (size_t i = 0; i < N; ++i)
        {
            result[i] += f[0] * q0[i] + f[1] * q1[i];
        }
        return result;
    }

    // The angle between q0 and q1 must be in [0,pi) and cosA = Dot(q0,q1).
    template <typename T, size_t N>
    std::array<T, N> Slerp(T t,
        std::array<T, N> const& q0, std::array<T, N> const& q1, T cosA)
    {
        static_assert(
            N >= 2,
            "Invalid dimension.");

        std::array<T, 2> f = ChebyshevRatiosUsingCosAngle<T>(t, cosA);

        std::array<T, N> result{};
        result.fill(static_cast<T>(0));
        for (size_t i = 0; i < N; ++i)
        {
            result[i] += f[0] * q0[i] + f[1] * q1[i];
        }
        return result;
    }

    // The angle between q0 and q1 is in [0,pi). The input qh is halfway
    // between q0 and q1 along a hyperspherical arc. If cosA = Dot(q0,q1),
    // then cosAH = sqrt((1+cosA)/2) and qh = (q0+q1)/(2*cosAH).
    template <typename T, size_t N>
    std::array<T, N> Slerp(T t,
        std::array<T, N> const& q0, std::array<T, N> const& q1,
        std::array<T, N> const& qh, T cosAH)
    {
        static_assert(
            N >= 2,
            "Invalid dimension.");

        std::array<T, 2> f{};
        std::array<T, N> result{};
        result.fill(static_cast<T>(0));

        T twoT = static_cast<T>(2) * t;
        if (twoT <= static_cast<T>(1))
        {
            f = ChebyshevRatiosUsingCosAngle<T>(twoT, cosAH);
            for (size_t i = 0; i < N; ++i)
            {
                result[i] += f[0] * q0[i] + f[1] * qh[i];
            }
        }
        else
        {
            f = ChebyshevRatiosUsingCosAngle<T>(twoT - static_cast<T>(1), cosAH);
            for (size_t i = 0; i < N; ++i)
            {
                result[i] += f[0] * qh[i] + f[1] * q1[i];
            }
        }
        return result;
    }
}
