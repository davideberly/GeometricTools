// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the capsule to be a solid.
//
// The test-intersection queries are based on distance computations.

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/DistLineSegment.h>
#include <Mathematics/Capsule.h>
#include <Mathematics/Vector3.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line3<T>, Capsule3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Line3<T> const& line, Capsule3<T> const& capsule)
        {
            Result result{};
            DCPQuery<T, Line3<T>, Segment3<T>> lsQuery{};
            auto lsResult = lsQuery(line, capsule.segment);
            result.intersect = (lsResult.distance <= capsule.radius);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line3<T>, Capsule3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                parameter{ static_cast<T>(0), static_cast<T>(0) },
                point{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            bool intersect;
            size_t numIntersections;
            std::array<T, 2> parameter;
            std::array<Vector3<T>, 2> point;
        };

        Result operator()(Line3<T> const& line, Capsule3<T> const& capsule)
        {
            Result result{};
            DoQuery(line.origin, line.direction, capsule, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = line.origin + result.parameter[i] * line.direction;
                }
            }
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector3<T> const& lineOrigin,
            Vector3<T> const& lineDirection, Capsule3<T> const& capsule,
            Result& result)
        {
            // Create a coordinate system for the capsule. In this system,
            // the capsule segment center C is the origin and the capsule axis
            // direction W is the z-axis. U and V are the other coordinate
            // axis directions. If P = x*U+y*V+z*W, the cylinder containing
            // the capsule wall is x^2 + y^2 = r^2, where r is the capsule
            // radius. The finite cylinder that makes up the capsule minus
            // its hemispherical end caps has z-values |z| <= e, where e is
            // the extent of the capsule segment. The top hemisphere cap is
            // x^2+y^2+(z-e)^2 = r^2 for z >= e and the bottom hemisphere cap
            // is x^2+y^2+(z+e)^2 = r^2 for z <= -e.

            Vector3<T> segOrigin{};     // P
            Vector3<T> segDirection{};  // D
            T segExtent{};              // e
            capsule.segment.GetCenteredForm(segOrigin, segDirection, segExtent);
            std::array<Vector3<T>, 3> basis{};  // {W, U, V}
            basis[0] = segDirection;
            ComputeOrthogonalComplement(1, basis.data());
            T rSqr = capsule.radius * capsule.radius;
            Vector3<T> const& W = basis[0];
            Vector3<T> const& U = basis[1];
            Vector3<T> const& V = basis[2];

            // Convert incoming line origin to capsule coordinates.
            Vector3<T> diff = lineOrigin - segOrigin;
            Vector3<T> P{ Dot(U, diff), Dot(V, diff), Dot(W, diff) };

            // Get the z-value, in capsule coordinates, of the incoming line's
            // unit-length direction.
            T const zero = static_cast<T>(0);
            T dz = Dot(W, lineDirection);
            if (std::fabs(dz) == static_cast<T>(1))
            {
                // The line is parallel to the capsule axis. Determine
                // whether the line intersects the capsule hemispheres.
                T radialSqrDist = rSqr - P[0] * P[0] - P[1] * P[1];
                if (radialSqrDist >= zero)
                {
                    // The line intersects the hemispherical caps.
                    result.intersect = true;
                    result.numIntersections = 2;
                    T zOffset = std::sqrt(radialSqrDist) + segExtent;
                    if (dz > zero)
                    {
                        result.parameter[0] = -P[2] - zOffset;
                        result.parameter[1] = -P[2] + zOffset;
                    }
                    else
                    {
                        result.parameter[0] = P[2] - zOffset;
                        result.parameter[1] = P[2] + zOffset;
                    }
                }
                // else: The line outside the capsule's cylinder, no
                // intersection.
                return;
            }

            // Convert the incoming line unit-length direction to capsule
            // coordinates.
            Vector3<T> D{ Dot(U, lineDirection), Dot(V, lineDirection), dz };

            // Test intersection of line P+t*D with infinite cylinder
            // x^2+y^2 = r^2. This reduces to computing the roots of a
            // quadratic equation. If P = (px,py,pz) and D = (dx,dy,dz), then
            // the quadratic equation is
            //   (dx^2+dy^2)*t^2 + 2*(px*dx+py*dy)*t + (px^2+py^2-r^2) = 0
            T a0 = P[0] * P[0] + P[1] * P[1] - rSqr;
            T a1 = P[0] * D[0] + P[1] * D[1];
            T a2 = D[0] * D[0] + D[1] * D[1];
            T discr = a1 * a1 - a0 * a2;
            if (discr < zero)
            {
                // The line does not intersect the infinite cylinder, so it
                // cannot intersect the capsule.
                return;
            }

            T root, tValue, zValue;
            if (discr > zero)
            {
                // The line intersects the infinite cylinder in two places.
                root = std::sqrt(discr);
                tValue = (-a1 - root) / a2;
                zValue = P[2] + tValue * D[2];
                if (std::fabs(zValue) <= segExtent)
                {
                    result.intersect = true;
                    result.parameter[result.numIntersections++] = tValue;
                }

                tValue = (-a1 + root) / a2;
                zValue = P[2] + tValue * D[2];
                if (std::fabs(zValue) <= segExtent)
                {
                    result.intersect = true;
                    result.parameter[result.numIntersections++] = tValue;
                }

                if (result.numIntersections == 2)
                {
                    // The line intersects the capsule wall in two places.
                    return;
                }
            }
            else
            {
                // The line is tangent to the infinite cylinder but intersects
                // the cylinder in a single point.
                tValue = -a1 / a2;
                zValue = P[2] + tValue * D[2];
                if (std::fabs(zValue) <= segExtent)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.parameter[0] = tValue;
                    result.parameter[1] = result.parameter[0];
                    return;
                }
            }

            // Test intersection with bottom hemisphere. The quadratic
            // equation is
            //   t^2 + 2*(px*dx+py*dy+(pz+e)*dz)*t
            //     + (px^2+py^2+(pz+e)^2-r^2) = 0
            // Use the fact that currently a1 = px*dx+py*dy and
            // a0 = px^2+py^2-r^2. The leading coefficient is a2 = 1, so no
            // need to include in the construction.
            T PZpE = P[2] + segExtent;
            a1 += PZpE * D[2];
            a0 += PZpE * PZpE;
            discr = a1 * a1 - a0;
            if (discr > zero)
            {
                root = std::sqrt(discr);
                tValue = -a1 - root;
                zValue = P[2] + tValue * D[2];
                if (zValue <= -segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }

                tValue = -a1 + root;
                zValue = P[2] + tValue * D[2];
                if (zValue <= -segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }
            }
            else if (discr == zero)
            {
                tValue = -a1;
                zValue = P[2] + tValue * D[2];
                if (zValue <= -segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }
            }

            // Test intersection with top hemisphere. The quadratic equation
            // is
            //   t^2 + 2*(px*dx+py*dy+(pz-e)*dz)*t
            //     + (px^2+py^2+(pz-e)^2-r^2) = 0
            // Use the fact that currently a1 = px*dx+py*dy+(pz+e)*dz and
            // a0 = px^2+py^2+(pz+e)^2-r^2. The leading coefficient is a2 = 1,
            // so no need to include in the construction.
            a1 -= static_cast<T>(2) * segExtent * D[2];
            a0 -= static_cast<T>(4) * segExtent * P[2];
            discr = a1 * a1 - a0;
            if (discr > zero)
            {
                root = std::sqrt(discr);
                tValue = -a1 - root;
                zValue = P[2] + tValue * D[2];
                if (zValue >= segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }

                tValue = -a1 + root;
                zValue = P[2] + tValue * D[2];
                if (zValue >= segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }
            }
            else if (discr == zero)
            {
                tValue = -a1;
                zValue = P[2] + tValue * D[2];
                if (zValue >= segExtent)
                {
                    result.parameter[result.numIntersections++] = tValue;
                    if (result.numIntersections == 2)
                    {
                        result.intersect = true;
                        if (result.parameter[0] > result.parameter[1])
                        {
                            std::swap(result.parameter[0], result.parameter[1]);
                        }
                        return;
                    }
                }
            }

            if (result.numIntersections == 1)
            {
                result.parameter[1] = result.parameter[0];
            }
        }
    };
}
