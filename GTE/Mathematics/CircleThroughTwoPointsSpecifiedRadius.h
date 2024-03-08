// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// This file provides an implementation of the algorithm in Section 8.6 of the
// book
//    Geometric Tools for Computer Graphics,
//    Philip J. Schneider and David H. Eberly,
//    Morgan Kaufmnn, San Francisco CA, 2002
//
// Given two distinct points P and Q and given a radius r, compute the centers
// of circles, each containing the points and having the specified radius.
//
// The book states that the circle centers are the points of intersection of
// circles |X-P|^2 = r^2 and |X-Q|^2 = r^2. The pseudocode simply calls a
// function to compute these intersections.
//
// In my opinion, a simpler approach uses the fact that the bisector of the
// line segment with endpoints P and Q is a line that contains the centers.
// The bisector is parameterized by X(t) = t*Perp(P-Q)+(P+Q)/2, where
// Perp(P-Q) is perpendicular to P-Q and has the same length as that of P-Q.
// We need values of t for which X(t)-P has length r,
//   X(t)-P = t*Perp(P - Q)-(P-Q)/2
//   r^2 = |X(t)-P|^2
//       = |t*Perp(P-Q)-(P-Q)/2|^2
//       = |Perp(P-Q)|^2 * t^2 - 2*t*Dot(Perp(P-Q),P-Q) + |P-Q|^2/4
//       = |P-Q|^2 * t^2 + |P-Q|^2/4
//       = |P-Q|^2 * (t^2 + 1/4)
// Observe that t^2+1/4 >= 1/4, which implies that r >= |P-Q|/2. This
// condition is clear geometrically. The radius must be at least half the
// length of the segment connecting P and Q.
//
// If r = |P-Q|/2, there is a single circle with center (P+Q)/2. If
// r > |P-Q|/2, there are two circles whose centers occur when
// t^2 = r^2/|P-Q|^2 - 1/4, which implies t = +/- sqrt(r^2/|P-Q|^2-1/4).

#include <Mathematics/Hypersphere.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>

namespace gte
{
    // The function returns the number of circles satisfying the constraints.
    // The number is 2 when r > |P-Q|/2, 1 when r = |P-Q|/2 or 0 when P = Q
    // or r < |P-Q|/2. Any circle[i] with i >= numIntersections has members
    // set to zero.
    template <typename T>
    size_t CircleThroughTwoPointsSpecifiedRadius(Vector2<T> const& P,
        Vector2<T> const& Q, T const& r, std::array<Circle2<T>, 2>& circle)
    {
        T const zero = static_cast<T>(0);

        Vector2<T> PmQ = P - Q;
        T sqrLengthPmQ = Dot(PmQ, PmQ);
        if (sqrLengthPmQ != zero)
        {
            T argument = r * r / sqrLengthPmQ - static_cast<T>(0.25);
            if (argument > zero)
            {
                T root = std::sqrt(argument);
                Vector2<T> bisectorOrigin = static_cast<T>(0.5) * (P + Q);
                Vector2<T> bisectorDirection = Perp(PmQ);
                circle[0].center = bisectorOrigin - root * bisectorDirection;
                circle[0].radius = r;
                circle[1].center = bisectorOrigin + root * bisectorDirection;
                circle[1].radius = r;
                return 2;
            }

            if (argument == zero)
            {
                circle[0].center = static_cast<T>(0.5) * (P + Q);
                circle[0].radius = r;
                circle[1].center = { zero, zero };
                circle[1].radius = zero;
                return 1;
            }
        }

        circle[0].center = { zero, zero };
        circle[0].radius = zero;
        circle[1].center = { zero, zero };
        circle[1].radius = zero;
        return 0;
    }
}
