// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a ray and a solid aligned box in 2D.
// 
// The ray is P + t * D for t >= 0, where D is not required to be unit length.
// 
// The aligned box has minimum corner A and maximum corner B. A box point is X
// where A <= X <= B; the comparisons are componentwise.
// 
// The closest point on the ray is stored in closest[0] with parameter t. The
// closest point on the box is stored in closest[1]. When there are infinitely
// many choices for the pair of closest points, only one of them is returned.

#include <Mathematics/DistLine2AlignedBox2.h>
#include <Mathematics/DistPointAlignedBox.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Ray2<T>, AlignedBox2<T>>
    {
    public:
        using AlignedQuery = DCPQuery<T, Line2<T>, AlignedBox2<T>>;
        using Result = typename AlignedQuery::Result;

        Result operator()(Ray2<T> const& ray, AlignedBox2<T> const& box)
        {
            Result result{};

            Line2<T> line(ray.origin, ray.direction);
            AlignedQuery lbQuery{};
            auto lbResult = lbQuery(line, box);
            T const zero = static_cast<T>(0);
            if (lbResult.parameter >= zero)
            {
                result = lbResult;
            }
            else
            {
                DCPQuery<T, Vector2<T>, AlignedBox2<T>> pbQuery{};
                auto pbResult = pbQuery(ray.origin, box);
                result.distance = pbResult.distance;
                result.sqrDistance = pbResult.sqrDistance;
                result.parameter = zero;
                result.closest[0] = ray.origin;
                result.closest[1] = pbResult.closest[1];
            }

            return result;
        }
    };
}
