// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/IntrLine2Line2.h>
#include <Mathematics/Segment.h>
#include <array>
#include <cstdint>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line2<T>, Segment2<T>>
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

            // If the line and segment do not intersect,
            //   intersect = false
            //   numIntersections = 0
            //
            // If the line and segment intersect in a single point,
            //   intersect = true
            //   numIntersections = 1
            //
            // If the line and segment are collinear,
            //   intersect = true
            //   numIntersections = std::numeric_limits<int32_t>::max()
            bool intersect;
            int32_t numIntersections;
        };

        Result operator()(Line2<T> const& line, Segment2<T> const& segment)
        {
            Result result{};

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            Line2<T> segLine(segment.p[0], segment.p[1] - segment.p[0]);
            auto llResult = llQuery(line, segLine);
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the segment.
                if (llResult.line1Parameter[0] >= static_cast<T>(0) &&
                    llResult.line1Parameter[1] <= static_cast<T>(1))
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
            else
            {
                result.intersect = llResult.intersect;
                result.numIntersections = llResult.numIntersections;
            }
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line2<T>, Segment2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                lineParameter{ static_cast<T>(0), static_cast<T>(0) },
                segmentParameter{ static_cast<T>(0), static_cast<T>(0) },
                point(Vector2<T>::Zero())
            {
            }

            // If the line and segment do not intersect,
            //   intersect = false
            //   numIntersections = 0
            //   lineParameter[] = { 0, 0 }  // invalid
            //   segmentParameter[] = { 0, 0 }  // invalid
            //   point = { 0, 0 }  // invalid
            //
            // If the line and segment intersect in a single point, the
            // parameter for line is s0 and the parameter for segment is
            // s1 in [0,1],
            //   intersect = true
            //   numIntersections = 1
            //   lineParameter = { s0, s0 }
            //   segmentParameter = { s1, s1 }
            //   point = line.origin + s0 * line.direction
            //         = segment.p[0] + s1 * (segment.p[1] - segment.p[0]);
            //
            // If the line and segment are collinear, let
            // maxT = std::numeric_limits<T>::max(),
            //   intersect = true
            //   numIntersections = std::numeric_limits<int32_t>::max()
            //   lineParameter[] = { -maxT, +maxT }
            //   segmentParameter[] = { 0, 1 }
            //   point = { 0, 0 }  // invalid

            bool intersect;
            int32_t numIntersections;
            std::array<T, 2> lineParameter;
            std::array<T, 2> segmentParameter;
            Vector2<T> point;
        };

        Result operator()(Line2<T> const& line, Segment2<T> const& segment)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            Line2<T> segLine(segment.p[0], segment.p[1] - segment.p[0]);
            auto llResult = llQuery(line, segLine);
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the segment.
                if (llResult.line1Parameter[0] >= zero &&
                    llResult.line1Parameter[1] <= one)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.lineParameter[0] = llResult.line0Parameter[0];
                    result.lineParameter[1] = result.lineParameter[0];
                    result.segmentParameter[0] = llResult.line1Parameter[0];
                    result.segmentParameter[1] = result.segmentParameter[0];
                    result.point = llResult.point;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int32_t>::max())
            {
                result.intersect = true;
                result.numIntersections = std::numeric_limits<int32_t>::max();
                T maxT = std::numeric_limits<T>::max();
                result.lineParameter[0] = -maxT;
                result.lineParameter[1] = +maxT;
                result.segmentParameter[0] = zero;
                result.segmentParameter[1] = one;
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
