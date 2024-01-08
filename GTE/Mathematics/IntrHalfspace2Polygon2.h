// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the box to be a solid and the polygon to be a
// convex solid.

#include <Mathematics/FIQuery.h>
#include <Mathematics/Halfspace.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/Vector2.h>
#include <cstdint>
#include <vector>

namespace gte
{
    template <typename T>
    class FIQuery<T, Halfspace<2, T>, std::vector<Vector2<T>>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                polygon{}
            {
            }

            bool intersect;

            // If 'intersect' is true, the halfspace and polygon intersect
            // in a convex polygon.
            std::vector<Vector2<T>> polygon;
        };

        Result operator()(Halfspace<2, T> const& halfspace,
            std::vector<Vector2<T>> const& polygon)
        {
            Result result{};

            // Determine whether the polygon vertices are outside the
            // halfspace, inside the halfspace, or on the halfspace boundary.
            int32_t const numVertices = static_cast<int32_t>(polygon.size());
            std::vector<T> distance(numVertices);
            int32_t positive = 0, negative = 0, positiveIndex = -1;
            for (int32_t i = 0; i < numVertices; ++i)
            {
                distance[i] = Dot(halfspace.normal, polygon[i]) - halfspace.constant;
                if (distance[i] > (T)0)
                {
                    ++positive;
                    if (positiveIndex == -1)
                    {
                        positiveIndex = i;
                    }
                }
                else if (distance[i] < (T)0)
                {
                    ++negative;
                }
            }

            if (positive == 0)
            {
                // The polygon is strictly outside the halfspace.
                result.intersect = false;
                return result;
            }

            if (negative == 0)
            {
                // The polygon is contained in the closed halfspace, so it is
                // fully visible and no clipping is necessary.
                result.intersect = true;
                return result;
            }

            // The line transversely intersects the polygon. Clip the polygon.
            Vector2<T> vertex;
            int32_t curr, prev;
            T t;

            if (positiveIndex > 0)
            {
                // Compute the first clip vertex on the line.
                curr = positiveIndex;
                prev = curr - 1;
                t = distance[curr] / (distance[curr] - distance[prev]);
                vertex = polygon[curr] + t * (polygon[prev] - polygon[curr]);
                result.polygon.push_back(vertex);

                // Include the vertices on the positive side of line.
                while (curr < numVertices && distance[curr] >(T)0)
                {
                    result.polygon.push_back(polygon[curr++]);
                }

                // Compute the kast clip vertex on the line.
                if (curr < numVertices)
                {
                    prev = curr - 1;
                }
                else
                {
                    curr = 0;
                    prev = numVertices - 1;
                }
                t = distance[curr] / (distance[curr] - distance[prev]);
                vertex = polygon[curr] + t * (polygon[prev] - polygon[curr]);
                result.polygon.push_back(vertex);
            }
            else  // positiveIndex is 0
            {
                // Include the vertices on the positive side of line.
                curr = 0;
                while (curr < numVertices && distance[curr] >(T)0)
                {
                    result.polygon.push_back(polygon[curr++]);
                }

                // Compute the last clip vertex on the line.
                prev = curr - 1;
                t = distance[curr] / (distance[curr] - distance[prev]);
                vertex = polygon[curr] + t * (polygon[prev] - polygon[curr]);
                result.polygon.push_back(vertex);

                // Skip the vertices on the negative side of the line.
                while (curr < numVertices && distance[curr] <= (T)0)
                {
                    curr++;
                }

                // Compute the first clip vertex on the line.
                if (curr < numVertices)
                {
                    prev = curr - 1;
                    t = distance[curr] / (distance[curr] - distance[prev]);
                    vertex = polygon[curr] + t * (polygon[prev] - polygon[curr]);
                    result.polygon.push_back(vertex);

                    // Keep the vertices on the positive side of the line.
                    while (curr < numVertices && distance[curr] >(T)0)
                    {
                        result.polygon.push_back(polygon[curr++]);
                    }
                }
                else
                {
                    curr = 0;
                    prev = numVertices - 1;
                    t = distance[curr] / (distance[curr] - distance[prev]);
                    vertex = polygon[curr] + t * (polygon[prev] - polygon[curr]);
                    result.polygon.push_back(vertex);
                }
            }

            result.intersect = true;
            return result;
        }
    };
}
