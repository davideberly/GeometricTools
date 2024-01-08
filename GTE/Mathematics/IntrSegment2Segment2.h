// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/Segment.h>
#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2Line2.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

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
            int32_t numIntersections;
        };

        // This version of the query uses Segment3<T>::GetCenteredForm, which
        // has a Normalize call. This generates rounding errors, so the query
        // should be used only with float or double.
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
            else if (llResult.numIntersections == std::numeric_limits<int32_t>::max())
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

        // This version of the query supports rational arithmetic.
        Result Exact(Segment2<T> const& segment0, Segment2<T> const& segment1)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            Vector2<T> seg0Direction = segment0.p[1] - segment0.p[0];
            Vector2<T> seg1Direction = segment1.p[1] - segment1.p[0];
            Line2<T> line0(segment0.p[0], seg0Direction);
            Line2<T> line1(segment1.p[0], seg1Direction);
            auto llResult = llQuery(line0, line1);
            if (llResult.numIntersections == 1)
            {
                // The lines are not parallel, so they intersect in a single
                // point. Test whether the line-line intersection is on the
                // segments.
                if (zero <= llResult.line0Parameter[0] && llResult.line0Parameter[1] <= one &&
                    zero <= llResult.line1Parameter[0] && llResult.line1Parameter[1] <= one)
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
            else if (llResult.numIntersections == std::numeric_limits<int32_t>::max())
            {
                // The lines are the same. Compute the location of segment1
                // endpoints relative to segment0.
                T dotD0D0 = Dot(seg0Direction, seg0Direction);
                Vector2<T> diff = segment1.p[0] - segment0.p[0];
                T t0 = Dot(seg0Direction, diff) / dotD0D0;
                diff = segment1.p[1] - segment0.p[0];
                T t1 = Dot(seg0Direction, diff) / dotD0D0;

                // Get the parameter intervals of the segments relative to
                // segment0.
                std::array<T, 2> interval0 = { zero, one };
                std::array<T, 2> interval1{};
                if (t1 >= t0)
                {
                    interval1[0] = t0;
                    interval1[1] = t1;
                }
                else
                {
                    interval1[0] = t1;
                    interval1[1] = t0;
                }

                // Compute the intersection of the intervals.
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                auto iiResult = iiQuery(interval0, interval1);
                result.intersect = iiResult.intersect;
                result.numIntersections = iiResult.numIntersections;
            }
            else
            {
                // The lines are parallel but not the same, so the segments
                // cannot intersect.
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
                segment0Parameter{ static_cast<T>(0), static_cast<T>(0) },
                segment1Parameter{ static_cast<T>(0), static_cast<T>(0) },
                point{ Vector2<T>::Zero(), Vector2<T>::Zero() }
            {
            }

            // The number is 0 (no intersection), 1 (segments intersect in a
            // a single point), or 2 (segments are collinear and intersect
            // in a segment).
            bool intersect;
            int32_t numIntersections;

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

        // This version of the query uses Segment3<T>::GetCenteredForm, which
        // has a Normalize call. This generates rounding errors, so the query
        // should be used only with float or double. NOTE: The parameters are
        // are relative to the centered form of the segment. Each segment has
        // a center C, a unit-length direction D and an extent e > 0. A
        // segment point is C+t*D where |t| <= e.
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
                    result.segment0Parameter[1] = result.segment0Parameter[0];
                    result.segment1Parameter[0] = llResult.line1Parameter[0];
                    result.segment1Parameter[1] = result.segment1Parameter[0];
                    result.point[0] = llResult.point;
                    result.point[1] = result.point[0];
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int32_t>::max())
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
                    for (int32_t i = 0; i < iiResult.numIntersections; ++i)
                    {
                        result.segment0Parameter[i] = iiResult.overlap[i];
                        result.segment1Parameter[i] = iiResult.overlap[i] - t;
                        result.point[i] = seg0Origin + result.segment0Parameter[i] * seg0Direction;
                    }
                    if (iiResult.numIntersections == 1)
                    {
                        result.segment0Parameter[1] = result.segment0Parameter[0];
                        result.segment1Parameter[1] = result.segment1Parameter[0];
                        result.point[1] = result.point[0];
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

        // This version of the query supports rational arithmetic. NOTE: The
        // parameters are relative to the endpoint form of the segment. Each
        // segment has endpoints P0 and P1. A segment point is P0+t*(P1-P0)
        // where 0 <= t <= 1.
        Result Exact(Segment2<T> const& segment0, Segment2<T> const& segment1)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            Vector2<T> seg0Direction = segment0.p[1] - segment0.p[0];
            Vector2<T> seg1Direction = segment1.p[1] - segment1.p[0];
            Line2<T> line0(segment0.p[0], seg0Direction);
            Line2<T> line1(segment1.p[0], seg1Direction);
            auto llResult = llQuery(line0, line1);
            if (llResult.numIntersections == 1)
            {
                // The lines are not parallel, so they intersect in a single
                // point. Test whether the line-line intersection is on the
                // segments.
                if (zero <= llResult.line0Parameter[0] && llResult.line0Parameter[1] <= one &&
                    zero <= llResult.line1Parameter[0] && llResult.line1Parameter[1] <= one)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.segment0Parameter[0] = llResult.line0Parameter[0];
                    result.segment0Parameter[1] = result.segment0Parameter[0];
                    result.segment1Parameter[0] = llResult.line1Parameter[0];
                    result.segment1Parameter[1] = result.segment1Parameter[0];
                    result.point[0] = llResult.point;
                    result.point[1] = result.point[0];
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int32_t>::max())
            {
                // The lines are the same. Compute the location of segment1
                // endpoints relative to segment0.
                T dotD0D0 = Dot(seg0Direction, seg0Direction);
                Vector2<T> diff = segment1.p[0] - segment0.p[0];
                T t0 = Dot(seg0Direction, diff) / dotD0D0;
                diff = segment1.p[1] - segment0.p[0];
                T t1 = Dot(seg0Direction, diff) / dotD0D0;

                // Get the parameter intervals of the segments relative to
                // segment0.
                std::array<T, 2> interval0 = { zero, one };
                std::array<T, 2> interval1{};
                if (t1 >= t0)
                {
                    interval1[0] = t0;
                    interval1[1] = t1;
                }
                else
                {
                    interval1[0] = t1;
                    interval1[1] = t0;
                }

                // Compute the intersection of the intervals.
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                auto iiResult = iiQuery(interval0, interval1);
                if (iiResult.intersect)
                {
                    result.intersect = true;
                    result.numIntersections = iiResult.numIntersections;

                    // Compute the results for segment0.
                    for (int32_t i = 0; i < iiResult.numIntersections; ++i)
                    {
                        result.segment0Parameter[i] = iiResult.overlap[i];
                        result.point[i] = segment0.p[0] + result.segment0Parameter[i] * seg0Direction;
                    }

                    // Compute the results for segment1. The interval1 was
                    // computed relative to segment0, so we have to reverse
                    // the process to obtain the parameters.
                    T dotD1D1 = Dot(seg1Direction, seg1Direction);
                    for (int32_t i = 0; i < iiResult.numIntersections; ++i)
                    {
                        diff = result.point[i] - segment1.p[0];
                        result.segment1Parameter[i] = Dot(seg1Direction, diff) / dotD1D1;
                    }

                    if (iiResult.numIntersections == 1)
                    {
                        result.segment0Parameter[1] = result.segment0Parameter[0];
                        result.segment1Parameter[1] = result.segment1Parameter[0];
                        result.point[1] = result.point[0];
                    }
                    else
                    {
                        if (t1 < t0)
                        {
                            std::swap(
                                result.segment1Parameter[0],
                                result.segment1Parameter[1]);
                        }
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
