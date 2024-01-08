// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.5.2023.08.08

#pragma once

// The query performs an exhaustive search of the segments and finds
// line-segment intersections.
// 
// TODO: If the number of mesh segments is large, good performance might
// require multiple threads, each thread processing a subset of segments.
// Add multithreading.
// 
// TODO: Add the ability to use a preprocessed bounding region tree to
// reduce the O(n) line-segment intersection tests to O(log n). This is
// similar to the picking system in the scene graph management code for
// ray-triangle intersection tests.

#include <Mathematics/FIQuery.h>
#include <Mathematics/IntrLine2Segment2.h>
#include <Mathematics/SegmentMesh.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

namespace gte
{
    template <typename T>
    class FIQuery<T, Line2<T>, SegmentMesh2<T>>
    {
    public:
        struct Intersection
        {
            Intersection()
                :
                indexPair{ 0, 0 },
                lineParameter(static_cast<T>(0)),
                meshSegmentParameter(static_cast<T>(0)),
                point(Vector2<T>::Zero())
            {
            }

            Intersection(
                std::array<size_t, 2> const& inIndexPair,
                T const& inLineParameter,
                T const& inMeshSegmentParameter,
                Vector2<T> const& inPoint)
                :
                indexPair(inIndexPair),
                lineParameter(inLineParameter),
                meshSegmentParameter(inMeshSegmentParameter),
                point(inPoint)
            {
            }

            std::array<size_t, 2> indexPair;
            T lineParameter;
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

        Result operator()(Line2<T> const& line, SegmentMesh2<T> const& mesh)
        {
            Result result{};

            FIQuery<T, Line2<T>, Segment2<T>> lsQuery{};
            Segment2<T> segment{};

            auto const& vertices = mesh.GetVertices();
            auto const& indices = mesh.GetIndices();
            for (size_t i = 0; i < indices.size(); ++i)
            {
                segment.p[0] = vertices[indices[i][0]];
                segment.p[1] = vertices[indices[i][1]];
                auto lsResult = lsQuery(line, segment);
                if (lsResult.intersect)
                {
                    if (lsResult.numIntersections == 1)
                    {
                        // The line and segment intersect in a unique point.
                        result.intersections.emplace_back(
                            indices[i],
                            lsResult.lineParameter[0],
                            lsResult.segmentParameter[0],
                            lsResult.point);
                    }
                    else
                    {
                        // The line and segment are coincident. Report both
                        // segment endpoints as intersections.
                        for (size_t j = 0; j < 2; ++j)
                        {
                            result.intersections.emplace_back(
                                indices[i],
                                lsResult.lineParameter[j],
                                lsResult.segmentParameter[j],
                                segment.p[j]);
                        }
                    }
                }
            }

            // Sort the intersection points by line parameter. This makes it
            // easier to implement the ray-mesh and segment-mesh queries than
            // by using the lower-level Ray2-Segment2 and Segment2-Segment2
            // intersection queries.
            std::sort(result.intersections.begin(), result.intersections.end(),
                [](Intersection const& object0, Intersection const& object1)
                {
                    return object0.lineParameter < object1.lineParameter;
                });

            return result;
        }
    };
}
