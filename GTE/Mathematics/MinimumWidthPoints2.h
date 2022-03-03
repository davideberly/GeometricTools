// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.03

#pragma once

#include <Mathematics/ConvexHull2.h>
#include <Mathematics/RotatingCalipers.h>

// The width for a set of 2D points is the minimum distance between pairs
// of parallel lines, each pair bounding the points. The width for a set
// of 2D points is equal to the width for the set of vertices of the convex
// hull of the 2D points. It can be computed using the rotating calipers
// algorithm. For details about the rotating calipers algorithm and computing
// the width of a set of 2D points, see
// http://www-cgrl.cs.mcgill.ca/~godfried/research/calipers.html
// https://web.archive.org/web/20150330010154/http://cgm.cs.mcgill.ca/~orm/rotcal.html

namespace gte
{
    template <typename T>
    class MinimumWidthPoints2
    {
    public:
        // The return value is the width of the points[] and the antipodal
        // vertex-edge pair that generates the width. The vertex and edge[]
        // indices are relative to the input points[] array.
        struct Result
        {
            Result()
                :
                width(static_cast<T>(0)),
                vertex(0),
                edge{ 0, 0 }
            {
            }

            T width;
            size_t vertex;
            std::array<size_t, 2> edge;
        };

        // The points are arbitrary, so the convex hull must be computed from
        // them to obtain the convex polygon whose minimum width is the
        // desired output.
        Result operator()(int32_t numPoints, Vector2<T> const* points,
            bool useRotatingCalipers = true)
        {
            LogAssert(
                numPoints >= 3 && points != nullptr,
                "Invalid input.");

            Result result{};

            // Get the convex hull of the points.
            T const zero = static_cast<T>(0);
            ConvexHull2<T> ch2{};
            ch2(numPoints, points, zero);
            int32_t dimension = ch2.GetDimension();

            if (dimension < 2)
            {
                // The points are all the same (dimension == 0) or all on the
                // same line (dimension == 1).
                return result;
            }

            // Get the indexed convex hull with vertices converted to the
            // compute type.
            std::vector<int32_t> hull = ch2.GetHull();
            std::vector<Vector2<T>> vertices(hull.size());
            Vector2<T> const* hullPoints = ch2.GetPoints();
            for (size_t i = 0; i < hull.size(); ++i)
            {
                vertices[i] = hullPoints[hull[i]];
            }

            ComputeMinWidth(vertices, useRotatingCalipers, result);

            // Remap the indices from those of vertices[] to those of
            // points[].
            result.vertex = hull[result.vertex];
            result.edge[0] = hull[result.edge[0]];
            result.edge[1] = hull[result.edge[1]];
            return result;
        }

        // The points are arbitrary, so the convex hull must be computed from
        // them to obtain the convex polygon whose minimum width is the
        // desired output.
        Result operator()(std::vector<Vector2<T>> const& points,
            bool useRotatingCalipers = true)
        {
            return operator()(static_cast<int32_t>(points.size()),
                points.data(), useRotatingCalipers);
        }

        // The points already form a counterclockwise, nondegenerate convex
        // polygon. If the points directly are the convex polygon, set
        // numIndices to 0 and indices to nullptr. If the polygon vertices
        // are a subset of the incoming points, that subset is identified by
        // numIndices >= 3 and indices having numIndices elements.
        Result operator()(int32_t numPoints, Vector2<T> const* points,
            int32_t numIndices, int32_t const* indices,
            bool useRotatingCalipers = true)
        {
            LogAssert(
                numPoints >= 3 && points != nullptr &&
                ((indices == nullptr && numIndices == 0) ||
                (indices != nullptr && numIndices >= 3)),
                "Invalid input.");

            if (indices)
            {
                std::vector<Vector2<T>> compactPoints(numIndices);
                for (size_t i = 0; i < numIndices; ++i)
                {
                    compactPoints[i] = points[indices[i]];
                }
                return operator()(compactPoints, useRotatingCalipers);
            }
            else
            {
                return operator()(numPoints, points, useRotatingCalipers);
            }
        }

        // The points already form a counterclockwise, nondegenerate convex
        // polygon. If the points directly are the convex polygon, pass an
        // indices object with 0 elements. If the polygon vertices are a
        // subset of the incoming points, that subset is identified by
        // numIndices >= 3 and indices having numIndices elements.
        Result operator()(std::vector<Vector2<T>> const& points,
            std::vector<int32_t> const& indices, bool useRotatingCalipers = true)
        {
            if (indices.size() > 0)
            {
                return operator()(static_cast<int32_t>(points.size()),
                    points.data(), static_cast<int32_t>(indices.size()),
                    indices.data(), useRotatingCalipers);
            }
            else
            {
                return operator()(points, useRotatingCalipers);
            }
        }

