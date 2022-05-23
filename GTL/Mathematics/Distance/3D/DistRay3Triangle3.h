// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a ray and a triangle in 3D.
// 
// The ray is P + t * D for t >= 0, where D is not required to be unit length.
// 
// The triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The closest point on the ray is stored in closest[0] with parameter t. The
// closest point on the triangle is closest[1] with barycentric coordinates
// (b[0],b[1],b[2]). When there are infinitely many choices for the pair of
// closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/3D/DistLine3Triangle3.h>
#include <GTL/Mathematics/Distance/ND/DistPointTriangle.h>
#include <GTL/Mathematics/Primitives/ND/Ray.h>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Ray3<T>, Triangle3<T>>
    {
    public:
        using LTQuery = DCPQuery<T, Line3<T>, Triangle3<T>>;
        using Output = typename LTQuery::Output;

        Output operator()(Ray3<T> const& ray, Triangle3<T> const& triangle)
        {
            Output output{};

            Line3<T> line(ray.origin, ray.direction);
            LTQuery ltQuery{};
            auto ltResult = ltQuery(line, triangle);
            if (ltResult.parameter >= C_<T>(0))
            {
                output = ltResult;
            }
            else
            {
                DCPQuery<T, Vector3<T>, Triangle3<T>> ptQuery{};
                auto ptResult = ptQuery(ray.origin, triangle);
                output.distance = ptResult.distance;
                output.sqrDistance = ptResult.sqrDistance;
                output.parameter = C_<T>(0);
                output.barycentric = ptResult.barycentric;
                output.closest[0] = ray.origin;
                output.closest[1] = ptResult.closest[1];
            }
            return output;
        }
    };
}
