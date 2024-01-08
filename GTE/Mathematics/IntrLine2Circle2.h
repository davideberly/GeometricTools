// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.09.28

#pragma once

// The queries consider the circle to be a solid (disk).

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/DistPointLine.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line2<T>, Circle2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Line2<T> const& line, Circle2<T> const& circle)
        {
            Result result{};
            DCPQuery<T, Vector2<T>, Line2<T>> plQuery;
            auto plResult = plQuery(circle.center, line);
            result.intersect = (plResult.distance <= circle.radius);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line2<T>, Circle2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                parameter{ (T)0, (T)0 },
                point{ Vector2<T>::Zero(), Vector2<T>::Zero() }
            {
            }

            bool intersect;
            int32_t numIntersections;
            std::array<T, 2> parameter;
            std::array<Vector2<T>, 2> point;
        };

        Result operator()(Line2<T> const& line, Circle2<T> const& circle)
        {
            Result result{};
            DoQuery(line.origin, line.direction, circle, result);
            for (int32_t i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = line.origin + result.parameter[i] * line.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector2<T> const& lineOrigin,
            Vector2<T> const& lineDirection, Circle2<T> const& circle,
            Result& result)
        {
            // Intersection of a the line P+t*D and the circle |X-C| = R.
            // The line direction is unit length. The t-value is a
            // real-valued root to the quadratic equation
            //   0 = |t*D+P-C|^2 - R^2
            //     = t^2 + 2*Dot(D,P-C)*t + |P-C|^2-R^2
            //     = t^2 + 2*a1*t + a0
            // If there are two distinct roots, the order is t0 < t1.
            Vector2<T> diff = lineOrigin - circle.center;
            T a0 = Dot(diff, diff) - circle.radius * circle.radius;
            T a1 = Dot(lineDirection, diff);
            T discr = a1 * a1 - a0;
            if (discr > (T)0)
            {
                T root = std::sqrt(discr);
                result.intersect = true;
                result.numIntersections = 2;
                result.parameter[0] = -a1 - root;
                result.parameter[1] = -a1 + root;
            }
            else if (discr < (T)0)
            {
                result.intersect = false;
                result.numIntersections = 0;
            }
            else  // discr == 0
            {
                // The line is tangent to the circle. Set the parameters to
                // the same number because other queries involving linear
                // components and circular components use interval-interval
                // intersection tests which consume both parameters.
                result.intersect = true;
                result.numIntersections = 1;
                result.parameter[0] = -a1;
                result.parameter[1] = -a1;
            }
        }
    };
}