    private:
        using RotatingCalipersType = typename RotatingCalipers<T>;
        using AntipodeType = typename RotatingCalipersType::Antipode;
        using Rational = BSRational<UIntegerAP32>;

        void ComputeMinWidth(std::vector<Vector2<T>> const& vertices,
            bool useRotatingCalipers, Result& result)
        {
            T const zero = static_cast<T>(0);
            T minWidth = zero;

            if (useRotatingCalipers)
            {
                std::vector<AntipodeType> antipodes{};
                RotatingCalipersType::ComputeAntipodes(vertices, antipodes);
                LogAssert(
                    antipodes.size() > 0,
                    "Antipodes must exist.");

                Rational minSqrWidth = ComputeSqrWidth(vertices, antipodes[0]);
                size_t minIndex = 0;
                for (size_t i = 1; i < antipodes.size(); ++i)
                {
                    Rational sqrWidth = ComputeSqrWidth(vertices, antipodes[i]);
                    if (sqrWidth < minSqrWidth)
                    {
                        minSqrWidth = sqrWidth;
                        minIndex = i;
                    }
                }
                minWidth = std::sqrt(minSqrWidth);

                result.width = minWidth;
                result.vertex = antipodes[minIndex].vertex;
                result.edge = antipodes[minIndex].edge;
            }
            else
            {
                // Remove duplicate and collinear vertices.
                size_t const numVertices = vertices.size();
                std::vector<size_t> indices{};
                indices.reserve(numVertices);

                Vector2<T> ePrev = vertices.front() - vertices.back();
                for (size_t i0 = 0, i1 = 1; i0 < numVertices; ++i0)
                {
                    Vector2<T> eNext = vertices[i1] - vertices[i0];

                    T dp = DotPerp(ePrev, eNext);
                    if (dp != zero)
                    {
                        indices.push_back(i0);
                    }

                    ePrev = eNext;
                    if (++i1 == numVertices)
                    {
                        i1 = 0;
                    }
                }

                // Iterate over the polygon edges to search for the edge that
                // leads to the minimum width.
                size_t const numIndices = indices.size();
                minWidth = std::numeric_limits<T>::max();
                result.vertex = indices.front();
                result.edge = { indices.back(), indices.front() };
                for (size_t i0 = numIndices - 1, i1 = 0; i1 < indices.size(); i0 = i1++)
                {
                    Vector2<T> const& origin = vertices[indices[i0]];
                    Vector2<T> U = vertices[indices[i1]] - origin;
                    Normalize(U);

                    T maxWidth = zero;
                    size_t maxIndex = 0;
                    for (size_t j = 0; j < numIndices; ++j)
                    {
                        Vector2<T> diff = vertices[indices[j]] - origin;
                        T width = U[0] * diff[1] - U[1] * diff[0];
                        if (width > maxWidth)
                        {
                            maxWidth = width;
                            maxIndex = j;
                        }
                    }

                    if (maxWidth < minWidth)
                    {
                        minWidth = maxWidth;
                        result.width = minWidth;
                        result.vertex = maxIndex;
                        result.edge = { i0, i1 };
                    }
                }
            }
        }

        Rational ComputeSqrWidth(std::vector<Vector2<T>> const& vertices,
            AntipodeType const& antipode)
        {
            Vector2<T> const& V = vertices[antipode.vertex];
            Vector2<T> const& E0 = vertices[antipode.edge[0]];
            Vector2<T> const& E1 = vertices[antipode.edge[1]];
            Vector2<Rational> rV = { V[0], V[1] };
            Vector2<Rational> rE0 = { E0[0], E0[1] };
            Vector2<Rational> rE1 = { E1[0], E1[1] };
            Vector2<Rational> rU = rE1 - rE0;
            Vector2<Rational> rDiff = rV - rE0;
            Rational rDotPerp = rU[1] * rDiff[0] - rU[0] * rDiff[1];
            Rational rSqrLenU = Dot(rU, rU);
            Rational rSqrWidth = rDotPerp * rDotPerp / rSqrLenU;
            return rSqrWidth;
        }
    };
}