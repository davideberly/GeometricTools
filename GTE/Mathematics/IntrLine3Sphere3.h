// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the sphere to be a solid.
//
// The sphere is (X-C)^T*(X-C)-r^2 = 0 and the line is X = P+t*D. Substitute
// the line equation into the sphere equation to obtain a quadratic equation
// Q(t) = t^2 + 2*a1*t + a0 = 0, where a1 = D^T*(P-C) and
// a0 = (P-C)^T*(P-C)-r^2. The algorithm involves an analysis of the
// real-valued roots of Q(t) for all real t.

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Line.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line3<T>, Sphere3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            };

            bool intersect;
        };

        Result operator()(Line3<T> const& line, Sphere3<T> const& sphere)
        {
            Result result{};

            Vector3<T> diff = line.origin - sphere.center;
            T a0 = Dot(diff, diff) - sphere.radius * sphere.radius;
            T a1 = Dot(line.direction, diff);

            // An intersection occurs when Q(t) has real roots.
            T discr = a1 * a1 - a0;
            result.intersect = (discr >= static_cast<T>(0));
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line3<T>, Sphere3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                parameter{ static_cast<T>(0), static_cast<T>(0) },
                point{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            bool intersect;
            size_t numIntersections;
            std::array<T, 2> parameter;
            std::array<Vector3<T>, 2> point;
        };

        Result operator()(Line3<T> const& line, Sphere3<T> const& sphere)
        {
            Result result{};
            DoQuery(line.origin, line.direction, sphere, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = line.origin + result.parameter[i] * line.direction;
                }
            }
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector3<T> const& lineOrigin,
            Vector3<T> const& lineDirection, Sphere3<T> const& sphere,
            Result& result)
        {
            Vector3<T> diff = lineOrigin - sphere.center;
            T a0 = Dot(diff, diff) - sphere.radius * sphere.radius;
            T a1 = Dot(lineDirection, diff);

            // Intersection occurs when Q(t) has real roots.
            T const zero = static_cast<T>(0);
            T discr = a1 * a1 - a0;
            if (discr > zero)
            {
                // The line intersects the sphere in 2 distinct points.
                result.intersect = true;
                result.numIntersections = 2;
                T root = std::sqrt(discr);
                result.parameter[0] = -a1 - root;
                result.parameter[1] = -a1 + root;
            }
            else if (discr == zero)
            {
                // The line is tangent to the sphere, so the intersection is
                // a single point. The parameter[1] value is set, because
                // callers will access the degenerate interval [-a1,-a1].
                result.intersect = true;
                result.numIntersections = 1;
                result.parameter[0] = -a1;
                result.parameter[1] = result.parameter[0];
            }
            // else:  The line is outside the sphere, no intersection.
        }
    };
}
