// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// The queries consider the cylinder to be a solid.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Cylinder.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Vector3<T>, Cylinder3<T>>
    {
    public:
        // The input point is stored in the member closest[0]. The cylinder
        // point closest to it is stored in the member closest[1].
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<Vector3<T>, 2> closest;
        };

        // TODO: Add height management to Cylinder just as it is done with
        // Cone. This avoids having to use std::numeric_limits, which has no
        // implementation for rational arithmetic. FOR NOW: If the input
        // cylinder is infinite, set cylinder.height = -1 as an indicator.
        Output operator()(Vector3<T> const& point, Cylinder3<T> const& cylinder)
        {
            Output output{};
            output.closest[0] = point;

            // Convert the point to the cylinder coordinate system. In this
            // system, the point believes (0,0,0) is the cylinder axis origin
            // and (0,0,1) is the cylinder axis direction.
            Vector3<T> U0, U1, U2 = cylinder.direction;
            ComputeOrthonormalBasis(1, U2, U0, U1);

            Vector3<T> delta = point - cylinder.center;
            Vector3<T> P{ Dot(U0, delta), Dot(U1, delta), Dot(U2, delta) };

            if (cylinder.height == -C_<T>(1))
            {
                DoQueryInfiniteCylinder(P, cylinder.radius, output);
            }
            else
            {
                DoQueryFiniteCylinder(P, cylinder.radius, cylinder.height, output);
            }

            // Convert the closest point from the cylinder coordinate system
            // to the original coordinate system.
            output.closest[1] = cylinder.center +
                output.closest[1][0] * U0 +
                output.closest[1][1] * U1 +
                output.closest[1][2] * U2;

            return output;
        }

    private:
        void DoQueryInfiniteCylinder(Vector3<T> const& P, T const& radius, Output& output)
        {
            T sqrRadius = radius * radius;
            T sqrDistance = P[0] * P[0] + P[1] * P[1];
            if (sqrDistance >= sqrRadius)
            {
                // The point is outside the cylinder or on the cylinder wall.
                T distance = std::sqrt(sqrDistance);
                output.distance = distance - radius;
                output.sqrDistance = output.distance * output.distance;
                T temp = radius / distance;
                output.closest[1][0] = P[0] * temp;
                output.closest[1][1] = P[1] * temp;
                output.closest[1][2] = P[2];
            }
            else
            {
                // The point is inside the cylinder.
                output.distance = C_<T>(0);
                output.sqrDistance = C_<T>(0);
                output.closest[1] = P;
            }
        }

        void DoQueryFiniteCylinder(Vector3<T> const& P, T const& radius,
            T const& height, Output& output)
        {
            DoQueryInfiniteCylinder(P, radius, output);

            // Clamp the infinite cylinder's closest point to the finite
            // cylinder.
            if (output.closest[1][2] > height)
            {
                output.closest[1][2] = height;
                Vector3<T> diff = output.closest[1] - P;
                output.sqrDistance = Dot(diff, diff);
                output.distance = std::sqrt(output.sqrDistance);
            }
            else if (output.closest[1][2] < -height)
            {
                output.closest[1][2] = -height;
                Vector3<T> diff = output.closest[1] - P;
                output.sqrDistance = Dot(diff, diff);
                output.distance = std::sqrt(output.sqrDistance);
            }
        }
    };
}
