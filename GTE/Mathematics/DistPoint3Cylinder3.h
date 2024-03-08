// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance from a point to a cylinder that is finite or infinite.
// The queries consider the cylinder to be a solid.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Vector3.h>
#include <array>
#include <cmath>
#include <limits>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Vector3<T>, Cylinder3<T>>
    {
    public:
        // The input point is stored in the member closest[0]. The cylinder
        // point closest to it is stored in the member closest[1].
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Vector3<T> const& point, Cylinder3<T> const& cylinder)
        {
            Result result;

            // Convert the point to the cylinder coordinate system.  In this
            // system, the point believes (0,0,0) is the cylinder axis origin
            // and (0,0,1) is the cylinder axis direction.
            std::array<Vector3<T>, 3> basis{};
            basis[0] = cylinder.axis.direction;
            ComputeOrthogonalComplement(1, basis.data());

            Vector3<T> delta = point - cylinder.axis.origin;
            Vector3<T> P
            {
                Dot(basis[1], delta),
                Dot(basis[2], delta),
                Dot(basis[0], delta)
            };

            if (cylinder.height == std::numeric_limits<T>::max())
            {
                DoQueryInfiniteCylinder(P, cylinder.radius, result);
            }
            else
            {
                DoQueryFiniteCylinder(P, cylinder.radius, cylinder.height, result);
            }

            // Convert the closest point from the cylinder coordinate system
            // to the original coordinate system.
            result.closest[0] = point;
            result.closest[1] = cylinder.axis.origin +
                result.closest[1][0] * basis[1] +
                result.closest[1][1] * basis[2] +
                result.closest[1][2] * basis[0];

            return result;
        }

    private:
        void DoQueryInfiniteCylinder(Vector3<T> const& P, T const& radius,
            Result& result)
        {
            T sqrRadius = radius * radius;
            T sqrDistance = P[0] * P[0] + P[1] * P[1];
            if (sqrDistance >= sqrRadius)
            {
                // The point is outside the cylinder or on the cylinder wall.
                T distance = std::sqrt(sqrDistance);
                result.distance = distance - radius;
                result.sqrDistance = result.distance * result.distance;
                T temp = radius / distance;
                result.closest[1][0] = P[0] * temp;
                result.closest[1][1] = P[1] * temp;
                result.closest[1][2] = P[2];
            }
            else
            {
                // The point is inside the cylinder.
                result.distance = (T)0;
                result.sqrDistance = (T)0;
                result.closest[1] = P;
            }
        }

        void DoQueryFiniteCylinder(Vector3<T> const& P, T const& radius,
            T const& height, Result& result)
        {
            DoQueryInfiniteCylinder(P, radius, result);

            // Clamp the infinite cylinder's closest point to the finite
            // cylinder.
            T halfHeight = static_cast<T>(0.5) * height;
            if (result.closest[1][2] > halfHeight)
            {
                result.closest[1][2] = halfHeight;
                Vector3<T> diff = result.closest[1] - P;
                result.sqrDistance = Dot(diff, diff);
                result.distance = std::sqrt(result.sqrDistance);
            }
            else if (result.closest[1][2] < -halfHeight)
            {
                result.closest[1][2] = -halfHeight;
                Vector3<T> diff = result.closest[1] - P;
                result.sqrDistance = Dot(diff, diff);
                result.distance = std::sqrt(result.sqrDistance);
            }
        }
    };
}
