// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2026.02.24

#pragma once

// Compute the distance between a line and a solid triangle in 2D.
// 
// The line is P + t * D, where D is not required to be unit length.
// 
// The triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The closest point on the line is stored in closest[0] with parameter t. The
// closest point on the triangle is closest[1] with barycentric coordinates
// (b[0],b[1],b[2]). When there are infinitely many choices for the pair of
// closest points, only one of them is returned. TODO: Compute the entire set
// of intersection when it is a line segment.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/Triangle.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line2<T>, Triangle2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                parameter(static_cast<T>(0)),
                barycentric{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector2<T>::Zero(), Vector2<T>::Zero() }
            {
            }

            T distance, sqrDistance;
            T parameter;
            std::array<T, 3> barycentric;
            std::array<Vector2<T>, 2> closest;
        };

        Result operator()(Line2<T> const& line, Triangle2<T> const& triangle)
        {
            Result result{};

            // Test whether the triangle is strictly on one side of the line,
            // in which case the distance is the smallest absolute line-normal
            // component of the vertices. If at least one vertex is on the
            // line, then the distance is 0. If the triangle has at least one
            // vertex strictly on one side of the line and at least one vertex
            // strictly on the other side of the line, then the distance is 0.
            T const zero = static_cast<T>(0);
            auto const& P = line.origin;
            auto const& D = line.direction;
            auto const& V = triangle.v;
            Vector2<T> N = Perp(D);
            std::array<T, 3> ncomp{};
            std::array<std::int32_t, 3> sign{};
            for (std::size_t i = 0; i < 3; ++i)
            {
                ncomp[i] = Dot(N, V[i] - P);
                if (ncomp[i] > zero)
                {
                    sign[i] = +1;
                }
                else if (ncomp[i] < zero)
                {
                    sign[i] = -1;
                }
                else // ncomp[i] = zero
                {
                    sign[i] = 0;
                }
            }

            // In the next ensuing blocks of code, s0s1s2 represents the
            // signs of the normal component (ncomp) at the vertices. The
            // term s? is in {+,0,-}.
            if (sign[0] > 0)
            {
                if (sign[1] > 0)
                {
                    if (sign[2] > 0)
                    {
                        // +++
                        // The triangle is strictly on the positive side of
                        // the line.
                        NoCommonPoints(P, D, V, ncomp, result);
                    }
                    else if (sign[2] < 0)
                    {
                        // ++-
                        // The line intersects triangle edges <V[2],V[0]> and
                        // <V[2],V[1]> at interior edge points.
                        LineIntersectsTwoEdges(P, D, V, 2, 0, 1, result);
                    }
                    else
                    {
                        // ++0
                        // The line intersects the triangle vertex V[2].
                        LineContainsVertex(P, D, V, 2, 0, 1, result);
                    }
                }
                else if (sign[1] < 0)
                {
                    if (sign[2] > 0)
                    {
                        // +-+
                        // The line intersects triangle edges <V[0],V[1]> and
                        // <V[2],V[1]> at interior edge points.
                        LineIntersectsTwoEdges(P, D, V, 0, 1, 2, result);
                    }
                    else if (sign[2] < 0)
                    {
                        // +--
                        // The line intersects triangle edges <V[0],V[1]> and
                        // <V[0],V[2]> at interior edge points.
                        LineIntersectsTwoEdges(P, D, V, 0, 1, 2, result);
                    }
                    else
                    {
                        // +-0
                        // The line intersects triangle edge <V[0],V[1]> at an
                        // interior point and at triangle vertex V[2].
                        LineContainsVertex(P, D, V, 2, 0, 1, result);
                    }
                }
                else
                {
                    if (sign[2] > 0)
                    {
                        // +0+
                        // The line intersects the triangle vertex V[1].
                        LineContainsVertex(P, D, V, 1, 2, 0, result);
                    }
                    else if (sign[2] < 0)
                    {
                        // +0-
                        // The line intersects triangle edge <V[0],V[2]> at an
                        // interior point and at triangle vertex V[1].
                        LineContainsVertex(P, D, V, 1, 2, 0, result);
                    }
                    else
                    {
                        // +00
                        // The line contains triangle edge <V[1],V[2]>.
                        LineContainsVertex(P, D, V, 1, 2, 0, result);
                    }
                }
            }
            else if (sign[0] < 0)
            {
                if (sign[1] > 0)
                {
                    if (sign[2] > 0)
                    {
                        // -++
                        // The line intersects triangle edges <V[1],V[0]> and
                        // <V[2],V[0]> at interior edge points.
                        LineIntersectsTwoEdges(P, D, V, 0, 1, 2, result);
                    }
                    else if (sign[2] < 0)
                    {
                        // -+-
                        // The line intersects triangle edges <V[1],V[0]> and
                        // <V[1],V[2]> at interior edge points.
                        LineIntersectsTwoEdges(P, D, V, 0, 1, 2, result);
                    }
                    else
                    {
                        // -+0
                        // The line intersects triangle edge <V[0],V[1]> at an
                        // interior point and at triangle vertex V[2].
                        LineContainsVertex(P, D, V, 2, 0, 1, result);
                    }
                }
                else if (sign[1] < 0)
                {
                    if (sign[2] > 0)
                    {
                        // --+
                        // The line intersects triangle edges <V[2],V[0]> and
                        // <V[2],V[1]> at interior edge points.
                        LineIntersectsTwoEdges(P, D, V, 1, 2, 0, result);
                    }
                    else if (sign[2] < 0)
                    {
                        // ---
                        // The triangle is strictly on the negative side of
                        // the line.
                        NoCommonPoints(P, D, V, ncomp, result);
                    }
                    else
                    {
                        // --0
                        // The line intersects the triangle vertex V[2].
                        LineContainsVertex(P, D, V, 2, 0, 1, result);
                    }
                }
                else
                {
                    if (sign[2] > 0)
                    {
                        // -0+
                        // The line intersects triangle edge <V[0],V[2]> at an
                        // interior point and at triangle vertex V[1].
                        LineContainsVertex(P, D, V, 1, 2, 0, result);
                    }
                    else if (sign[2] < 0)
                    {
                        // -0-
                        // The line intersects the triangle vertex V[1].
                        LineContainsVertex(P, D, V, 1, 2, 0, result);
                    }
                    else
                    {
                        // -00
                        // The line contains triangle <V[1],V[2]>.
                        LineContainsVertex(P, D, V, 1, 2, 0, result);
                    }
                }
            }
            else
            {
                if (sign[1] > 0)
                {
                    if (sign[2] > 0)
                    {
                        // 0++: The line contains only the vertex V[0].
                        // The line intersects the triangle vertex V[0].
                        LineContainsVertex(P, D, V, 0, 1, 2, result);
                    }
                    else if (sign[2] < 0)
                    {
                        // 0+-
                        // The line intersects triangle edge <V[1],V[2]> at an
                        // interior point and at triangle vertex V[0].
                        LineContainsVertex(P, D, V, 0, 1, 2, result);
                    }
                    else
                    {
                        // 0+0
                        // The line contains triangle edge <V[2],V[0]>.
                        LineContainsVertex(P, D, V, 2, 0, 1, result);
                    }
                }
                else if (sign[1] < 0)
                {
                    if (sign[2] > 0)
                    {
                        // 0-+
                        // The line intersects triangle edge <V[1],V[2]> at an
                        // interior point and at triangle vertex V[0].
                        LineContainsVertex(P, D, V, 0, 1, 2, result);
                    }
                    else if (sign[2] < 0)
                    {
                        // 0--
                        // The line intersects the triangle vertex V[0].
                        LineContainsVertex(P, D, V, 0, 1, 2, result);
                    }
                    else
                    {
                        // 0-0
                        // The line contains the triangle edge <V[2],V[0]>.
                        LineContainsVertex(P, D, V, 2, 0, 1, result);
                    }
                }
                else
                {
                    if (sign[2] > 0)
                    {
                        // 00+
                        // The line contains triangle edge <V[0],V[1]>.
                        LineContainsVertex(P, D, V, 0, 1, 2, result);
                    }
                    else if (sign[2] < 0)
                    {
                        // 00-
                        // The line contains triangle edge <V[0],V[1]>.
                        LineContainsVertex(P, D, V, 0, 1, 2, result);
                    }
                    else
                    {
                        // 000
                        // The triangle is degenerate, a single point.
                        LineContainsVertex(P, D, V, 0, 1, 2, result);
                    }
                }
            }

            Vector2<T> diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

    private:
        void LineContainsVertex(
            Vector2<T> const& P, Vector2<T> const& D, // line
            std::array<Vector2<T>, 3> const& V, // triangle
            std::size_t i0, std::size_t i1, std::size_t i2,
            Result& result)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            result.distance = zero;
            result.sqrDistance = zero;
            result.parameter = Dot(D, V[i0] - P) / Dot(D, D);
            result.barycentric[i0] = one;
            result.barycentric[i1] = zero;
            result.barycentric[i2] = zero;
            result.closest[0] = V[i0];
            result.closest[1] = V[i0];
        }

        // At V[i0] and V[i1] the signs satisfy sign[i0] * sign[i1] < 0.
        void LineIntersectsTwoEdges(
            Vector2<T> const& P, Vector2<T> const& D, // line
            std::array<Vector2<T>, 3> const& V, // triangle
            std::size_t i0, std::size_t i1, std::size_t i2,
            Result& result)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T s = DotPerp(D, P - V[i0]) / DotPerp(D, V[i1] - V[i0]);
            T oms = one - s;
            Vector2<T> Q = oms * V[i0] + s * V[i1];
            result.distance = zero;
            result.sqrDistance = zero;
            result.parameter = Dot(D, Q - P) / Dot(D, D);
            result.barycentric[i0] = oms;
            result.barycentric[i1] = s;
            result.barycentric[i2] = zero;
            result.closest[0] = Q;
            result.closest[1] = Q;
        }

        void NoCommonPoints(
            Vector2<T> const& P, Vector2<T> const& D, // line
            std::array<Vector2<T>, 3> const& V, // triangle
            std::array<T, 3> const& ncomp,
            Result& result)
        {
            T minDistance = std::fabs(ncomp[0]);
            std::size_t minIndex = 0;
            T distance = std::fabs(ncomp[1]);
            if (distance < minDistance)
            {
                minDistance = distance;
                minIndex = 1;
            }
            distance = std::fabs(ncomp[2]);
            if (distance < minDistance)
            {
                minDistance = distance;
                minIndex = 2;
            }

            result.distance = minDistance;
            result.sqrDistance = minDistance * minDistance;
            result.parameter = Dot(D, V[minIndex] - P) / Dot(D, D);
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            result.barycentric[0] = (minIndex == 0) ? one : zero;
            result.barycentric[1] = (minIndex == 1) ? one : zero;
            result.barycentric[2] = (minIndex == 2) ? one : zero;
            result.closest[0] = P + result.parameter * D;
            result.closest[1] = V[minIndex];
        }
    };
}


