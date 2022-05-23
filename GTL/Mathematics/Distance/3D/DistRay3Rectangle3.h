// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a ray and a solid rectangle in 3D.
// 
// The ray is P + t * D for t >= 0, where D is not required to be unit length.
// 
// The rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
// 
// The closest point on the ray is stored in closest[0] with parameter t. The
// closest point on the rectangle is stored in closest[1] with U-coordinates
// (s[0],s[1]). When there are infinitely many choices for the pair of closest
// points, only one of them is returned.
//
// TODO: Modify to support non-unit-length W[].

#include <GTL/Mathematics/Distance/3D/DistLine3Rectangle3.h>
#include <GTL/Mathematics/Distance/ND/DistPointRectangle.h>
#include <GTL/Mathematics/Primitives/ND/Ray.h>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Ray3<T>, Rectangle3<T>>
    {
    public:
        using LRQuery = DCPQuery<T, Line3<T>, Rectangle3<T>>;
        using Output = typename LRQuery::Output;

        Output operator()(Ray3<T> const& ray, Rectangle3<T> const& rectangle)
        {
            Output output{};

            Line3<T> line(ray.origin, ray.direction);
            LRQuery lrQuery{};
            auto lrResult = lrQuery(line, rectangle);
            if (lrResult.parameter >= C_<T>(0))
            {
                output = lrResult;
            }
            else
            {
                DCPQuery<T, Vector3<T>, Rectangle3<T>> prQuery{};
                auto prResult = prQuery(ray.origin, rectangle);
                output.distance = prResult.distance;
                output.sqrDistance = prResult.sqrDistance;
                output.parameter = C_<T>(0);
                output.cartesian = prResult.cartesian;
                output.closest[0] = ray.origin;
                output.closest[1] = prResult.closest[1];
            }
            return output;
        }
    };
}
