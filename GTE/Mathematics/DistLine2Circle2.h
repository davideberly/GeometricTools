// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.28

#pragma once

// Compute the distance between a line and a circle in 2D. The circle is
// considered to be a curve, not a solid disk.
//
// The line is P + t * D, where P is a point on the line and D is not required
// to be unit length. The t-value is any real number.
//
// The circle is C + r * U(s), where C is the center, r > 0 is the radius,
// and U(s) = (cos(s), sin(s)) for s in [0,2*pi).
//
// The number of pairs of closest points is result.numClosestPairs which is
// 1 or 2. If result.numClosestPairs is 1, result.parameter[0] is the line
// t-value for its closest point result.closest[0][0]. The circle closest
// point is result.closest[0][1]. If result.numClosestPairs is 2,
// result.parameter[0] and result.parameter[1] are the line t-values for its
// closest points result.closest[0][0] and result.closest[1][0]. The circle
// closest points are result.closest[0][1] and result.closest[1][1].

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Line.h>
#include <Mathematics/Vector2.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line2<T>, Circle2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                numClosestPairs(0),
                parameter{ static_cast<T>(0), static_cast<T>(0) },
                closest{ {
                    { Vector2<T>::Zero(), Vector2<T>::Zero() },
                    { Vector2<T>::Zero(), Vector2<T>::Zero() }
                } }
            {
            }

            T distance, sqrDistance;
            size_t numClosestPairs;
            std::array<T, 2> parameter;
            std::array<std::array<Vector2<T>, 2>, 2> closest;
        };

        Result operator()(Line2<T> const& line, Circle2<T> const& circle)
        {
            Result result{};

            // Translate the line and circle so that the circle has center at
            // the origin.
            Vector2<T> delta = line.origin - circle.center;

            // The query computes 'result' relative to the circle with center
            // at the origin.
            DoQuery(delta, line.direction, circle.radius, result);

            // Translate the closest points to the original coordinates and
            // the compute the distance and squared distance.
            for (size_t j = 0; j < result.numClosestPairs; ++j)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.closest[j][i] += circle.center;
                }
            }

            Vector2<T> diff = result.closest[0][0] - result.closest[0][1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

    protected:
        // Compute the distance and closest point between a line and an
        // aligned box whose center is the origin.
        static void DoQuery(Vector2<T> const& delta, Vector2<T> const& direction,
            T const& radius, Result& result)
        {
            // Compute the distance from the line to the origin. The distance
            // is d = |Dot(D,Perp(D))|/|D|. The line does not intersect the
            // circle when d > r. The line is tangent to the circle when
            // d = r. The line intersects the circle in 2 points when d < r.
            // Rather than normalize D at this time, replace the comparisons
            // by sign tests for |Dot(D,Perp(D))|^2 - r^2 * |D|^2. This allows
            // for theoretically correct classification of line-circle
            // tangency when using rational arithmetic.
            T const zero = static_cast<T>(0);
            T dotDirDir = Dot(direction, direction);
            T dotDirDel = Dot(direction, delta);
            T dotPerpDirDel = DotPerp(direction, delta);
            T rSqr = radius * radius;
            T test = dotPerpDirDel * dotPerpDirDel - rSqr * dotDirDir;
            if (test >= zero)
            {
                // When the line-origin distance equals the radius, the line
                // is tangent to the circle; there is 1 point of intersection
                // and the line-circle distance is 0. When the line-origin
                // distance is larger than the radius, the line and circle do
                // not intersect. The closest circle point is the tangent
                // point if the line were to be translated in its normal
                // direction to just touch the circle. In this case, the
                // distance between the circle and line is the difference
                // between the line-origin distance and the radius.

                // Compute the line point closest to the circle.
                result.numClosestPairs = 1;
                result.parameter[0] = -dotDirDel / dotDirDir;
                result.closest[0][0] = delta + result.parameter[0] * direction;
                result.closest[0][1] = result.closest[0][0];

                // Compute the circle point closest to the line.
                if (test > zero)
                {
                    Normalize(result.closest[0][1]);
                    result.closest[0][1] *= radius;
                }
            }
            else // lineOriginDistance < radius
            {
                // The line and circle intersect in 2 points. Solve the
                // quadratic equation a2*t^2 + 2*a1*t + a0 = 0. The solutions
                // are (-a1 +/- sqrt(a1 * a1 - a0 * a2)) / a2. Theoretically,
                // discr > 0. Guard against a negative floating-point result.
                T a0 = Dot(delta, delta) - radius * radius;
                T a1 = dotDirDel;
                T a2 = dotDirDir;
                T discr = std::max(a1 * a1 - a0 * a2, zero);
                T sqrtDiscr = std::sqrt(discr);

                // Evaluate the line parameters but do so to avoid subtractive
                // cancellation.
                T temp = -dotDirDel + (dotDirDel > zero ? -sqrtDiscr : sqrtDiscr);
                result.numClosestPairs = 2;
                result.parameter[0] = temp / dotDirDir;
                result.parameter[1] = a0 / temp;
                if (result.parameter[0] > result.parameter[1])
                {
                    std::swap(result.parameter[0], result.parameter[1]);
                }

                // Compute the intersection points.
                result.closest[0][0] = delta + result.parameter[0] * direction;
                result.closest[0][1] = result.closest[0][0];
                result.closest[1][0] = delta + result.parameter[1] * direction;
                result.closest[1][1] = result.closest[1][0];
            }
        }
    };
}
