// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the box to be a solid.
//
// The test-intersection query uses the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The set of potential separating directions includes the 2 edge normals
// of box0 and the 2 edge normals of box1.  The integer 'separating'
// identifies the axis that reported separation; there may be more than one
// but only one is reported.  The value is 0 when box0.axis[0] separates,
// 1 when box0.axis[1] separates, 2 when box1.axis[0] separates, or 3 when
// box1.axis[1] separates.

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, AlignedBox2<T>, OrientedBox2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                separating(0)
            {
            }

            bool intersect;
            int32_t separating;
        };

        Result operator()(AlignedBox2<T> const& box0, OrientedBox2<T> const& box1)
        {
            Result result{};

            // Get the centered form of the aligned box.  The axes are
            // implicitly A0[0] = (1,0) and A0[1] = (0,1).
            Vector2<T> C0, E0;
            box0.GetCenteredForm(C0, E0);

            // Convenience variables.
            Vector2<T> const& C1 = box1.center;
            Vector2<T> const* A1 = &box1.axis[0];
            Vector2<T> const& E1 = box1.extent;

            // Compute difference of box centers.
            Vector2<T> D = C1 - C0;

            std::array<std::array<T, 2>, 2> absDot01{};
            T rSum{};

            // Test box0.axis[0] = (1,0).
            absDot01[0][0] = std::fabs(A1[0][0]);
            absDot01[0][1] = std::fabs(A1[1][0]);
            rSum = E0[0] + E1[0] * absDot01[0][0] + E1[1] * absDot01[0][1];
            if (std::fabs(D[0]) > rSum)
            {
                result.intersect = false;
                result.separating = 0;
                return result;
            }

            // Test axis box0.axis[1] = (0,1).
            absDot01[1][0] = std::fabs(A1[0][1]);
            absDot01[1][1] = std::fabs(A1[1][1]);
            rSum = E0[1] + E1[0] * absDot01[1][0] + E1[1] * absDot01[1][1];
            if (std::fabs(D[1]) > rSum)
            {
                result.intersect = false;
                result.separating = 1;
                return result;
            }

            // Test axis box1.axis[0].
            rSum = E1[0] + E0[0] * absDot01[0][0] + E0[1] * absDot01[1][0];
            if (std::fabs(Dot(A1[0], D)) > rSum)
            {
                result.intersect = false;
                result.separating = 2;
                return result;
            }

            // Test axis box1.axis[1].
            rSum = E1[1] + E0[0] * absDot01[0][1] + E0[1] * absDot01[1][1];
            if (std::fabs(Dot(A1[1], D)) > rSum)
            {
                result.intersect = false;
                result.separating = 3;
                return result;
            }

            result.intersect = true;
            return result;
        }
    };
}
