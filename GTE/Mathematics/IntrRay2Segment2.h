// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2Line2.h>
#include <Mathematics/Ray.h>
#include <Mathematics/Segment.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray2<T>, Segment2<T>>
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

            // The number is 0 (no intersection), 1 (ray and segment intersect
            // in a single point), or 2 (ray and segment are collinear and
            // intersect in a segment).
            bool intersect;
            int32_t numIntersections;
        };

        Result operator()(Ray2<T> const& ray, Segment2<T> const& segment)
        {
            Result result{};

            T const zero = static_cast<T>(0);

            Vector2<T> segOrigin{}, segDirection{};
            T segExtent{};
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            Line2<T> line0(ray.origin, ray.direction);
            Line2<T> line1(segOrigin, segDirection);
            auto llResult = llQuery(line0, line1);
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the ray and
                // segment.
                if (llResult.line0Parameter[0] >= zero &&
                    std::fabs(llResult.line1Parameter[0]) <= segExtent)
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
                // Compute the location of the right-most point of the segment
                // relative to the ray direction.
                Vector2<T> diff = segOrigin - ray.origin;
                T t = Dot(ray.direction, diff) + segExtent;
                if (t > zero)
                {
                    result.intersect = true;
                    result.numIntersections = 2;
                }
                else if (t < zero)
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
                else  // t == 0
                {
                    result.intersect = true;
                    result.numIntersections = 1;
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

    template <typename T>
    class FIQuery<T, Ray2<T>, Segment2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                rayParameter{ static_cast<T>(0), static_cast<T>(0) },
                segmentParameter{ static_cast<T>(0), static_cast<T>(0) },
                point{ Vector2<T>::Zero(), Vector2<T>::Zero() }
            {
            }

            // The number is 0 (no intersection), 1 (ray and segment intersect
            // in a single point), or 2 (ray and segment are collinear and
            // intersect in a segment).
            bool intersect;
            int32_t numIntersections;

            // If numIntersections is 1, the intersection is
            //   point[0] = ray.origin + rayParameter[0] * ray.direction
            //     = segment.center + segmentParameter[0] * segment.direction
            // If numIntersections is 2, the endpoints of the segment of
            // intersection are
            //   point[i] = ray.origin + rayParameter[i] * ray.direction
            //     = segment.center + segmentParameter[i] * segment.direction
            // with rayParameter[0] <= rayParameter[1] and
            // segmentParameter[0] <= segmentParameter[1].
            std::array<T, 2> rayParameter, segmentParameter;
            std::array<Vector2<T>, 2> point;
        };

        Result operator()(Ray2<T> const& ray, Segment2<T> const& segment)
        {
            Result result{};

            T const zero = static_cast<T>(0);

            Vector2<T> segOrigin{}, segDirection{};
            T segExtent{};
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            Line2<T> line0(ray.origin, ray.direction);
            Line2<T> line1(segOrigin, segDirection);
            auto llResult = llQuery(line0, line1);
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the ray and
                // segment.
                if (llResult.line0Parameter[0] >= zero &&
                    std::fabs(llResult.line1Parameter[0]) <= segExtent)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.rayParameter[0] = llResult.line0Parameter[0];
                    result.segmentParameter[0] = llResult.line1Parameter[0];
                    result.point[0] = llResult.point;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int32_t>::max())
            {
                // Compute t for which segment.origin =
                // ray.origin + t*ray.direction.
                Vector2<T> diff = segOrigin - ray.origin;
                T t = Dot(ray.direction, diff);

                // Get the ray interval.
                std::array<T, 2> interval0 =
                {
                    zero, std::numeric_limits<T>::max()
                };

                // Compute the location of the segment endpoints relative to
                // the ray.
                std::array<T, 2> interval1 = { t - segExtent, t + segExtent };

                // Compute the intersection of [0,+infinity) and [tmin,tmax].
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery;
                auto iiResult = iiQuery(interval0, interval1);
                if (iiResult.intersect)
                {
                    result.intersect = true;
                    result.numIntersections = iiResult.numIntersections;
                    for (int32_t i = 0; i < iiResult.numIntersections; ++i)
                    {
                        result.rayParameter[i] = iiResult.overlap[i];
                        result.segmentParameter[i] = iiResult.overlap[i] - t;
                        result.point[i] = ray.origin + result.rayParameter[i] * ray.direction;
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
