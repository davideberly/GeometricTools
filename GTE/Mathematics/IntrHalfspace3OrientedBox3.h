// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Queries for intersection of objects with halfspaces.  These are useful for
// containment testing, object culling, and clipping.

#include <Mathematics/TIQuery.h>
#include <Mathematics/Halfspace.h>
#include <Mathematics/OrientedBox.h>
#include <cmath>

namespace gte
{
    template <typename T>
    class TIQuery<T, Halfspace3<T>, OrientedBox3<T>>
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

        Result operator()(Halfspace3<T> const& halfspace, OrientedBox3<T> const& box)
        {
            Result result{};

            // Project the box center onto the normal line.  The plane of the
            // halfspace occurs at the origin (zero) of the normal line.
            T center = Dot(halfspace.normal, box.center) - halfspace.constant;

            // Compute the radius of the interval of projection.
            T radius =
                std::fabs(box.extent[0] * Dot(halfspace.normal, box.axis[0])) +
                std::fabs(box.extent[1] * Dot(halfspace.normal, box.axis[1])) +
                std::fabs(box.extent[2] * Dot(halfspace.normal, box.axis[2]));

            // The box and halfspace intersect when the projection interval
            // maximum is nonnegative.
            result.intersect = (center + radius >= (T)0);
            return result;
        }
    };
}
