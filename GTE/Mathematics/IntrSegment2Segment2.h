// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.11.11

#pragma once

#include <Mathematics/Segment.h>
#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2Line2.h>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment2<T>, Segment2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0)
            {
            }

            // The number is 0 (no intersection), 1 (segments intersect in a
            // single point), or 2 (segments are collinear and intersect in a
            // segment).
            bool intersect;
            int numIntersections;
        };

        Result operator()(Segment2<T> const& segment0, Segment2<T> const& segment1)
        {
            Result result{};
            Vector2<T> seg0Origin{}, seg0Direction{}, seg1Origin{}, seg1Direction{};
            T seg0Extent{}, seg1Extent{};
            segment0.GetCenteredForm(seg0Origin, seg0Direction, seg0Extent);
            segment1.GetCenteredForm(seg1Origin, seg1Direction, seg1Extent);

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            Line2<T> line0(seg0Origin, seg0Direction);
            Line2<T> line1(seg1Origin, seg1Direction);
            auto llResult = llQuery(line0, line1);
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the segments.
                if (std::fabs(llResult.line0Parameter[0]) <= seg0Extent &&
                    std::fabs(llResult.line1Parameter[0]) <= seg1Extent)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int>::max())
            {
                // Compute the location of segment1 endpoints relative to
                // segment0.
                Vector2<T> diff = seg1Origin - seg0Origin;
                T t = Dot(seg0Direction, diff);

                // Get the parameter intervals of the segments relative to
                // segment0.
                std::array<T, 2> interval0 = { -seg0Extent, seg0Extent };
                std::array<T, 2> interval1 = { t - seg1Extent, t + seg1Extent };

                // Compute the intersection of the intervals.
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery;
                auto iiResult = iiQuery(interval0, interval1);
                result.intersect = iiResult.intersect;
                result.numIntersections = iiResult.numIntersections;
            }
            else
            {
                result.intersect = false;
                result.numIntersections = 0;
            }

            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Segment2<T>, Segment2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                segment0Parameter{ (T)0, (T)0 },
                segment1Parameter{ (T)0, (T)0 },
                point{ Vector2<T>::Zero(), Vector2<T>::Zero() }
            {
            }

            // The number is 0 (no intersection), 1 (segments intersect in a
            // a single point), or 2 (segments are collinear and intersect
            // in a segment).
            bool intersect;
            int numIntersections;

            // If numIntersections is 1, the intersection is
            //   point[0]
            //   = segment0.origin + segment0Parameter[0] * segment0.direction
            //   = segment1.origin + segment1Parameter[0] * segment1.direction
            // If numIntersections is 2, the endpoints of the segment of
            // intersection are
            //   point[i]
            //   = segment0.origin + segment0Parameter[i] * segment0.direction
            //   = segment1.origin + segment1Parameter[i] * segment1.direction
            // with segment0Parameter[0] <= segment0Parameter[1] and
            // segment1Parameter[0] <= segment1Parameter[1].
            std::array<T, 2> segment0Parameter, segment1Parameter;
            std::array<Vector2<T>, 2> point;
        };

        Result operator()(Segment2<T> const& segment0, Segment2<T> const& segment1)
        {
            Result result{};
            Vector2<T> seg0Origin{}, seg0Direction{}, seg1Origin{}, seg1Direction{};
            T seg0Extent{}, seg1Extent{};
            segment0.GetCenteredForm(seg0Origin, seg0Direction, seg0Extent);
            segment1.GetCenteredForm(seg1Origin, seg1Direction, seg1Extent);

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            Line2<T> line0(seg0Origin, seg0Direction);
            Line2<T> line1(seg1Origin, seg1Direction);
            auto llResult = llQuery(line0, line1);
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the segments.
                if (std::fabs(llResult.line0Parameter[0]) <= seg0Extent &&
                    std::fabs(llResult.line1Parameter[0]) <= seg1Extent)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.segment0Parameter[0] = llResult.line0Parameter[0];
                    result.segment1Parameter[0] = llResult.line1Parameter[0];
                    result.point[0] = llResult.point;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int>::max())
            {
                // Compute the location of segment1 endpoints relative to
                // segment0.
                Vector2<T> diff = seg1Origin - seg0Origin;
                T t = Dot(seg0Direction, diff);

                // Get the parameter intervals of the segments relative to
                // segment0.
                std::array<T, 2> interval0 = { -seg0Extent, seg0Extent };
                std::array<T, 2> interval1 = { t - seg1Extent, t + seg1Extent };

                // Compute the intersection of the intervals.
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery;
                auto iiResult = iiQuery(interval0, interval1);
                if (iiResult.intersect)
                {
                    result.intersect = true;
                    result.numIntersections = iiResult.numIntersections;
                    for (int i = 0; i < iiResult.numIntersections; ++i)
                    {
                        result.segment0Parameter[i] = iiResult.overlap[i];
                        result.segment1Parameter[i] = iiResult.overlap[i] - t;
                        result.point[i] = seg0Origin + result.segment0Parameter[i] * seg0Direction;
                    }
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else
            {
                result.intersect = false;
                result.numIntersections = 0;
            }

            return result;
        }
    };
}
