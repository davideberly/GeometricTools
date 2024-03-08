// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.5.2023.08.08

#pragma once

// The query uses a line-segment intersection test with the segments of the
// mesh. The resulting set of intersection points is trimmed by discarding
// those for which the line parameters are outside [0,1].

#include <Mathematics/IntrLine2SegmentMesh2.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

namespace gte
{
    template <typename T>
    class FIQuery<T, Segment2<T>, SegmentMesh2<T>>
    {
    public:
        struct Intersection
        {
            Intersection()
                :
                indexPair{ 0, 0 },
                segmentParameter(static_cast<T>(0)),
                meshSegmentParameter(static_cast<T>(0)),
                point(Vector2<T>::Zero())
            {
            }

            Intersection(
                std::array<size_t, 2> const& inIndexPair,
                T const& inSegmentParameter,
                T const& inMeshSegmentParameter,
                Vector2<T> const& inPoint)
                :
                indexPair(inIndexPair),
                segmentParameter(inSegmentParameter),
                meshSegmentParameter(inMeshSegmentParameter),
                point(inPoint)
            {
            }

            std::array<size_t, 2> indexPair;
            T segmentParameter;
            T meshSegmentParameter;
            Vector2<T> point;
        };

        struct Result
        {
            Result()
                :
                intersections{}
            {
            }

            std::vector<Intersection> intersections;
        };

        Result operator()(Segment2<T> const& segment, SegmentMesh2<T> const& mesh)
        {
            Result result{};

            using LSQuery = FIQuery<T, Line2<T>, SegmentMesh2<T>>;
            using LSResult = typename LSQuery::Result;
            using LSIntersection = typename LSQuery::Intersection;

            LSQuery lsQuery{};
            Line2<T> line(segment.p[0], segment.p[1] - segment.p[0]);
            LSResult lsResult = lsQuery(line, mesh);
            auto last = std::remove_if(
                lsResult.intersections.begin(),
                lsResult.intersections.end(),
                [](LSIntersection const& object)
                {
                    return object.lineParameter < static_cast<T>(0) ||
                           object.lineParameter > static_cast<T>(1);
                });

            for (auto iter = lsResult.intersections.begin(); iter != last; ++iter)
            {
                result.intersections.emplace_back(
                    iter->indexPair,
                    iter->lineParameter,
                    iter->meshSegmentParameter,
                    iter->point);
            }

            return result;
        }
    };
}
