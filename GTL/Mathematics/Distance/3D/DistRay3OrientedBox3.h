// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/3D/DistLine3OrientedBox3.h>
#include <GTL/Mathematics/Distance/ND/DistPointOrientedBox.h>
#include <GTL/Mathematics/Primitives/ND/Ray.h>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Ray3<T>, OrientedBox3<T>>
    {
    public:
        using OrientedQuery = DCPQuery<T, Line3<T>, OrientedBox3<T>>;
        using Output = typename OrientedQuery::Output;

        Output operator()(Ray3<T> const& ray, OrientedBox3<T> const& box)
        {
            Output output{};

            Line3<T> line(ray.origin, ray.direction);
            OrientedQuery lbQuery{};
            auto lbOutput = lbQuery(line, box);
            if (lbOutput.parameter >= C_<T>(0))
            {
                output = lbOutput;
            }
            else
            {
                DCPQuery<T, Vector3<T>, OrientedBox3<T>> pbQuery{};
                auto pbOutput = pbQuery(ray.origin, box);
                output.distance = pbOutput.distance;
                output.sqrDistance = pbOutput.sqrDistance;
                output.parameter = C_<T>(0);
                output.closest[0] = ray.origin;
                output.closest[1] = pbOutput.closest[1];
            }
            return output;
        }
    };
}
