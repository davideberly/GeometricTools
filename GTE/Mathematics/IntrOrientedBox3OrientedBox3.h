// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.07.14

#pragma once

// The queries consider the box to be a solid.
//
// The test-intersection query uses the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The set of potential separating directions includes the 3 face normals of
// box0, the 3 face normals of box1, and 9 directions, each of which is the
// cross product of an edge of box0 and an edge of box1.
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
// nearly parallel: |Dot(N0,N1)| >= 1 - epsilon. If the epsilon input to the
// operator()(...) function is negative, it is clamped to zero.
//
// The pair of integers 'separating', say, (i0,i1), identifies the axes that
// reported separation; there may be more than one but only one is reported.
// If the separating axis is a face normal N[i0] of object0, then (i0,-1) is
// returned. If the axis is a face normal N[i1], then (-1,i1) is returned. If
// the axis is a cross product of edges, Cross(N[i0],N[i1]), then (i0,i1) is
// returned. If 'intersect' is true, the separating[] values are invalid
// because there is no separation.

#include <Mathematics/TIQuery.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/Vector3.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, OrientedBox3<T>, OrientedBox3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                separating{ 0, 0 }
            {
            }

            bool intersect;
            std::array<int32_t, 2> separating;
        };

        Result operator()(OrientedBox3<T> const& box0, OrientedBox3<T> const& box1,
            T epsilon = static_cast<T>(0))
        {
            Result result{};

            // Convenience variables.
            Vector3<T> const& C0 = box0.center;
            Vector3<T> const* A0 = &box0.axis[0];
            Vector3<T> const& E0 = box0.extent;
            Vector3<T> const& C1 = box1.center;
            Vector3<T> const* A1 = &box1.axis[0];
            Vector3<T> const& E1 = box1.extent;

            epsilon = std::max(epsilon, static_cast<T>(0));
            T const cutoff = static_cast<T>(1) - epsilon;
            bool existsParallelPair = false;

            // Compute difference of box centers.
            Vector3<T> D = C1 - C0;

            // dot01[i][j] = Dot(A0[i],A1[j]) = A1[j][i]
            std::array<std::array<T, 3>, 3> dot01{};

            // |dot01[i][j]|
            std::array<std::array<T, 3>, 3> absDot01{};

            // Dot(D, A0[i])
            std::array<T, 3> dotDA0{};

            // interval radii and distance between centers
            T r0{}, r1{}, r{};

            // r0 + r1
            T r01{};

            // Test for separation on the axis C0 + t*A0[0].
            for (size_t i = 0; i < 3; ++i)
            {
                dot01[0][i] = Dot(A0[0], A1[i]);
                absDot01[0][i] = std::fabs(dot01[0][i]);
                if (absDot01[0][i] > cutoff)
                {
                    existsParallelPair = true;
                }
            }
            dotDA0[0] = Dot(D, A0[0]);
            r = std::fabs(dotDA0[0]);
            r1 = E1[0] * absDot01[0][0] + E1[1] * absDot01[0][1] + E1[2] * absDot01[0][2];
            r01 = E0[0] + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 0;
                result.separating[1] = -1;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[1].
            for (size_t i = 0; i < 3; ++i)
            {
                dot01[1][i] = Dot(A0[1], A1[i]);
                absDot01[1][i] = std::fabs(dot01[1][i]);
                if (absDot01[1][i] > cutoff)
                {
                    existsParallelPair = true;
                }
            }
            dotDA0[1] = Dot(D, A0[1]);
            r = std::fabs(dotDA0[1]);
            r1 = E1[0] * absDot01[1][0] + E1[1] * absDot01[1][1] + E1[2] * absDot01[1][2];
            r01 = E0[1] + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 1;
                result.separating[1] = -1;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[2].
            for (size_t i = 0; i < 3; ++i)
            {
                dot01[2][i] = Dot(A0[2], A1[i]);
                absDot01[2][i] = std::fabs(dot01[2][i]);
                if (absDot01[2][i] > cutoff)
                {
                    existsParallelPair = true;
                }
            }
            dotDA0[2] = Dot(D, A0[2]);
            r = std::fabs(dotDA0[2]);
            r1 = E1[0] * absDot01[2][0] + E1[1] * absDot01[2][1] + E1[2] * absDot01[2][2];
            r01 = E0[2] + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 2;
                result.separating[1] = -1;
                return result;
            }

            // Test for separation on the axis C0 + t*A1[0].
            r = std::fabs(Dot(D, A1[0]));
            r0 = E0[0] * absDot01[0][0] + E0[1] * absDot01[1][0] + E0[2] * absDot01[2][0];
            r01 = r0 + E1[0];
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = -1;
                result.separating[1] = 0;
                return result;
            }

            // Test for separation on the axis C0 + t*A1[1].
            r = std::fabs(Dot(D, A1[1]));
            r0 = E0[0] * absDot01[0][1] + E0[1] * absDot01[1][1] + E0[2] * absDot01[2][1];
            r01 = r0 + E1[1];
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = -1;
                result.separating[1] = 1;
                return result;
            }

            // Test for separation on the axis C0 + t*A1[2].
            r = std::fabs(Dot(D, A1[2]));
            r0 = E0[0] * absDot01[0][2] + E0[1] * absDot01[1][2] + E0[2] * absDot01[2][2];
            r01 = r0 + E1[2];
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = -1;
                result.separating[1] = 2;
                return result;
            }

            // At least one pair of box axes was parallel, so the separation is
            // effectively in 2D. The edge-edge axes do not need to be tested.
            if (existsParallelPair)
            {
                // The result.separating[] values are invalid because there is
                // no separation.
                result.intersect = true;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[0]xA1[0].
            r = std::fabs(dotDA0[2] * dot01[1][0] - dotDA0[1] * dot01[2][0]);
            r0 = E0[1] * absDot01[2][0] + E0[2] * absDot01[1][0];
            r1 = E1[1] * absDot01[0][2] + E1[2] * absDot01[0][1];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 0;
                result.separating[1] = 0;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[0]xA1[1].
            r = std::fabs(dotDA0[2] * dot01[1][1] - dotDA0[1] * dot01[2][1]);
            r0 = E0[1] * absDot01[2][1] + E0[2] * absDot01[1][1];
            r1 = E1[0] * absDot01[0][2] + E1[2] * absDot01[0][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 0;
                result.separating[1] = 1;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[0]xA1[2].
            r = std::fabs(dotDA0[2] * dot01[1][2] - dotDA0[1] * dot01[2][2]);
            r0 = E0[1] * absDot01[2][2] + E0[2] * absDot01[1][2];
            r1 = E1[0] * absDot01[0][1] + E1[1] * absDot01[0][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 0;
                result.separating[1] = 2;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[1]xA1[0].
            r = std::fabs(dotDA0[0] * dot01[2][0] - dotDA0[2] * dot01[0][0]);
            r0 = E0[0] * absDot01[2][0] + E0[2] * absDot01[0][0];
            r1 = E1[1] * absDot01[1][2] + E1[2] * absDot01[1][1];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 1;
                result.separating[1] = 0;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[1]xA1[1].
            r = std::fabs(dotDA0[0] * dot01[2][1] - dotDA0[2] * dot01[0][1]);
            r0 = E0[0] * absDot01[2][1] + E0[2] * absDot01[0][1];
            r1 = E1[0] * absDot01[1][2] + E1[2] * absDot01[1][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 1;
                result.separating[1] = 1;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[1]xA1[2].
            r = std::fabs(dotDA0[0] * dot01[2][2] - dotDA0[2] * dot01[0][2]);
            r0 = E0[0] * absDot01[2][2] + E0[2] * absDot01[0][2];
            r1 = E1[0] * absDot01[1][1] + E1[1] * absDot01[1][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 1;
                result.separating[1] = 2;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[2]xA1[0].
            r = std::fabs(dotDA0[1] * dot01[0][0] - dotDA0[0] * dot01[1][0]);
            r0 = E0[0] * absDot01[1][0] + E0[1] * absDot01[0][0];
            r1 = E1[1] * absDot01[2][2] + E1[2] * absDot01[2][1];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 2;
                result.separating[1] = 0;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[2]xA1[1].
            r = std::fabs(dotDA0[1] * dot01[0][1] - dotDA0[0] * dot01[1][1]);
            r0 = E0[0] * absDot01[1][1] + E0[1] * absDot01[0][1];
            r1 = E1[0] * absDot01[2][2] + E1[2] * absDot01[2][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 2;
                result.separating[1] = 1;
                return result;
            }

            // Test for separation on the axis C0 + t*A0[2]xA1[2].
            r = std::fabs(dotDA0[1] * dot01[0][2] - dotDA0[0] * dot01[1][2]);
            r0 = E0[0] * absDot01[1][2] + E0[1] * absDot01[0][2];
            r1 = E1[0] * absDot01[2][1] + E1[1] * absDot01[2][0];
            r01 = r0 + r1;
            if (r > r01)
            {
                result.intersect = false;
                result.separating[0] = 2;
                result.separating[1] = 2;
                return result;
            }

            // The result.separating[] values are invalid because there is no
            // separation.
            result.intersect = true;
            return result;
        }
    };
}
