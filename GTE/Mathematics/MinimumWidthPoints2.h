// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2023.08.08

#pragma once

// The width for a set of 2D points is the minimum distance between pairs
// of parallel lines, each pair bounding the points. The width for a set
// of 2D points is equal to the width for the set of vertices of the convex
// hull of the 2D points. It can be computed using the rotating calipers
// algorithm. For details about the rotating calipers algorithm and computing
// the width of a set of 2D points, see
// http://www-cgrl.cs.mcgill.ca/~godfried/research/calipers.html
// https://web.archive.org/web/20150330010154/http://cgm.cs.mcgill.ca/~orm/rotcal.html

#include <Mathematics/ConvexHull2.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/RotatingCalipers.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <vector>

namespace gte
{
    template <typename T>
    class MinimumWidthPoints2
    {
    public:
        // The return value is an oriented box in 2D. The width of the point
        // set is in the direction box.axis[0]; the width is 2*box.extent[0].
        // The corresponding height is in the direction box.axis[1] =
        // -Perp(box.axis[0]); the height is 2*box.extent[1].

        // The points are arbitrary, so the convex hull must be computed from
        // them to obtain the convex polygon whose minimum width is the
        // desired output.
        OrientedBox2<T> operator()(int32_t numPoints, Vector2<T> const* points,
            bool useRotatingCalipers = true)
        {
            LogAssert(
                numPoints >= 3 && points != nullptr,
                "Invalid input.");

            OrientedBox2<T> box{};

            // Get the convex hull of the points.
            T const zero = static_cast<T>(0);
            ConvexHull2<T> ch2{};
            ch2(numPoints, points, zero);
            int32_t dimension = ch2.GetDimension();

            if (dimension == 0)
            {
                box.center = points[0];
                box.axis[0] = Vector2<T>::Unit(0);
                box.axis[1] = Vector2<T>::Unit(1);
                box.extent[0] = zero;
                box.extent[1] = zero;
                return box;
            }

            if (dimension == 1)
            {
                // The points lie on a line. Determine the extreme t-values
                // for the points represented as P = origin + t*direction. We
                // know that 'origin' is an input vertex, so we can start
                // both t-extremes at zero.
                Line2<T> const& line = ch2.GetLine();
                T tmin = zero, tmax = zero;
                for (int32_t i = 0; i < numPoints; ++i)
                {
                    Vector2<T> diff = points[i] - line.origin;
                    T t = Dot(diff, line.direction);
                    if (t > tmax)
                    {
                        tmax = t;
                    }
                    else if (t < tmin)
                    {
                        tmin = t;
                    }
                }

                T const half = static_cast<T>(0.5);
                box.center = line.origin + half * (tmin + tmax) * line.direction;
                box.extent[0] = zero;
                box.extent[1] = half * (tmax - tmin);
                box.axis[0] = Perp(line.direction);
                box.axis[1] = line.direction;
                return box;
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

            ComputeMinWidth(vertices, useRotatingCalipers, box);
            return box;
        }

        // The points are arbitrary, so the convex hull must be computed from
        // them to obtain the convex polygon whose minimum width is the
        // desired output.
        OrientedBox2<T> operator()(std::vector<Vector2<T>> const& points,
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
        OrientedBox2<T> operator()(int32_t numPoints, Vector2<T> const* points,
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
                for (int32_t i = 0; i < numIndices; ++i)
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
        OrientedBox2<T> operator()(std::vector<Vector2<T>> const& points,
            std::vector<int32_t> const& indices,
            bool useRotatingCalipers = true)
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
        using RotatingCalipersType = RotatingCalipers<T>;
        using AntipodeType = typename RotatingCalipersType::Antipode;
        using Rational = BSRational<UIntegerAP32>;

        void ComputeMinWidth(std::vector<Vector2<T>> const& vertices,
            bool useRotatingCalipers, OrientedBox2<T>& box)
        {
            T const zero = static_cast<T>(0);
            std::function<Vector2<T>(size_t)> GetVertex{};
            std::vector<size_t> indices{};
            size_t numElements{}, i0Min{}, i1Min{};
            T minWidth{};

            if (useRotatingCalipers)
            {
                std::vector<AntipodeType> antipodes{};
                RotatingCalipersType::ComputeAntipodes(vertices, antipodes);
                LogAssert(
                    antipodes.size() > 0,
                    "Antipodes must exist.");

                Rational minSqrWidth = ComputeSqrWidth(vertices, antipodes[0]);
                size_t minAntipode = 0;
                for (size_t i = 1; i < antipodes.size(); ++i)
                {
                    Rational sqrWidth = ComputeSqrWidth(vertices, antipodes[i]);
                    if (sqrWidth < minSqrWidth)
                    {
                        minSqrWidth = sqrWidth;
                        minAntipode = i;
                    }
                }
                minWidth = std::sqrt(minSqrWidth);

                GetVertex = [&vertices](size_t j) { return vertices[j]; };
                numElements = vertices.size();
                i0Min = antipodes[minAntipode].edge[0];
                i1Min = antipodes[minAntipode].edge[1];
            }
            else
            {
                // Remove duplicate and collinear vertices.
                size_t const numVertices = vertices.size();
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
                i0Min = numIndices - 1;
                i1Min = 0;
                for (size_t i0 = numIndices - 1, i1 = 0; i1 < indices.size(); i0 = i1++)
                {
                    Vector2<T> const& origin = vertices[indices[i0]];
                    Vector2<T> U = vertices[indices[i1]] - origin;
                    Normalize(U);

                    T maxWidth = zero;
                    for (size_t j = 0; j < numIndices; ++j)
                    {
                        Vector2<T> diff = vertices[indices[j]] - origin;
                        T width = U[0] * diff[1] - U[1] * diff[0];
                        if (width > maxWidth)
                        {
                            maxWidth = width;
                        }
                    }

                    if (maxWidth < minWidth)
                    {
                        minWidth = maxWidth;
                        i0Min = i0;
                        i1Min = i1;
                    }
                }

                GetVertex = [&vertices, &indices](size_t j)
                {
                    return vertices[indices[j]];
                };

                numElements = numIndices;
            }

            Vector2<T> origin{}, U{};
            T minHeight{}, maxHeight{};
            Compute(GetVertex, numElements, i0Min, i1Min,
                origin, U, minHeight, maxHeight);

            T const half = static_cast<T>(0.5);
            box.extent[0] = half * minWidth;
            box.extent[1] = half * (maxHeight - minHeight);
            box.axis[0] = -Perp(U);
            box.axis[1] = U;
            box.center = origin + box.extent[0] * box.axis[0] +
                (half * (maxHeight + minHeight)) * box.axis[1];
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

        void Compute(std::function<Vector2<T>(size_t)> const& GetVertex,
            size_t numElements, size_t i0Min, size_t i1Min,
            Vector2<T>& origin, Vector2<T>& U, T& minHeight, T& maxHeight)
        {
            T const zero = static_cast<T>(0);
            origin = GetVertex(i0Min);
            U = GetVertex(i1Min) - origin;
            Normalize(U);

            minHeight = zero;
            maxHeight = zero;
            for (size_t j = 0; j < numElements; ++j)
            {
                Vector2<T> diff = GetVertex(j) - origin;
                T height = Dot(U, diff);
                if (height < minHeight)
                {
                    minHeight = height;
                }
                else if (height > maxHeight)
                {
                    maxHeight = height;
                }
            }
        }
    };
}