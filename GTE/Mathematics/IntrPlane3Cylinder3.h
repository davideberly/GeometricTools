// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.11.11

#pragma once

// The intersection queries between a plane and a cylinder (finite or
// infinite) are described in
// https://www.geometrictools.com/Documentation/IntersectionCylinderPlane.pdf
//
// The plane is Dot(N, X - P) = 0, where P is a point on the plane and N is a
// nonzero vector that is not necessarily unit length.
// 
// The cylinder is (X - C)^T * (I - W * W^T) * (X - C) = r^2, where C is the
// center, W is the axis direction and r > 0 is the radius. The cylinder has
// height h. In the intersection queries, an infinite cylinder is specified
// by setting h = -1. Read the aforementioned PDF for details about this
// choice.

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/Circle3.h>
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Ellipse3.h>

// TODO: TEMPORARY UNTIL CODE IS FIXED
#include <Mathematics/DistPointHyperplane.h>

namespace gte
{
    template <typename T>
    class TIQuery<T, Plane3<T>, Cylinder3<T>>
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

        // For an infinite cylinder, set cylinder.height = -1. For a finite
        // cylinder, set cylinder.height >= 0.
        Result operator()(Plane3<T> const& plane, Cylinder3<T> const& cylinder)
        {
            Result result{};

            if (cylinder.IsInfinite())
            {
                if (Dot(plane.normal, cylinder.axis.direction) != static_cast<T>(0))
                {
                    result.intersect = true;
                    return result;
                }
                else
                {
                    Vector3<T> CmP = cylinder.axis.origin - plane.origin;
                    Vector3<T> const& N = plane.normal;
                    T dotNDelta = Dot(N, CmP);
                    T dotNN = Dot(N, N);
                    T rSqr = cylinder.radius * cylinder.radius;
                    result.intersect = (dotNDelta * dotNDelta <= dotNN * rSqr);
                    return result;
                }
            }
            else
            {
                DCPQuery<T, Vector3<T>, Plane3<T>> vpQuery{};
                T distance = vpQuery(cylinder.axis.origin, plane).distance;
                T absNdW = std::fabs(Dot(plane.normal, cylinder.axis.direction));
                T root = std::sqrt(std::max((T)1 - absNdW * absNdW, (T)0));
                T term = cylinder.radius * root + (T)0.5 * cylinder.height * absNdW;

                // Intersection occurs if and only if 0 is in the interval
                // [min,max].
                result.intersect = (distance <= term);
            }

            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Plane3<T>, Cylinder3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                type(0),
                line{
                    Line3<T>(Vector3<T>::Zero(), Vector3<T>::Zero()),
                    Line3<T>(Vector3<T>::Zero(), Vector3<T>::Zero())},
                circle(Vector3<T>::Zero(), Vector3<T>::Zero(), (T)0),
                ellipse(Vector3<T>::Zero(), Vector3<T>::Zero(),
                    { Vector3<T>::Zero(), Vector3<T>::Zero() }, Vector2<T>::Zero())
            {
            }

            bool intersect;

            // The type of intersection.
            //   0: none
            //   1: single line (cylinder is tangent to plane), line[0] valid
            //   2: two parallel lines (plane cuts cylinder in two lines)
            //   3: circle (cylinder axis perpendicular to plane)
            //   4: ellipse (cylinder axis neither parallel nor perpendicular)
            int type;
            std::array<Line3<T>, 2> line;
            Circle3<T> circle;
            Ellipse3<T> ellipse;
        };

        // The cylinder must have infinite height.
        Result operator()(Plane3<T> const& plane, Cylinder3<T> const& cylinder)
        {
            LogAssert(cylinder.height != std::numeric_limits<T>::max(),
                "Cylinder height must be finite.");

            Result result{};

            DCPQuery<T, Vector3<T>, Plane3<T>> vpQuery;
            T sdistance = vpQuery(cylinder.axis.origin, plane).signedDistance;
            Vector3<T> center = cylinder.axis.origin - sdistance * plane.normal;
            T cosTheta = Dot(cylinder.axis.direction, plane.normal);
            T absCosTheta = std::fabs(cosTheta);

            if (absCosTheta > (T)0)
            {
                // The cylinder axis intersects the plane in a unique point.
                result.intersect = true;
                if (absCosTheta < (T)1)
                {
                    result.type = 4;
                    result.ellipse.normal = plane.normal;
                    result.ellipse.center = cylinder.axis.origin -
                        (sdistance / cosTheta) * cylinder.axis.direction;
                    result.ellipse.axis[0] = cylinder.axis.direction -
                        cosTheta * plane.normal;
                    Normalize(result.ellipse.axis[0]);
                    result.ellipse.axis[1] = UnitCross(plane.normal,
                        result.ellipse.axis[0]);
                    result.ellipse.extent[0] = cylinder.radius / absCosTheta;
                    result.ellipse.extent[1] = cylinder.radius;
                }
                else
                {
                    result.type = 3;
                    result.circle.normal = plane.normal;
                    result.circle.center = center;
                    result.circle.radius = cylinder.radius;
                }
            }
            else
            {
                // The cylinder is parallel to the plane.
                T distance = std::fabs(sdistance);
                if (distance < cylinder.radius)
                {
                    result.intersect = true;
                    result.type = 2;

                    Vector3<T> offset = Cross(cylinder.axis.direction, plane.normal);
                    T extent = std::sqrt(cylinder.radius * cylinder.radius - sdistance * sdistance);

                    result.line[0].origin = center - extent * offset;
                    result.line[0].direction = cylinder.axis.direction;
                    result.line[1].origin = center + extent * offset;
                    result.line[1].direction = cylinder.axis.direction;
                }
                else if (distance == cylinder.radius)
                {
                    result.intersect = true;
                    result.type = 1;
                    result.line[0].origin = center;
                    result.line[0].direction = cylinder.axis.direction;
                }
                else
                {
                    result.intersect = false;
                    result.type = 0;
                }
            }

            return result;
        }
    };
}
