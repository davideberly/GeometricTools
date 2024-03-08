// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/DistSegmentSegment.h>
#include <Mathematics/Capsule.h>

namespace gte
{
    template <typename T>
    class TIQuery<T, Capsule3<T>, Capsule3<T>>
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

        Result operator()(Capsule3<T> const& capsule0, Capsule3<T> const& capsule1)
        {
            Result result{};
            DCPQuery<T, Segment3<T>, Segment3<T>> ssQuery;
            auto ssResult = ssQuery(capsule0.segment, capsule1.segment);
            T rSum = capsule0.radius + capsule1.radius;
            result.intersect = (ssResult.distance <= rSum);
            return result;
        }
    };
}
