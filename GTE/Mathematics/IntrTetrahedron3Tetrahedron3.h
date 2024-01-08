// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2023.08.08

#pragma once

// The queries consider the tetrahedron to be a solid.
//
// The test-intersection query uses the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The set of potential separating directions includes the 4 face normals of
// tetra0, the 4 face normals of tetra1, and 36 directions, each of which is
// the cross product of an edge of tetra0 and and an edge of tetra1.
//
// The separating axes involving cross products of edges has numerical
// robustness problems when the two edges are nearly parallel. The cross
// product of the edges is nearly the zero vector, so normalization of the
// cross product may produce unit-length directions that are not close to the
// true direction. Such a pair of edges occurs when an object0 face normal N0
// and an object1 face normal N1 are nearly parallel. In this case, you may
// skip the edge-edge directions, which is equivalent to projecting the
// objects onto the plane with normal N0 and applying a 2D separating axis
// test. The ability to do so involves choosing a small nonnegative epsilon.
// It is used to determine whether two face normals, one from each object, are
// nearly parallel: |Dot(N0,N1)| >= 1 - epsilon, where 0 <= epsilon <= 1.
// The epsilon input to the operator()(...) function is clamped to [0,1].
//
// The pair of integers 'separating', say, (i0,i1), identifies the axes that
// reported separation; there may be more than one but only one is reported.
// If the separating axis is a face normal N[i0] of object0, then (i0,smax) is
// returned, where smax = std::numeric_limits<size_t>::max(). If the axis is a
// face normal N[i1], then (smax,i1) is returned. If the axis is a cross
// product of edges, Cross(N[i0],N[i1]), then (i0,i1) is returned. If
// 'intersect' is true, the separating[] values are invalid because there is
// no separation.

#include <Mathematics/TIQuery.h>
#include <Mathematics/Tetrahedron3.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Tetrahedron3<T>, Tetrahedron3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                separating{ std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max() }
            {
            }

            bool intersect;
            std::array<size_t, 2> separating;
        };

        Result operator()(Tetrahedron3<T> const& tetra0, Tetrahedron3<T> const& tetra1,
            T epsilon = static_cast<T>(0))
        {
            Result result{};

            // Test face normals of tetra0 for separation. Because of the
            // counterclockwise ordering of the face vertices relative to an
            // observer outside the tetrahedron, the projection interval for
            // the face is [T,0], where T < 0. Determine whether tetra1 is on
            // the positive side of the face-normal line.
            for (size_t i = 0; i < 4; ++i)
            {
                auto const& faceIndices = Tetrahedron3<T>::GetFaceIndices(i);
                Vector3<T> P = tetra0.v[faceIndices[0]];
                Vector3<T> N = tetra0.ComputeFaceNormal(i);
                if (WhichSide(tetra1, P, N) > 0)
                {
                    // Tetra1 is entirely on the positive side of the
                    // normal line P + t * N.
                    result.intersect = false;
                    result.separating[0] = i;
                    result.separating[1] = std::numeric_limits<size_t>::max();
                    return result;
                }
            }

            // Test face normals of tetra1 for separation. Because of the
            // counterclockwise ordering of the face vertices relative to an
            // observer outside the tetrahedron, the projection interval for
            // the face is [T,0], where T < 0. Determine whether tetra1 is on
            // the positive side of the face-normal line.
            for (size_t i = 0; i < 4; ++i)
            {
                auto const& faceIndices = Tetrahedron3<T>::GetFaceIndices(i);
                Vector3<T> P = tetra1.v[faceIndices[0]];
                Vector3<T> N = tetra1.ComputeFaceNormal(i);
                if (WhichSide(tetra0, P, N) > 0)
                {
                    // Tetra1 is entirely on the positive side of the
                    // normal line P + t * N.
                    result.intersect = false;
                    result.separating[0] = std::numeric_limits<size_t>::max();
                    result.separating[1] = i;
                    return result;
                }
            }

            // Test cross products of pairs of edge directions, one edge from
            // each tetrahedron.
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const cutoff = std::min(std::max(one - epsilon, zero), one);
            for (size_t i0 = 0; i0 < 6; ++i0)
            {
                auto const& edge0Indices = Tetrahedron3<T>::GetEdgeIndices(i0);
                Vector3<T> P0 = tetra0.v[edge0Indices[0]];
                Vector3<T> E0 = tetra0.v[edge0Indices[1]] - P0;
                for (size_t i1 = 0; i1 < 6; ++i1)
                {
                    auto const& edge1Indices = Tetrahedron3<T>::GetEdgeIndices(i1);
                    Vector3<T> P1 = tetra0.v[edge1Indices[0]];
                    Vector3<T> E1 = tetra1.v[edge1Indices[1]] - P1;

                    if (std::fabs(Dot(E0, E1)) < cutoff)
                    {
                        Vector3<T> N = Cross(E0, E1);
                        int32_t side0 = WhichSide(tetra0, P0, N);
                        if (side0 == 0)
                        {
                            continue;
                        }

                        int32_t side1 = WhichSide(tetra1, P0, N);
                        if (side1 == 0)
                        {
                            continue;
                        }

                        if (side0 * side1 < 0)
                        {
                            // The projections of tetra0 and tetra1 onto the
                            // line P + t * N are on opposite sides of the
                            // projection of P.
                            result.intersect = false;
                            result.separating[0] = i0;
                            result.separating[1] = i1;
                            return result;
                        }
                    }
                }
            }

            result.intersect = true;
            result.separating[0] = std::numeric_limits<size_t>::max();
            result.separating[1] = std::numeric_limits<size_t>::max();
            return result;
        }

    private:
        static int32_t WhichSide(Tetrahedron3<T> const& tetra,
            Vector3<T> const& P, Vector3<T> const& N)
        {
            // The vertices of tetra are projected to the form P + t * N.
            // The return values is +1 if all t > 0, -1 if all t < 0, but
            // 0 otherwise, in which case tetra has points on both sides
            // of the plane Dot(N,X-P) = 0.
            T const zero = static_cast<T>(0);
            size_t positive = 0, negative = 0;
            for (size_t i = 0; i < 4; ++i)
            {
                // Project a vertex onto the normal line.
                T t = Dot(N, tetra.v[i] - P);
                if (t > zero)
                {
                    ++positive;
                }
                else if (t < zero)
                {
                    ++negative;
                }

                if (positive > 0 && negative > 0)
                {
                    // Tetra has vertices on both sides of the line, so the
                    // line is not a separating axis.
                    return 0;
                }
            }

            // Either positive > 0 or negative > 0 but not both are positive.
            return (positive > 0 ? +1 : -1);
        }
    };
}
