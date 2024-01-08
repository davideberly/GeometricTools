// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2023.08.08

#pragma once

// The line is parameterized by L(t) = P + t * D, where P is a point on the
// line and D is a nonzero direction vector that is not necessarily unit
// length.
//
// The standard torus has center (0,0,0), plane of symmetry z = 0, axis of
// symmetry containing (0,0,0) in the direction (0,0,1), outer radius r0
// and inner radius r1 > r0 (a "ring torus"). It is defined implicitly by
//   (x^2 + y^2 + z^2 + r0^2 - r1^2)^2 - 4 * r0^2 * (x^2 + y^2) = 0
// where (x,y,z) is a point on the torus. A parameterization is
//   x(u,v) = (r0 + r1 * cos(v)) * cos(u)
//   y(u,v) = (r0 + r1 * cos(v)) * sin(u)
//   z(u,v) = r1 * sin(v)
// for u in [0,2*pi) and v in [0,2*pi).
//   
// Generally, the torus has center C with plane of symmetry containing C and
// having unit-length normal N. The axis of symmetry is the normal line to
// the plane at C. If X is a point on the torus, the implicit formulation is
//   (|X-C|^2 + r0^2 - r1^2)^2 - 4 * r0^2 * (|X-C|^2 - (Dot(N,X-C))^2) = 0
// Let D0 and D1 be unit-length vectors that span the symmetry plane where
// {D0,D1,N} is a right-handed orthonormal basis. A parameterization for the
// torus is
//    X(u,v) = C + (r0 + r1*cos(v))*(cos(u)*D0 + sin(u)*D1) + r1*sin(v)*N
// for u in [0,2*pi) and v in [0,2*pi).
//
// Compute the intersections of a line with a torus. The number of
// intersections is between 0 and 4. As noted, line direction D does not
// have to be unit length. The normal vector N must be unit length, but
// notice that the implicit formulation has a term
//   (Dot(N,X-C))^2 = (X-C)^T * (N * N^T) * (X - C)
// If the normal were chosen to be nonzero but not unit length, say M, then
// N = M/|M}. The term can be modified to
//   (Dot(N,X-C))^2 = (X-C)^T * ((M * M^T)/|M|^2) * (X - C)
// This formulation supports exact rational arithmetic when computing the
// roots of a quartic polynomial associated with the find-intersection query.
// The rational arithmetic allows for a theoretically correct classification
// of the polynomial roots, although the actual root computation will have
// rounding errors when converting to a floating-point result.

#include <Mathematics/FIQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/Torus3.h>
#include <Mathematics/Polynomial1.h>
#include <Mathematics/RootsPolynomial.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>

namespace gte
{
    template <typename T>
    class FIQuery<T, Line3<T>, Torus3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                lineParameter{
                    static_cast<T>(0),
                    static_cast<T>(0),
                    static_cast<T>(0),
                    static_cast<T>(0)
                },
                torusParameter{ {
                    { static_cast<T>(0), static_cast<T>(0) },
                    { static_cast<T>(0), static_cast<T>(0) },
                    { static_cast<T>(0), static_cast<T>(0) },
                    { static_cast<T>(0), static_cast<T>(0) }
                } },
                point{
                    Vector3<T>::Zero(),
                    Vector3<T>::Zero(),
                    Vector3<T>::Zero(),
                    Vector3<T>::Zero()
                }
            {
            }

            bool intersect;
            size_t numIntersections;
            std::array<T, 4> lineParameter;
            std::array<std::array<T, 2>, 4> torusParameter;
            std::array<Vector3<T>, 4> point;
        };

        Result operator()(Line3<T> const& line, Torus3<T> const& torus)
        {
            Result result{};

            // Short names for readability.
            auto const& P = line.origin;
            auto const& D = line.direction;
            auto const& C = torus.center;
            auto const& N = torus.normal;
            auto const& r0 = torus.radius0;  // outer radius
            auto const& r1 = torus.radius1;  // inner radius

            // Common intermediate terms.
            T const two = static_cast<T>(2);
            T const four = static_cast<T>(4);
            T r0Sqr = r0 * r0;
            T r1Sqr = r1 * r1;
            Vector3<T> PmC = P - C;
            T sqrLenPmC = Dot(PmC, PmC);
            T dotDPmC = Dot(D, PmC);
            T sqrLenD = Dot(D, D);
            T sqrLenN = Dot(N, N);
            T dotND = Dot(N, D);
            T dotNPmC = Dot(N, PmC);

            // |X-C|^2
            Polynomial1<T> quad0(2);
            quad0[0] = sqrLenPmC;
            quad0[1] = two * dotDPmC;
            quad0[2] = sqrLenD;

            // |X-C|^2 + r0^2 - r1^2
            Polynomial1<T> quad1 = quad0;
            quad1[0] += r0Sqr - r1Sqr;

            // Dot(N,X-C)
            Polynomial1<T> linear{ dotNPmC, dotND };

            // Dot(N,X-C)^2 with adjustment for non-unit N
            Polynomial1<T> quad2 = (linear * linear) / sqrLenN;

            // |X-C|^2 - (Dot(N,X-C))^2
            Polynomial1<T> quad3 = quad0 - quad2;

            // (|X-C|^2 + r0^2-r1^2)^2 - 4*r0^2 * (|X-C|^2 - (Dot(N,X-C))^2)
            Polynomial1<T> quartic = quad1 * quad1 - four * r0Sqr * quad3;

            // Solve the quartic.
            std::map<T, int32_t> rmMap{};
            RootsPolynomial<T>::SolveQuartic(quartic[0], quartic[1],
                quartic[2], quartic[3], quartic[4], rmMap);

            // Get the intersection parameters and points.
            result.numIntersections = rmMap.size();
            result.intersect = (result.numIntersections > 0);
            size_t i = 0;
            for (auto const& element : rmMap)
            {
                result.lineParameter[i] = element.first;
                result.point[i] = line.origin + element.first * line.direction;
                torus.GetParameters(result.point[i],
                    result.torusParameter[i][0], result.torusParameter[i][1]);
                ++i;
            }

            return result;
        }
    };
}
