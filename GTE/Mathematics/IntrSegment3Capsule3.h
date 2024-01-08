// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the capsule to be a solid.
//
// The test-intersection queries are based on distance computations.

#include <Mathematics/DistSegmentSegment.h>
#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3Capsule3.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment3<T>, Capsule3<T>>
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

        Result operator()(Segment3<T> const& segment, Capsule3<T> const& capsule)
        {
            Result result{};
            DCPQuery<T, Segment3<T>, Segment3<T>> ssQuery{};
            auto ssResult = ssQuery(segment, capsule.segment);
            result.intersect = (ssResult.distance <= capsule.radius);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Segment3<T>, Capsule3<T>>
        :
        public FIQuery<T, Line3<T>, Capsule3<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line3<T>, Capsule3<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line3<T>, Capsule3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Segment3<T> const& segment, Capsule3<T> const& capsule)
        {
            Vector3<T> segOrigin{}, segDirection{};
            T segExtent{};
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result{};
            DoQuery(segOrigin, segDirection, segExtent, capsule, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = segOrigin + result.parameter[i] * segDirection;
                }
            }
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector3<T> const& segOrigin,
            Vector3<T> const& segDirection, T segExtent,
            Capsule3<T> const& capsule, Result& result)
        {
            FIQuery<T, Line3<T>, Capsule3<T>>::DoQuery(
                segOrigin, segDirection, capsule, result);

            if (result.intersect)
            {
                // The line containing the segment intersects the capsule; the
                // t-interval is [t0,t1]. The segment intersects the capsule
                // as long as [t0,t1] overlaps the segment t-interval
                // [-segExtent,+segExtent].
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                std::array<T, 2> segInterval = { -segExtent, segExtent };
                auto iiResult = iiQuery(result.parameter, segInterval);
                if (iiResult.intersect)
                {
                    result.numIntersections = iiResult.numIntersections;
                    result.parameter = iiResult.overlap;
                }
                else
                {
                    // The line containing the segment does not intersect the
                    // capsule.
                    result = Result{};
                }
            }
        }
    };
}
