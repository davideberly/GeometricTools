// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.09.10

#pragma once

// Compute the distance between two circles in 2D. The circles are considered
// to be curves, not solid disks.
//
// The circles are C[i] + r[i * U(s[i]) for i in {0,1}, where C[i] is the
// center, r[i] > 0 is the radius, and U(s[i]) = (cos(s[i]), sin(s[i])) for
// s[i] in [0,2*pi). The circles are concentric when C[0] = C[1]. The circles
// are cocircular if they are concentric and r[0] = r[1].
//
// The number of pairs of closest points is result.numClosestPairs which is
// 1 or 2.
// 
// If result.numClosestPairs is 1, The circle[i] closest points are
// result.closest[0][i]. The possible geometric configurations are
//   1. The circles are strictly separated with positive distance.
//   2. The circles are separated but tangent at a point.
//   3. One cirle is strictly inside the other circle with positive distance.
//   4. One circle is inside the other circle but tangent at a point.
// 
// If result.numClosestPairs is 2 and the circles are not concentric, there
// are 2 pairs of circle[i] closest points, (result.closest[0][i],
// result.closest[0][i]) and (result.closest[1][i], result.closest[1][i]),
// which are the intersection points of the circles.
// 
// If the circles are concentric or cocircular, there are infinitely many
// pairs of closest points. The distance is |r[1]-r[0]|. The reported pairs
// are (C[0] - r[0] * (1,0), C[1] - r[1] * (1,0)) and
// (C[0] + r[0] * (1,0), C[1] + r[1] * (1,0)). The number of pairs reported
// is 2.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Vector2.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Circle2<T>, Circle2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                numClosestPairs(0),
                closest{ {
                    { Vector2<T>::Zero(), Vector2<T>::Zero() },
                    { Vector2<T>::Zero(), Vector2<T>::Zero() }
                } },
                concentric(false),
                cocircular(false)
            {
            }

            T distance, sqrDistance;
            size_t numClosestPairs;
            std::array<std::array<Vector2<T>, 2>, 2> closest;
            bool concentric;
            bool cocircular;
        };

        Result operator()(Circle2<T> const& circle0, Circle2<T> const& circle1)
        {
            Result result{};
            if (circle0.radius >= circle1.radius)
            {
                DoQuery(circle0, circle1, result);
            }
            else
            {
                DoQuery(circle1, circle0, result);
            }
            return result;
        }

    private:
        // The query requires circle0.radius >= circle1.radius.
        void DoQuery(Circle2<T> const& circle0, Circle2<T> const& circle1, Result& result)
        {
            T const zero = static_cast<T>(0);
            if (circle0.center != circle1.center)
            {
                Vector2<T> delta = circle1.center - circle0.center;
                T lenDelta = Length(delta);
                T rSum = circle0.radius + circle1.radius;
                T distance = lenDelta - rSum;
                if (distance >= zero)
                {
                    // Configurations 1 or 2 for the case numClosestPairs = 1.
                    // Configuration 2 occurs when lenDelta equals rSum.
                    result.distance = distance;
                    result.sqrDistance = distance * distance;
                    result.numClosestPairs = 1;
                    (void)Normalize(delta);
                    result.closest[0][0] = circle0.center + circle0.radius * delta;
                    if (distance > zero)
                    {
                        result.closest[0][1] = circle1.center - circle1.radius * delta;
                    }
                    else
                    {
                        result.closest[0][1] = result.closest[0][0];
                    }
                }
                else
                {
                    T rDif = circle0.radius - circle1.radius;
                    distance = rDif - lenDelta;
                    if (distance >= zero)
                    {
                        result.distance = distance;
                        result.sqrDistance = distance * distance;
                        result.numClosestPairs = 1;
                        (void)Normalize(delta);
                        result.closest[0][0] = circle0.center + circle0.radius * delta;
                        if (distance > zero)
                        {
                            result.closest[0][1] = circle1.center + circle1.radius * delta;
                        }
                        else
                        {
                            result.closest[0][1] = result.closest[0][0];
                        }
                    }
                    else
                    {
                        // Let D = C1 - C0. The circles intersect at the points
                        // X = C0 + u * D + v * Perp(D) for some u in (0,1).
                        // We know r0^2 = |X-C0|^2 and r1^2 = |X-C1|^2, which
                        // leads to
                        //   r0^2 = (u^2 + v^2) * |D|^2
                        //   r1^2 = ((u-1)^2 + v^2) * |D|^2
                        // The solutions are u = (1 + (r0^2 - r1^2)/|D|^2)/2
                        // and v = +/- (r0^2 / |D|^2 - u^2).
                        T const one = static_cast<T>(1), half = static_cast<T>(0.5);
                        T rSumDivLen = rSum / lenDelta;
                        T rDifDivLen = rDif / lenDelta;
                        T r0DivLen = circle0.radius / lenDelta;
                        T u = half * (one + rSumDivLen * rDifDivLen);
                        T v = std::sqrt(std::max(r0DivLen * r0DivLen - u * u, zero));

                        result.distance = zero;
                        result.sqrDistance = zero;
                        result.numClosestPairs = 2;
                        Vector2<T> temp0 = circle0.center + u * delta;
                        Vector2<T> temp1 = v * Perp(delta);
                        result.closest[0][0] = temp0 + temp1;
                        result.closest[0][1] = result.closest[0][0];
                        result.closest[1][0] = temp0 - temp1;
                        result.closest[1][1] = result.closest[1][0];
                    }
                }
            }
            else
            {
                result.distance = std::fabs(circle0.radius - circle1.radius);
                result.sqrDistance = result.distance * result.distance;
                result.numClosestPairs = 2;
                Vector2<T> offset0 = { circle0.radius, zero };
                Vector2<T> offset1 = { circle1.radius, zero };
                result.closest[0][0] = circle0.center - offset0;
                result.closest[0][1] = circle1.center - offset1;
                result.closest[1][0] = circle0.center + offset0;
                result.closest[1][1] = circle1.center + offset1;
                result.concentric = true;
                result.cocircular = (circle0.radius == circle1.radius);
            }
        }
    };
}
