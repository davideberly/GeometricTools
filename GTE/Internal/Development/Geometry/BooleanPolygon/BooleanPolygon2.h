// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#pragma once

#include <Mathematics/AlignedBox.h>
#include <Mathematics/IntrSegment2Segment2.h>
#include <Mathematics/UniqueVerticesSimplices.h>
#include <map>
#include <memory>

// An implementation of Boolean operations polygon trees. Each polygon tree
// must partition the plane so that it is 2-colored.

namespace gte
{
    struct IndexTree
    {
        // TODO: Add a member "bool isOuter"?
        std::vector<size_t> loop;
        std::vector<std::shared_ptr<IndexTree>> nested;
    };

    template <typename Real>
    class Polygon
    {
    public:
        using Vertex = Vector2<Real>;
        using Edge = std::array<size_t, 2>;

        Polygon(std::vector<Vertex> const& vertices, std::vector<Edge> const& edges)
            :
            mVertices{},
            mEdges{}
        {
            LogAssert(
                vertices.size() >= 3 && edges.size() >= 3,
                "The polygon must be at least a triangle.");
            for (auto const& edge : edges)
            {
                LogAssert(
                    edge[0] != edge[1],
                    "Degenerate edges are not allowed.");
            }

            UniqueVerticesSimplices<Vertex, size_t, 2> uvs;
            uvs.RemoveDuplicateAndUnusedVertices(vertices, edges, mVertices, mEdges);

            bool validGeometry = VerifyGeometry();
            LogAssert(
                validGeometry,
                "The geometry of the input vertices and edges is invalid.");

            bool validTopology = VerifyTopology();
            LogAssert(
                validTopology,
                "The topology of the input vertices and edges is invalid.");

            // Create a bounding box to use for early-out no-intersection
            // tests between polygons.
            auto extremes = std::minmax_element(mVertices.begin(), mVertices.end());
            mAABB.min = *extremes.first;
            mAABB.max = *extremes.second;
        }

    private:
        bool VerifyGeometry()
        {
            return true;
        }

        bool VerifyTopology()
        {
            // For each vertex, construct the set of adjacent vertices.
            std::vector<std::set<size_t>> incomingAdjacent(mVertices.size());
            std::vector<std::set<size_t>> outgoingAdjacent(mVertices.size());
            for (auto const& edge : mEdges)
            {
                incomingAdjacent[edge[1]].insert(edge[0]);
                outgoingAdjacent[edge[0]].insert(edge[1]);
            }

            // At each vertex, the number of incoming edges must be even and
            // the number of outgoing edges must be even.
            for (size_t v = 0; v < mVertices.size(); ++v)
            {
                if ((incomingAdjacent[v].size() & 1) ||
                    (outgoingAdjacent[v].size() & 1))
                {
                    return false;
                }
            }

            return true;
        }

        std::vector<Vector2<Real>> mVertices;
        std::vector<std::array<size_t, 2>> mEdges;
        std::shared_ptr<IndexTree> mTree;
        AlignedBox2<Real> mAABB;
    };


    template <typename Real>
    class BooleanPolygon2
    {
    public:
        using Polygon = std::vector<Vector2<Real>>;
        using PolygonArray = std::vector<Polygon>;

        BooleanPolygon2() = default;

        void Intersection(Polygon const& input0, Polygon const& input1,
            PolygonArray& output)
        {
            output.clear();
            std::array<Polygon const*, 2> input = { &input0, &input1 };
            std::vector<std::array<Vector2<Real>, 2>> edges;
            GetEdges(input, edges);
        }

        void Union(Polygon const& input0, Polygon const& input1,
            PolygonArray& output)
        {
            Polygon negInput0 = input0, negInput1 = input1;
            std::reverse(negInput0.begin(), negInput0.end());
            std::reverse(negInput1.begin(), negInput1.end());
            Intersection(negInput0, negInput1, output);
            for (auto& polygon : output)
            {
                std::reverse(polygon.begin(), polygon.end());
            }
        }

