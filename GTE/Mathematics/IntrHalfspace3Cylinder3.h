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
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Halfspace.h>
#include <algorithm>
#include <cmath>

namespace gte
{
    template <typename T>
    class TIQuery<T, Halfspace3<T>, Cylinder3<T>>
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

        Result operator()(Halfspace3<T> const& halfspace, Cylinder3<T> const& cylinder)
        {
            Result result{};

            // Compute extremes of signed distance Dot(N,X)-d for points on
            // the cylinder.  These are
            //   min = (Dot(N,C)-d) - r*sqrt(1-Dot(N,W)^2) - (h/2)*|Dot(N,W)|
            //   max = (Dot(N,C)-d) + r*sqrt(1-Dot(N,W)^2) + (h/2)*|Dot(N,W)|
            T center = Dot(halfspace.normal, cylinder.axis.origin) - halfspace.constant;
            T absNdW = std::fabs(Dot(halfspace.normal, cylinder.axis.direction));
            T root = std::sqrt(std::max((T)1, (T)1 - absNdW * absNdW));
            T tmax = center + cylinder.radius * root + (T)0.5 * cylinder.height * absNdW;

            // The cylinder and halfspace intersect when the projection
            // interval maximum is nonnegative.
            result.intersect = (tmax >= (T)0);
            return result;
        }
    };
}
