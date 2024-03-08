// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/IntrPlane3Plane3.h>
#include <Mathematics/Circle3.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Plane3<T>, Circle3<T>>
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

        Result operator()(Plane3<T> const& plane, Circle3<T> const& circle)
        {
            Result result{};

            // Construct the plane of the circle.
            Plane3<T> cPlane(circle.normal, circle.center);

            // Compute the intersection of this plane with the input plane.
            FIQuery<T, Plane3<T>, Plane3<T>> ppQuery{};
            auto ppResult = ppQuery(plane, cPlane);
            if (!ppResult.intersect)
            {
                // The planes are parallel and nonintersecting.
                result.intersect = false;
                return result;
            }

            if (!ppResult.isLine)
            {
                // The planes are the same, so the circle is the set of
                // intersection.
                result.intersect = true;
                return result;
            }

            // The planes intersect in a line. Locate one or two points that
            // are on the circle and line. If the line is t*D+P, the circle
            // center is C and the circle radius is r, then
            //   r^2 = |t*D+P-C|^2 = |D|^2*t^2 + 2*Dot(D,P-C)*t + |P-C|^2
            // This is a quadratic equation of the form
            // a2*t^2 + 2*a1*t + a0 = 0.
            Vector3<T> diff = ppResult.line.origin - circle.center;
            T a2 = Dot(ppResult.line.direction, ppResult.line.direction);
            T a1 = Dot(diff, ppResult.line.direction);
            T a0 = Dot(diff, diff) - circle.radius * circle.radius;

            // T-valued roots imply an intersection.
            T discr = a1 * a1 - a0 * a2;
            result.intersect = (discr >= static_cast<T>(0));
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Plane3<T>, Circle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                point{ Vector3<T>::Zero(), Vector3<T>::Zero() },
                circle(Vector3<T>::Zero(), Vector3<T>::Zero(), static_cast<T>(0))
            {
            }

            // If 'intersect' is false, the set of intersection is empty.
            // 'numIntersections' is 0 and 'points[]' and 'circle' have
            // members all set to 0.
            // 
            // If 'intersect' is true, the set of intersection contains either
            // 1 or 2 points or the entire circle.
            //
            // (1) When the set of intersection has 1 point, the circle is
            //     just touching the plane. 'numIntersections' is 1 and
            //     'point[0]' and 'point[1]' are the same point. The
            //     'circle' is set to invalid (center at the origin, normal
            //     is the zero vector, radius is 0).
            // 
            // (2) When the set of intersection has 2 points, the plane cuts
            //     the circle into 2 arcs. 'numIntersections' is 2 and
            //     'point[0]' and 'point[1]' are the distinct intersection
            //     points. The 'circle' is set to invalid (center at the
            //     origin, normal is the zero vector, radius is 0).
            // 
            // (3) When the set of intersection contains the entire circle, the
            //     plane of the circle and the input plane are the same.
            //     'numIntersections' is std::numeric_limits<size_t>::max().
            //     'points[0]' and 'points[1]' are set to the zero vector.
            //     'circle' is set to the input circle.
            //     
            bool intersect;
            size_t numIntersections;
            std::array<Vector3<T>, 2> point;
            Circle3<T> circle;
        };

        Result operator()(Plane3<T> const& plane, Circle3<T> const& circle)
        {
            // The 'result' members have initial values set by the default
            // constructor. In each return block, only the relevant members
            // are modified.
            Result result{};

            // Construct the plane of the circle.
            Plane3<T> cPlane(circle.normal, circle.center);

            // Compute the intersection of this plane with the input plane.
            FIQuery<T, Plane3<T>, Plane3<T>> ppQuery;
            auto ppResult = ppQuery(plane, cPlane);
            if (!ppResult.intersect)
            {
                // The planes are parallel and nonintersecting.
                return result;
            }

            if (!ppResult.isLine)
            {
                // The planes are the same, so the circle is the set of
                // intersection.
                result.intersect = true;
                result.numIntersections = std::numeric_limits<size_t>::max();
                result.circle = circle;
                return result;
            }

            // The planes intersect in a line. Locate one or two points that
            // are on the circle and line. If the line is t*D+P, the circle
            // center is C, and the circle radius is r, then
            //   r^2 = |t*D+P-C|^2 = |D|^2*t^2 + 2*Dot(D,P-C)*t + |P-C|^2
            // This is a quadratic equation of the form
            // a2*t^2 + 2*a1*t + a0 = 0.
            Vector3<T> diff = ppResult.line.origin - circle.center;
            T a2 = Dot(ppResult.line.direction, ppResult.line.direction);
            T a1 = Dot(diff, ppResult.line.direction);
            T a0 = Dot(diff, diff) - circle.radius * circle.radius;

            T const zero = static_cast<T>(0);
            T discr = a1 * a1 - a0 * a2;
            if (discr < zero)
            {
                // No real roots, the circle does not intersect the plane.
                return result;
            }

            if (discr == zero)
            {
                // The quadratic polynomial has 1 real-valued repeated root.
                // The circle just touches the plane.
                result.intersect = true;
                result.numIntersections = 1;
                result.point[0] = ppResult.line.origin - (a1 / a2) * ppResult.line.direction;
                result.point[1] = result.point[0];
                return result;
            }

            // The quadratic polynomial has 2 distinct, real-valued roots.
            // The circle intersects the plane in two points.
            T root = std::sqrt(discr);
            result.intersect = true;
            result.numIntersections = 2;
            result.point[0] = ppResult.line.origin - ((a1 + root) / a2) * ppResult.line.direction;
            result.point[1] = ppResult.line.origin - ((a1 - root) / a2) * ppResult.line.direction;
            return result;
        }
    };
}