        void Difference(Polygon const& input0, Polygon const& input1,
            PolygonArray& output)
        {
            Polygon negInput1 = input1;
            std::reverse(negInput1.begin(), negInput1.end());
            Intersection(input0, negInput1, output);
        }

        void ExclusiveOr(Polygon const& input0, Polygon const& input1,
            PolygonArray& output)
        {
            PolygonArray outputDiff01, outputDiff10, outputUnion;
            Difference(input0, input1, outputDiff01);
            Difference(input1, input0, outputDiff10);
            Union(outputDiff01, outputDiff10, output);
        }

        void Intersection(PolygonArray const& polygons0, PolygonArray const& polygons1,
            PolygonArray& output)
        {
            PolygonArray outputIntersection;
            output.clear();
            for (auto const& input0 : polygons0)
            {
                for (auto const& input1 : polygons1)
                {
                    Intersection(input0, input1, outputIntersection);
                    output.push_back(output.end(),
                        outputIntersection.begin(), outputIntersection.end());
                }
            }
        }

        void Union(PolygonArray const& polygons0, PolygonArray const& polygons1,
            PolygonArray& output)
        {
            PolygonArray outputUnion;
            output.clear();
            for (auto const& input0 : polygons0)
            {
                for (auto const& input1 : polygons1)
                {
                    Union(input0, input1, outputUnion);
                    output.push_back(output.end(),
                        outputUnion.begin(), outputUnion.end());
                }
            }
        }

        void Difference(PolygonArray const& polygons0, PolygonArray const& polygons1,
            PolygonArray& output)
        {
            PolygonArray outputDifference;
            output.clear();
            for (auto const& input0 : polygons0)
            {
                for (auto const& input1 : polygons1)
                {
                    Difference(input0, input1, outputDifference);
                    output.push_back(output.end(),
                        outputDifference.begin(), outputDifference.end());
                }
            }
        }

        void ExclusiveOr(PolygonArray const& polygons0, PolygonArray const& polygons1,
            PolygonArray& output)
        {
            PolygonArray outputExclusiveOr;
            output.clear();
            for (auto const& input0 : polygons0)
            {
                for (auto const& input1 : polygons1)
                {
                    ExclusiveOr(input0, input1, outputExclusiveOr);
                    output.push_back(output.end(),
                        outputExclusiveOr.begin(), outputExclusiveOr.end());
                }
            }
        }

    private:
        struct Rectangle
        {
            AlignedBox2<Real> aabb;
            size_t whichPolygon = 0;
        };

        struct Endpoint
        {
            Real value;     // endpoint value
            size_t type;    // 0 if interval min, 1 if interval max
            size_t index;   // index of rectangle;

            bool operator<(Endpoint const& endpoint) const
            {
                if (value < endpoint.value)
                {
                    return true;
                }
                if (value > endpoint.value)
                {
                    return false;
                }
                return type < endpoint.type;
            }
        };

