// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a ray and a solid canonical box in 3D.
// 
// The ray is P + t * D for t >= 0, where D is not required to be unit length.
// 
// The canonical box has center at the origin and is aligned with the
// coordinate axes. The extents are E = (e[0],e[1],e[2]). A box point is
// Y = (y[0],y[1],y[2]) with |y[i]| <= e[i] for all i.
// 
// The closest point on the ray is stored in closest[0] with parameter t. The
// closest point on the box is stored in closest[1]. When there are infinitely
// many choices for the pair of closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/3D/DistLine3CanonicalBox3.h>
#include <GTL/Mathematics/Distance/ND/DistPointCanonicalBox.h>
#include <GTL/Mathematics/Primitives/ND/Ray.h>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Ray3<T>, CanonicalBox3<T>>
    {
    public:
        using LBQuery = DCPQuery<T, Line3<T>, CanonicalBox3<T>>;
        using Output = typename LBQuery::Output;

        Output operator()(Ray3<T> const& ray, CanonicalBox3<T> const& box)
        {
            Output output{};

            Line3<T> line(ray.origin, ray.direction);
            LBQuery lbQuery{};
            auto lbOutput = lbQuery(line, box);
            if (lbOutput.parameter >= C_<T>(0))
            {
                output = lbOutput;
            }
            else
            {
                DCPQuery<T, Vector3<T>, CanonicalBox3<T>> pbQuery{};
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
