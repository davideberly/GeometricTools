// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Line.h>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Line3<Real>, Sphere3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
        };

        Result operator()(Line3<Real> const& line, Sphere3<Real> const& sphere)
        {
            // The sphere is (X-C)^T*(X-C)-1 = 0 and the line is X = P+t*D.
            // Substitute the line equation into the sphere equation to
            // obtain a quadratic equation Q(t) = t^2 + 2*a1*t + a0 = 0, where
            // a1 = D^T*(P-C) and a0 = (P-C)^T*(P-C)-1.
            Result result;

            Vector3<Real> diff = line.origin - sphere.center;
            Real a0 = Dot(diff, diff) - sphere.radius * sphere.radius;
            Real a1 = Dot(line.direction, diff);

            // Intersection occurs when Q(t) has real roots.
            Real discr = a1 * a1 - a0;
            result.intersect = (discr >= (Real)0);
            return result;
        }
    };

    template <typename Real>
    class FIQuery<Real, Line3<Real>, Sphere3<Real>>
    {
    public:
        struct Result
        {
            bool intersect;
            int numIntersections;
            std::array<Real, 2> parameter;
            std::array<Vector3<Real>, 2> point;
        };

        Result operator()(Line3<Real> const& line, Sphere3<Real> const& sphere)
        {
            Result result;
            DoQuery(line.origin, line.direction, sphere, result);
            for (int i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = line.origin + result.parameter[i] * line.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<Real> const& lineOrigin,
            Vector3<Real> const& lineDirection, Sphere3<Real> const& sphere,
            Result& result)
        {
            // The sphere is (X-C)^T*(X-C)-1 = 0 and the line is X = P+t*D.
            // Substitute the line equation into the sphere equation to
            // obtain a quadratic equation Q(t) = t^2 + 2*a1*t + a0 = 0, where
            // a1 = D^T*(P-C) and a0 = (P-C)^T*(P-C)-1.
            Vector3<Real> diff = lineOrigin - sphere.center;
            Real a0 = Dot(diff, diff) - sphere.radius * sphere.radius;
            Real a1 = Dot(lineDirection, diff);

            // Intersection occurs when Q(t) has real roots.
            Real discr = a1 * a1 - a0;
            if (discr > (Real)0)
            {
                result.intersect = true;
                result.numIntersections = 2;
                Real root = std::sqrt(discr);
                result.parameter[0] = -a1 - root;
                result.parameter[1] = -a1 + root;
            }
            else if (discr < (Real)0)
            {
                result.intersect = false;
                result.numIntersections = 0;
            }
            else
            {
                result.intersect = true;
                result.numIntersections = 1;
                result.parameter[0] = -a1;
            }
        }
    };
}
