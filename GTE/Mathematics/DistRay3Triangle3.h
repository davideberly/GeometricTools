// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DistLine3Triangle3.h>
#include <Mathematics/DistPointTriangle.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Ray3<T>, Triangle3<T>>
    {
    public:
        using LTQuery = DCPQuery<T, Line3<T>, Triangle3<T>>;
        using Result = typename LTQuery::Result;

        Result operator()(Ray3<T> const& ray, Triangle3<T> const& triangle)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            Line3<T> line(ray.origin, ray.direction);
            LTQuery ltQuery{};
            auto ltResult = ltQuery(line, triangle);
            if (ltResult.parameter >= zero)
            {
                result = ltResult;
            }
            else
            {
                DCPQuery<T, Vector3<T>, Triangle3<T>> ptQuery{};
                auto ptResult = ptQuery(ray.origin, triangle);
                result.distance = ptResult.distance;
                result.sqrDistance = ptResult.sqrDistance;
                result.parameter = zero;
                result.barycentric = ptResult.barycentric;
                result.closest[0] = ray.origin;
                result.closest[1] = ptResult.closest[1];
            }
            return result;
        }
    };
}
