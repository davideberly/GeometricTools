// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// Least-squares fit of a rectangle to a convex quadrilateral that is nearly
// a rectangle. The algorithm is described in
//   https://www.geometrictools.com/Documentation/FitConvexQuadByRect.pdf

#include <Mathematics/OrientedBox.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cstddef>
#include <cmath>

namespace gte
{
    template <typename T>
    class ApprConvexQuadByRect
    {
    public:
        static OrientedBox2<T> Fit(std::array<Vector2<T>, 4> const& P)
        {
            OrientedBox2<T> rectangle{};

            rectangle.center = static_cast<T>(0.25) * (P[0] + P[1] + P[2] + P[3]);

            std::array<Vector2<T>, 4> Q{};
            for (size_t i = 0; i < 4; ++i)
            {
                Q[i] = P[i] - rectangle.center;
            }

            Vector2<T> Q0mQ2 = Q[0] - Q[2];
            Vector2<T> Q1mQ3 = Q[1] - Q[3];
            T k0 = Q1mQ3[0] * Q0mQ2[1] + Q0mQ2[0] * Q1mQ3[1];
            T k1 = Q1mQ3[0] * Q0mQ2[0] - Q1mQ3[1] * Q0mQ2[1];
            T theta = static_cast<T>(0.5) * std::atan2(k0, k1);
            T cosTheta = std::cos(theta);
            T sinTheta = std::sin(theta);
            rectangle.axis[0] = { cosTheta, sinTheta };
            rectangle.axis[1] = { -sinTheta, cosTheta };

            T s0ms2 = Dot(rectangle.axis[0], Q0mQ2);
            T s1ms3 = Dot(rectangle.axis[0], Q1mQ3);
            T t0mt2 = Dot(rectangle.axis[1], Q0mQ2);
            T t1mt3 = Dot(rectangle.axis[1], Q1mQ3);
            T gder2Test = s0ms2 * s1ms3 - t0mt2 * t1mt3;
            if (gder2Test < static_cast<T>(0))
            {
                rectangle.axis[0] = { -sinTheta, cosTheta };
                rectangle.axis[1] = { -cosTheta, -sinTheta };
                T oldt0mt2 = t0mt2;
                T oldt1mt3 = t1mt3;
                t0mt2 = -s0ms2;
                t1mt3 = -s1ms3;
                s0ms2 = oldt0mt2;
                s1ms3 = oldt1mt3;
            }
            rectangle.extent[0] = static_cast<T>(0.25) * (s0ms2 + s1ms3);
            rectangle.extent[1] = static_cast<T>(0.25) * (t0mt2 - t1mt3);

            return rectangle;
        }
    };
}
