// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// This file provides an implementation of the algorithm in Section 8.7 of the
// book
//    Geometric Tools for Computer Graphics,
//    Philip J. Schneider and David H. Eberly,
//    Morgan Kaufmnn, San Francisco CA, 2002
//
// Given a point P, a radius r and a line Dot(N,X-A) = 0, where A is a point
// on the line and N is a unit-length normal to the line. Compute the centers
// of circles, each containing the point, having the specified radius and have
// the line as a tangent line. The book describes one algebraic approach to
// solving the problem. The implementation here is another approach, a portion
// using the algorithm of Section 8.6.
// 
// Let N = (n0,n1) and define the unit-length perpendicular D = Perp(N) =
// (-n1,n0). Represent P = A+u*D+s*N with parameters u = Dot(D,P-A) and
// s = Dot(N,P-A). The parameter s is the signed distance from P to the line.
// To simplify the logic of the implementation, if s < 0, the values of s, N
// and D are negated. The discussion below assumes s >= 0.
// 
// The cases are
//
//   (1) s = 0: P is on the line. There are two circles containing P and
//       tangent to the line at P. This is shown in the left image of Figure
//       8.13. The circle centers are C0 = P-r*N and C1 = P+r*N.
//
//   (2) s = r: The book does not have a figure to illustrate this case. The
//       two circles have a single point of intersection, which is P. The
//       circle centers are C0 = P-r*D and C1 = P+r*D.
//
//   (3) s = 2*r: P is the farthest point on a circle of radius r which has
//       the line as the tangent line. The circle center is C0 = P-r*N.
//
//   (4) s > 2*r: The distance from P to the tangent line is larger than the
//       desired circle diameter, so there is no circle that satisfies the
//       constraints.
//
//   (5a) 0 < s < r: This is shown in Figure 8.12. Observe that the two
//        circles intersect in P. There is another point of intersection Q
//        that is not labeled in the figure. We can represent
//        Q = P+u*D+(2*r-s)*N. The bisector of segment <P,Q> has origin
//        (P+Q)/2 = P+u*D+r*N. The bisector direction is D. If a circle center
//        is C, the triangle <P,B,C> is a right triangle at B. Using the
//        Pythagorean theorem, the length of segment <B,C> is h = |B-C| =
//        sqrt(r^2 - (r-s)^2). The circle centers are C0 = B-h*D and
//        C1 = B+h*D.
//
//   (5b) r < s < 2*r: This is analogous to (5a). Figure 8.12 still applies
//        except that Q must be the label on the intersection point closest to
//        the tangent line and P must be the label on the intersection point
//        farthest from the tangent line. The construction of the centers is
//        the same as that (5a).

#include <Mathematics/Hypersphere.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>

namespace gte
{
    // The function returns the number of circles satisfying the constraints.
    // Any circle[i] with i >= numIntersections has members set to zero.
    template <typename T>
    size_t CircleThroughPointSpecifiedTangentAndRadius(Vector2<T> const& P,
        Vector2<T> const& A, Vector2<T> N, T const& r, std::array<Circle2<T>, 2>& circle)
    {
        T const zero = static_cast<T>(0);

        Vector2<T> PmA = P - A;
        T s = Dot(N, PmA);
        if (s == zero)
        {
            // Case (1).
            circle[0].center = P - r * N;
            circle[0].radius = r;
            circle[1].center = P + r * N;
            circle[1].radius = r;
            return 2;
        }

        if (s < zero)
        {
            N = -N;
            s = -s;
        }

        if (s == r)
        {
            // Case (2).
            Vector2<T> D = Perp(N);
            circle[0].center = P - r * D;
            circle[0].radius = r;
            circle[1].center = P + r * D;
            circle[1].radius = r;
            return 2;
        }

        T const twoR = static_cast<T>(2) * r;
        if (s == twoR)
        {
            // Case (3).
            circle[0].center = P - r * N;
            circle[0].radius = r;
            circle[1].center = { zero, zero };
            circle[1].radius = zero;
            return 1;
        }

        if (s > twoR)
        {
            // Case (4).
            circle[0].center = { zero, zero };
            circle[0].radius = zero;
            circle[1].center = { zero, zero };
            circle[1].radius = zero;
            return 0;
        }

        // The bisector origin is D = Perp(N) and The bisector origin is
        // B = (P + Q) / 2 = A + t * D + r * N with t = Dot(D, P - A).
        Vector2<T> bisectorDirection = Perp(N);
        T t = Dot(bisectorDirection, PmA);
        Vector2<T> bisectorOrigin = A + t * bisectorDirection + r * N;

        T diffRS = r - s;
        T argument = r * r - diffRS * diffRS;
        if (argument > zero)
        {
            T h = std::sqrt(argument);
            circle[0].center = bisectorOrigin - h * bisectorDirection;
            circle[0].radius = r;
            circle[1].center = bisectorOrigin + h * bisectorDirection;
            circle[1].radius = r;
            return 2;
        }
        else
        {
            // Theoretically this code cannot be reached, but floating-point
            // rounding errors might trigger it. This corresponds to Case (3)
            // where r = s.
            circle[0].center = bisectorOrigin;
            circle[0].radius = r;
            circle[1].center = { zero, zero };
            circle[1].radius = zero;
            return 1;
        }
    }
}
