// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a ray and a solid oriented box in 2D.
// 
// The ray is P + t * D for t >= 0, where D is not required to be unit length.
//
// The oriented box has center C, unit-length axis directions U[i] and extents
// e[i] for 0 <= i < N. A box point is X = C + sum_i y[i] * U[i], where
// |y[i]| <= e[i] for all i.
// 
// The closest point on the ray is stored in closest[0] with parameter t. The
// closest point on the box is stored in closest[1]. When there are infinitely
// many choices for the pair of closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/2D/DistLine2OrientedBox2.h>
#include <GTL/Mathematics/Distance/ND/DistPointOrientedBox.h>
#include <GTL/Mathematics/Primitives/ND/Ray.h>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Ray2<T>, OrientedBox2<T>>
    {
    public:
        using OrientedQuery = DCPQuery<T, Line2<T>, OrientedBox2<T>>;
        using Output = typename OrientedQuery::Output;

        Output operator()(Ray2<T> const& ray, OrientedBox2<T> const& box)
        {
            Output output{};

            Line2<T> line(ray.origin, ray.direction);
            OrientedQuery lbQuery{};
            auto lbResult = lbQuery(line, box);
            if (lbResult.parameter >= C_<T>(0))
            {
                output = lbResult;
            }
            else
            {
                DCPQuery<T, Vector2<T>, OrientedBox2<T>> pbQuery{};
                auto pbResult = pbQuery(ray.origin, box);
                output.distance = pbResult.distance;
                output.sqrDistance = pbResult.sqrDistance;
                output.parameter = C_<T>(0);
                output.closest[0] = ray.origin;
                output.closest[1] = pbResult.closest[1];
            }

            return output;
        }
    };
}
