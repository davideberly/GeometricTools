// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// Approximate a 2-dimensional parametric curve X(t) for t in [tmin,tmax] by a
// collection of circular arcs. Some of the arcs can be degenerate in that the
// arc center is a point at infinity. In this case, the arc represents a line
// segment connecting its endpoints, and the arc radius is set to the number
// std::numeric_limits<T>::max() to let the caller know the object is actually
// a line segment. The algorithm is described in
//   https://www.geometrictools.com/Documentation/ApproximateCurveByArcs.pdf
// The collection of arcs form a C0-continuous curve. Generally, the
// derivatives at a curve point shared by two arcs are not equal.

#include <Mathematics/Arc2.h>
#include <Mathematics/Logger.h>
#include <Mathematics/ParametricCurve.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

namespace gte
{
    // The number of arcs N (input numArcs) must be positive. The times[] and
    // points[] inputs are resized to have 2*N+1 elements. The parametric
    // curve times and samples are stored in the even-indexed locations of
    // these containers. The odd-indexed locations store the times and
    // midpoints that are used for fitting arcs to subcurves.
    // 
    // An arc has endpoints {P0,P1} = {point[2*i],point[2*i+2]} corresponding
    // to parameters {t0,t1} = {times[2*i],times[2*i+1]}. The midpoint of the
    // arc is at M = point[2*i+1] corresponding to parameter tmid =
    // times[2*i+1].
    // 
    // The arc has a center point C and a radius r. If {P0,M,P1} are not
    // colinear, the radius is finite and the arc is truly an arc. If the
    // point triple is colinear (or nearly colinear), the center components
    // and radius are set to std::numeric_limits<T>::max() to let the caller
    // know that the arc represents a line segment; the segment endpoints are
    // the arc endpoints.
    //
    // The arc center is computed as the solution to a linear system of
    // equations in the components of C. If the determinant of this system is
    // nearly 0, the triple {P0,M,P1} is nearly collinear. The epsilon input
    // to the function must be nonnegative and is a lower threshold for the
    // determinant; that is, if the determinant is greater or equal to
    // epsilon, the linear system is solved for C. If the determinant is
    // smaller than epsilon, the arc center and radius are set as mentioned
    // in the previous paragraph to let the caller know the arc really
    // represents a line segment. You can set epsilon to 0, but nearly
    // colinear {P0,M,P1} can cause floating-point rounding errors to produce
    // inaccurate center and radius.

    template <typename T>
    void ApproximateCurveByArcs(
        std::shared_ptr<ParametricCurve<2, T>> const& curve,
        size_t numArcs,
        std::vector<T>& times,
        std::vector<Vector2<T>>& points,
        std::vector<Arc2<T>>& arcs,
        T epsilon = static_cast<T>(0))
    {
        static_assert(std::is_floating_point<T>::value,
            "The template type must be 'float' or 'double'.");

        LogAssert(
            curve != nullptr && numArcs >= 1,
            "Invalid input.");

        size_t const numTimes = 2 * numArcs + 1;
        times.resize(numTimes);
        points.resize(numTimes);
        arcs.resize(numArcs);

        // Subdivide the curve by arc length. The arc length between any pair
        // of consecutive points is constant. The consecutive points are
        // stored in the even-indexed locations. The odd-indexed locations are
        // assigned in the block of code after this one.
        T totalLength = curve->GetTotalLength();
        T deltaLength = totalLength / static_cast<T>(numTimes - 1);
        for (size_t i = 0; i < numTimes; i += 2)
        {
            T length = deltaLength * static_cast<T>(i);
            times[i] = curve->GetTime(length);
            points[i] = curve->GetPosition(times[i]);
        }

        T const zero = static_cast<T>(0);
        T const half = static_cast<T>(0.5);
        for (size_t i = 0, j0 = 0, j1 = 1, j2 = 2; i < numArcs; ++i, j0 += 2, j1 += 2, j2 += 2)
        {
            Arc2<T>& arc = arcs[i];
            arc.end[0] = points[j0];
            arc.end[1] = points[j2];
            auto const& P0 = arc.end[0];
            auto const& P1 = arc.end[1];

            // Let P0 = arc.end[0] and P1 = arc.end[1]. Compute a point of
            // intersection between the bisector of segment <P0, P1> and the
            // curve X(t). This is accomplished using bisection for
            // F(t) = Dot(D, X(t) - A) on [t0,t1] with P0 = X(t0), P1 = X(t1),
            // D = P1 - P0, and A = (P0 + P1) / 2. Observe that
            //   F(t0) = Dot(D, P0 - A) = -|D|^2/2 < 0
            //   F(t1) = Dot(D, P1 - A) = +|D|^2/2 > 0
            // There must be a tRoot in [t0,t1] for which F(tRoot) = 0.
            // 
            // The loop is guaranteed to terminate because a sufficient number
            // of iterations will either find a tRoot where F(tRoot) = 0 using
            // floating-point computations. or the interval to bisect has
            // consecutive floating-point endpoints and the interval midpoint
            // rounds to one of those endpoints.
            Vector2<T> D = P1 - P0;
            Vector2<T> A = half * (P0 + P1);
            T t0 = times[j0], t1 = times[j2], tRoot{}, fAtRoot{};
            for (;;)
            {
                tRoot = half * (t0 + t1);
                fAtRoot = Dot(D, curve->GetPosition(tRoot) - A);
                int32_t signRoot = (fAtRoot > zero ? +1 : (fAtRoot < zero ? -1 : 0));
                if (signRoot == 0 || tRoot == t0 || tRoot == t1)
                {
                    break;
                }

                if (signRoot == -1)
                {
                    t0 = tRoot;
                }
                else // signRoot = +1
                {
                    t1 = tRoot;
                }
            }

            // Fill in the odd-indexed values.
            times[j1] = tRoot;
            points[j1] = curve->GetPosition(tRoot);
            auto const& M = points[j1];

            // The points P0, X(tRoot), and P1 are circumscribed to determine
            // the arc. If the three points are colinear, the center and
            // radius of the arc are set to std::numeric_limits<T>::max() as
            // a signal to the caller that the arc represents a line segment.
            Vector2<T> diffP0M = P0 - M;
            Vector2<T> diffP1M = P1 - M;
            Vector2<T> avrgP0M = half * (P0 + M);
            Vector2<T> avrgP1M = half * (P1 + M);
            T dot0 = Dot(diffP0M, avrgP0M);
            T dot1 = Dot(diffP1M, avrgP1M);
            T det = DotPerp(diffP0M, diffP1M);
            if (std::fabs(det) >= epsilon)
            {
                arc.center[0] = (diffP1M[1] * dot0 - diffP0M[1] * dot1) / det;
                arc.center[1] = (diffP0M[0] * dot1 - diffP1M[0] * dot0) / det;
                arc.radius = Length(M - arc.center);
            }
            else
            {
                T constexpr tmax = std::numeric_limits<T>::max();
                arc.center = { tmax, tmax };
                arc.radius = tmax;
            }
        }
    }
}
