// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

// The algorithm for representing an arc as a NURBS curve is described in
//   https://www.geometrictools.com/Documentation/NURBSCircleSphere.pdf
// The SampleCircularArc class generates points on an arc. The arc must
// be counterclockwise ordered. The number of returned points is the
// approximate length of the arc. This is useful for 2D applications
// where you want to draw an arc. The alternative is to derive an algorithm
// for integer-based pixel selection similar to Bresenham's algorithm for
// a full circle.

#include <Mathematics/Arc2.h>
#include <Mathematics/Constants.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace gte
{
    template <typename T>
    class SampleCircularArc
    {
    public:
        void operator()(Arc2<T> const& arc, std::vector<Vector2<T>>& points)
        {
            // Translate and scale the arc to the unit circle centered at the
            // origin. Compute the angle subtended by the arc.
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const negOne = static_cast<T>(-1);
            T const twoPi = static_cast<T>(GTE_C_TWO_PI);
            Vector2<T> P0 = (arc.end[0] - arc.center) / arc.radius;
            Vector2<T> P2 = (arc.end[1] - arc.center) / arc.radius;
            T dot = std::max(negOne, std::min(Dot(P0, P2), one));
            T angle = std::acos(dot);

            // Decompose the arc into subarcs, each with subtended angle
            // in (0,pi/2].
            T dotPerp = DotPerp(P0, P2);
            if (dotPerp >= zero)
            {
                // The subtended angle is in [0,pi].
                if (dot >= zero)
                {
                    // The subtended angle is in [0,pi/2].
                    SampleArc1(arc.center, arc.radius, P0, P2, angle, points);
                }
                else
                {
                    // The subtended angle is in [pi/2,pi].
                    SampleArc2(arc.center, arc.radius, P0, P2, angle, points);
                }
            }
            else
            {
                // The subtended angle is in [pi,2*pi].
                if (dot <= zero)
                {
                    // The subtended angle is in [pi,3*pi/2].
                    SampleArc3(arc.center, arc.radius, P0, P2, twoPi - angle, points);
                }
                else
                {
                    // The subtended angle is in [3*pi/2,2*pi].
                    SampleArc4(arc.center, arc.radius, P0, P2, twoPi - angle, points);
                }
            }
        }

    private:
        void SampleArc1(
            Vector2<T> const& center, T radius, Vector2<T> const& P0, Vector2<T> const& P2,
            T angle, std::vector<Vector2<T>>& points)
        {
            double dNumPoints = static_cast<double>(radius * angle);
            std::size_t numPoints = static_cast<std::size_t>(dNumPoints);
            points.resize(numPoints);
            SampleAcuteArc(center, radius, P0, P2, numPoints, points.data());
        }

        void SampleArc2(
            Vector2<T> const& center, T radius, Vector2<T> const& P0, Vector2<T> const& P2,
            T angle, std::vector<Vector2<T>>& points)
        {
            T const two = static_cast<T>(2);
            double dNumPoints = static_cast<double>(radius * angle / two);
            std::size_t numPoints = static_cast<std::size_t>(dNumPoints);
            points.resize(2 * numPoints);
            Vector2<T> bisector0 = P0 + P2;
            (void)Normalize(bisector0);
            SampleAcuteArc(center, radius, P0, bisector0, numPoints, points.data());
            SampleAcuteArc(center, radius, bisector0, P2, numPoints, points.data() + numPoints);
        }

        void SampleArc3(
            Vector2<T> const& center, T radius, Vector2<T> const& P0, Vector2<T> const& P2,
            T angle, std::vector<Vector2<T>>& points)
        {
            T const two = static_cast<T>(2);
            T const three = static_cast<T>(3);
            double dNumPoints = static_cast<double>(radius * angle / three);
            std::size_t numPoints = static_cast<std::size_t>(dNumPoints);
            points.resize(3 * numPoints);
            Vector2<T> trisector0 = -(two * P0 + P2);
            (void)Normalize(trisector0);
            Vector2<T> trisector1 = -(P0 + two * P2);
            (void)Normalize(trisector1);
            SampleAcuteArc(center, radius, P0, trisector0, numPoints, points.data());
            SampleAcuteArc(center, radius, trisector0, trisector1, numPoints, points.data() + numPoints);
            SampleAcuteArc(center, radius, trisector1, P2, numPoints, points.data() + 2 * numPoints);
        }

        void SampleArc4(
            Vector2<T> const& center, T radius, Vector2<T> const& P0, Vector2<T> const& P2,
            T angle, std::vector<Vector2<T>>& points)
        {
            T const three = static_cast<T>(3);
            T const four = static_cast<T>(4);
            double dNumPoints = static_cast<double>(radius * angle / four);
            std::size_t numPoints = static_cast<std::size_t>(dNumPoints);
            points.resize(4 * numPoints);
            Vector2<T> quadsector0 = -(three * P0 + P2);
            (void)Normalize(quadsector0);
            Vector2<T> quadsector1 = -(P0 + P2);
            (void)Normalize(quadsector1);
            Vector2<T> quadsector2 = -(P0 + three * P2);
            (void)Normalize(quadsector2);
            SampleAcuteArc(center, radius, P0, quadsector0, numPoints, points.data());
            SampleAcuteArc(center, radius, quadsector0, quadsector1, numPoints, points.data() + numPoints);
            SampleAcuteArc(center, radius, quadsector1, quadsector2, numPoints, points.data() + 2 * numPoints);
            SampleAcuteArc(center, radius, quadsector2, P2, numPoints, points.data() + 3 * numPoints);
        }

        // The preconditions are:
        // 1. The arc is on the unit circle centered at the origin.
        // 2. The angle subtended by the arc is in the interval (0,pi/2].
        // 3. The memory referenced by 'points' has enough storage for the
        //    requested number of samples.
        void SampleAcuteArc(
            Vector2<T> const& center, T radius, Vector2<T> const& P0, Vector2<T> const& P2,
            std::size_t numPoints, Vector2<T>* points)
        {
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);

            // Translate the center of the circle of the arc to the origin
            // and scale the circle to have radius 1. The ordered points of
            // the arc are {P0,P1,P2}.
            Vector2<T> P1 = Perp(P2 - P0) / DotPerp(P0, P2);

            // Compute the NURBS weights for the parameterization. The weights
            // w1 = 1 and w2 = w0.
            T w0 = std::sqrt(two * (Dot(P1, P1) - one) / (one - Dot(P0, P2)));

            // Compute the NURBS control points for the parameterization.
            Vector2<T> C0 = center + radius * P0;
            Vector2<T> C1 = center + radius * P1;
            Vector2<T> C2 = center + radius * P2;

            // Compute the samples for u in [0,1].
            for (std::size_t i = 0; i < numPoints; ++i)
            {
                T u = static_cast<T>(i) / static_cast<T>(numPoints);
                T onemu = one - u;
                T k0 = w0 * onemu * onemu;
                T k1 = two * u * onemu;
                T k2 = w0 * u * u;
                *points++ = (k0 * C0 + k1 * C1 + k2 * C2) / (k0 + k1 + k2);
            }
        }
    };
}


