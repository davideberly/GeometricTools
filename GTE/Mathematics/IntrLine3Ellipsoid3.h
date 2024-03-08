// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the ellipsoid to be a solid.
//
// The ellipsoid is (X-C)^T*M*(X-C)-1 = 0 and the line is X = P+t*D.
// Substitute the line equation into the ellipsoid equation to obtain a
// quadratic equation Q(t) = a2*t^2 + 2*a1*t + a0 = 0, where a2 = D^T*M*D,
// a1 = D^T*M*(P-C) and a0 = (P-C)^T*M*(P-C)-1. The algorithm involves an
// analysis of the real-valued roots of Q(t) for all real t.

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/Line.h>
#include <Mathematics/Matrix3x3.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line3<T>, Ellipsoid3<T>>
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

        Result operator()(Line3<T> const& line, Ellipsoid3<T> const& ellipsoid)
        {
            Result result{};

            Matrix3x3<T> M{};
            ellipsoid.GetM(M);
            Vector3<T> diff = line.origin - ellipsoid.center;
            Vector3<T> matDir = M * line.direction;
            Vector3<T> matDiff = M * diff;
            T a2 = Dot(line.direction, matDir);
            T a1 = Dot(line.direction, matDiff);
            T a0 = Dot(diff, matDiff) - static_cast<T>(1);

            // An intersection occurs when Q(t) has real roots.
            T discr = a1 * a1 - a0 * a2;
            result.intersect = (discr >= static_cast<T>(0));
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line3<T>, Ellipsoid3<T>>
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

        Result operator()(Line3<T> const& line, Ellipsoid3<T> const& ellipsoid)
        {
            Result result{};
            DoQuery(line.origin, line.direction, ellipsoid, result);
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
            Vector3<T> const& lineDirection, Ellipsoid3<T> const& ellipsoid,
            Result& result)
        {
            Matrix3x3<T> M{};
            ellipsoid.GetM(M);
            Vector3<T> diff = lineOrigin - ellipsoid.center;
            Vector3<T> matDir = M * lineDirection;
            Vector3<T> matDiff = M * diff;
            T a2 = Dot(lineDirection, matDir);
            T a1 = Dot(lineDirection, matDiff);
            T a0 = Dot(diff, matDiff) - static_cast<T>(1);

            // Intersection occurs when Q(t) has real roots.
            T const zero = static_cast<T>(0);
            T discr = a1 * a1 - a0 * a2;
            if (discr > zero)
            {
                // The line intersects the ellipsoid in 2 distinct points.
                result.intersect = true;
                result.numIntersections = 2;
                T root = std::sqrt(discr);
                result.parameter[0] = (-a1 - root) / a2;
                result.parameter[1] = (-a1 + root) / a2;
            }
            else if (discr == zero)
            {
                // The line is tangent to the ellipsoid, so the intersection
                // is a single point. The parameter[1] value is set, because
                // callers will access the degenerate interval
                // [-a1/a2, -a1/a2].
                result.intersect = true;
                result.numIntersections = 1;
                result.parameter[0] = -a1 / a2;
                result.parameter[1] = result.parameter[0];
            }
            // else:  The line is outside the ellipsoid, no intersection.
        }
    };
}
