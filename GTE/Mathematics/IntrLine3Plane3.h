// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/DistPointHyperplane.h>
#include <cstdint>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line3<T>, Plane3<T>>
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

        Result operator()(Line3<T> const& line, Plane3<T> const& plane)
        {
            Result result{};

            T DdN = Dot(line.direction, plane.normal);
            if (DdN != (T)0)
            {
                // The line is not parallel to the plane, so they must
                // intersect.
                result.intersect = true;
            }
            else
            {
                // The line and plane are parallel.
                DCPQuery<T, Vector3<T>, Plane3<T>> vpQuery;
                result.intersect = (vpQuery(line.origin, plane).distance == (T)0);
            }

            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line3<T>, Plane3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                parameter((T)0),
                point{ (T)0, (T)0, (T)0 }
            {
            }

            bool intersect;

            // The number of intersections is 0 (no intersection), 1 (linear
            // component and plane intersect in a point), or
            // std::numeric_limits<int32_t>::max() (linear component is on the
            // plane).  If the linear component is on the plane, 'point'
            // component's origin and 'parameter' is zero.
            int32_t numIntersections;
            T parameter;
            Vector3<T> point;
        };

        Result operator()(Line3<T> const& line, Plane3<T> const& plane)
        {
            Result result{};
            DoQuery(line.origin, line.direction, plane, result);
            if (result.intersect)
            {
                result.point = line.origin + result.parameter * line.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<T> const& lineOrigin,
            Vector3<T> const& lineDirection, Plane3<T> const& plane,
            Result& result)
        {
            T DdN = Dot(lineDirection, plane.normal);
            DCPQuery<T, Vector3<T>, Plane3<T>> vpQuery;
            auto vpResult = vpQuery(lineOrigin, plane);

            if (DdN != (T)0)
            {
                // The line is not parallel to the plane, so they must
                // intersect.
                result.intersect = true;
                result.numIntersections = 1;
                result.parameter = -vpResult.signedDistance / DdN;
            }
            else
            {
                // The line and plane are parallel.  Determine whether the
                // line is on the plane.
                if (vpResult.distance == (T)0)
                {
                    // The line is coincident with the plane, so choose t = 0
                    // for the parameter.
                    result.intersect = true;
                    result.numIntersections = std::numeric_limits<int32_t>::max();
                    result.parameter = (T)0;
                }
                else
                {
                    // The line is not on the plane.
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
        }
    };
}
