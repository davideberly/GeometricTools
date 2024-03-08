// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a ray and a solid orienteded box in 3D.
// 
// The ray is P + t * D for t >= 0, where D is not required to be unit length.
// 
// The oriented box has center C, unit-length axis directions U[i] and extents
// e[i] for all i. A box point is X = C + sum_i y[i] * U[i], where
// |y[i]| <= e[i] for all i.
// 
// The closest point on the ray is stored in closest[0] with parameter t. The
// closest point on the box is stored in closest[1]. When there are infinitely
// many choices for the pair of closest points, only one of them is returned.

#include <Mathematics/DistLine3OrientedBox3.h>
#include <Mathematics/DistPointOrientedBox.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Ray3<T>, OrientedBox3<T>>
    {
    public:
        using OrientedQuery = DCPQuery<T, Line3<T>, OrientedBox3<T>>;
        using Result = typename OrientedQuery::Result;

        Result operator()(Ray3<T> const& ray, OrientedBox3<T> const& box)
        {
            Result result{};

            Line3<T> line(ray.origin, ray.direction);
            OrientedQuery lbQuery{};
            auto lbOutput = lbQuery(line, box);
            T const zero = static_cast<T>(0);
            if (lbOutput.parameter >= zero)
            {
                result = lbOutput;
            }
            else
            {
                DCPQuery<T, Vector3<T>, OrientedBox3<T>> pbQuery{};
                auto pbOutput = pbQuery(ray.origin, box);
                result.distance = pbOutput.distance;
                result.sqrDistance = pbOutput.sqrDistance;
                result.parameter = zero;
                result.closest[0] = ray.origin;
                result.closest[1] = pbOutput.closest[1];
            }
            return result;
        }
    };
}
