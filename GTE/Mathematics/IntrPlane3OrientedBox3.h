// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/DistPointHyperplane.h>
#include <Mathematics/OrientedBox.h>
#include <cmath>

namespace gte
{
    template <typename T>
    class TIQuery<T, Plane3<T>, OrientedBox3<T>>
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

        Result operator()(Plane3<T> const& plane, OrientedBox3<T> const& box)
        {
            Result result{};

            T radius =
                std::fabs(box.extent[0] * Dot(plane.normal, box.axis[0])) +
                std::fabs(box.extent[1] * Dot(plane.normal, box.axis[1])) +
                std::fabs(box.extent[2] * Dot(plane.normal, box.axis[2]));

            DCPQuery<T, Vector3<T>, Plane3<T>> ppQuery;
            auto ppResult = ppQuery(box.center, plane);
            result.intersect = (ppResult.distance <= radius);
            return result;
        }
    };
}