        void GetEdges(std::array<Polygon const*, 2> const& input,
            std::vector<std::array<Vector2<Real>, 2>>&)
        {
            // Get the bounding rectangles.
            size_t const numRectangles = input[0]->size() + input[1]->size();
            std::vector<Rectangle> rectangles(numRectangles);
            size_t currentRectangle = 0;
            for (size_t p = 0; p < 2; ++p)
            {
                for (size_t i0 = input[p]->size() - 1, i1 = 0; i1 < input[p]->size(); i0 = i1++)
                {
                    auto& rectangle = rectangles[currentRectangle++];
                    auto v0 = (*input[p])[i0];
                    auto v1 = (*input[p])[i1];
                    for (int j = 0; j < 2; ++j)
                    {
                        if (v0[j] <= v1[j])
                        {
                            rectangle.aabb.min[j] = v0[j];
                            rectangle.aabb.max[j] = v1[j];
                        }
                        else
                        {
                            rectangle.aabb.min[j] = v1[j];
                            rectangle.aabb.max[j] = v0[j];
                        }
                    }
                    rectangle.whichPolygon = p;
                }
            }

            // Get the rectangle endpoints to support the sort-and-sweep
            // intersection algorithm.
            size_t const numEndpoints = 2 * numRectangles;
            std::array<std::vector<Endpoint>, 2> endpoints =
            {
                std::vector<Endpoint>(numEndpoints),
                std::vector<Endpoint>(numEndpoints)
            };

            size_t currentEndpoint = 0;
            currentRectangle = 0;
            for (size_t p = 0; p < 2; ++p)
            {
                for (size_t i0 = input[p]->size() - 1, i1 = 0; i1 < input[p]->size(); i0 = i1++)
                {
                    auto& rectangle = rectangles[currentRectangle];

                    for (int j = 0; j < 2; ++j)
                    {
                        auto& emin = endpoints[j][currentEndpoint];
                        emin.type = 0;
                        emin.index = currentRectangle;
                        emin.value = rectangle.aabb.min[j];
                    }
                    ++currentEndpoint;

                    for (int j = 0; j < 2; ++j)
                    {
                        auto& emax = endpoints[j][currentEndpoint];
                        emax.type = 1;
                        emax.index = currentRectangle;
                        emax.value = rectangle.aabb.max[j];
                    }
                    ++currentEndpoint;

                    ++currentRectangle;
                }
            }

            // Sort the endpoints in each dimension.
            for (size_t j = 0; j < 2; ++j)
            {
                std::sort(endpoints[j].begin(), endpoints[j].end());
            }

            // The active set of rectangles for the sweep phase, stored by
            // index into rectangles[].
            std::set<size_t> active;

            // The set of overlapping rectangles, stored by pairs of indices
            // into rectangles[].
            std::set<std::array<size_t, 2>> overlap;

            // Sweep through the endpoints to find overlapping x-intervals.
            for (auto const& xEnd : endpoints[0])
            {
                if (xEnd.type == 0)  // an interval 'begin' value
                {
                    for (auto activeIndex : active)
                    {
                        // Rectangles r0 and r1 x-overlap.
                        auto const& r0 = rectangles[activeIndex];
                        auto const& r1 = rectangles[xEnd.index];

                        // Test for y-overlap between two rectangles only
                        // when they are from different polygons.
                        if (r0.whichPolygon != r1.whichPolygon &&
                            r0.aabb.max[1] >= r1.aabb.min[1] &&
                            r0.aabb.min[1] <= r1.aabb.max[1])
                        {
                            if (activeIndex < xEnd.index)
                            {
                                overlap.insert({ activeIndex, xEnd.index });
                            }
                            else
                            {
                                overlap.insert({ xEnd.index, activeIndex });
                            }
                        }
                    }
                    active.insert(xEnd.index);
                }
                else  // an interval 'end' value
                {
                    active.erase(xEnd.index);
                }
            }

            FIQuery<Real, Segment2<Real>, Segment2<Real>> query;
            typename FIQuery<Real, Segment2<Real>, Segment2<Real>>::Result result;
            for (auto const& element : overlap)
            {
                LogAssert(rectangles[element[0]].whichPolygon == 0
                    && rectangles[element[1]].whichPolygon == 1,
                    "Unexpected condition.");

                size_t i01 = element[0];
                size_t i00 = (i01 > 0 ? i01 - 1 : input[0]->size() - 1);
                size_t i11 = element[1] - input[0]->size();
                size_t i10 = (i11 > 0 ? i11 - 1 : input[1]->size() - 1);

                Segment2<Real> edge0((*input[0])[i00], (*input[0])[i01]);
                Segment2<Real> edge1((*input[1])[i10], (*input[1])[i11]);
                result = query(edge0, edge1);
            }

            int stophere;
            stophere = 0;
        }
    };
}
