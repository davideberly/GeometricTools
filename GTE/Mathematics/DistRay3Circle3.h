// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.2.2024.08.25

#pragma once

// The 3D ray-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document. The circle has
// center C and the plane of the circle has unit-length normal N. The ray has
// origin B and non-zero direction M. The parameterization is P(t) = t*M+B. It
// is not necessary that M be a unit-length vector. The type T can be a
// floating-point type or a rational type.

#include <Mathematics/DistLine3Circle3.h>
#include <Mathematics/DistPoint3Circle3.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Ray3<T>, Circle3<T>>
    {
    public:
        // THe file DistLine3Circle3.h contains the definition of
        // DCPQuery<T,Line3<T>,Circle3<T>>::Result. This structure is also
        // used by DCPQuery<T,Ray3<T>,Circle3<T>> to avoid having a separate
        // structure with rayClosest[] rather than lineClosest[]. This avoids
        // copying data between two equivalent structures.
        using LCQuery = DCPQuery<T, Line3<T>, Circle3<T>>;
        using Result = typename LCQuery::Result;

        Result operator()(Ray3<T> const& ray, Circle3<T> const& circle)
        {
            Result result{};
            Critical critical{};
            Execute(ray, circle, result, critical);
            return result;
        }

    private:
        using Critical = typename LCQuery::Critical;
        using PCQuery = DCPQuery<T, Vector3<T>, Circle3<T>>;
        using PCOutput = typename PCQuery::Result;

        void Execute(Ray3<T> const& ray, Circle3<T> const& circle,
            Result& result, Critical& critical)
        {
            // Compute the line points closest to the circle. The line is
            // L(t) = P + t * D for any real-valued t. The ray restricts
            // t >= 0 and has origin P = L(0).
            Line3<T> line(ray.origin, ray.direction);
            LCQuery{}.Execute(line, circle, result, critical);

            // Clamp the query output to the ray domain.
            if (critical.numPoints == 1)
            {
                HasOneCriticalPoint(ray, circle, critical, result);
            }
            else
            {
                HasTwoCriticalPoints(ray, circle, critical, result);
            }
        }

        void HasOneCriticalPoint(Ray3<T> const& ray, Circle3<T> const& circle,
            Critical const& critical, Result& result)
        {
            T const& t0 = critical.parameter[0];

            T const zero = static_cast<T>(0);
            if (t0 <= zero)
            {
                // The critical point is not on the ray. The ray origin is
                // the ray point closest to the circle. See the red ray of
                // the one-critical-point graph of figure 7 in the PDF.
                return RayOriginClosest(ray.origin, circle, result);
            }

            // At this time, t0 > 0. The closest line-circle pair is the
            // closest ray-origin circle pair. The output does not need to be
            // modified. See the green ray of the one-critical-point graph of
            // figure 7 in the PDF.
        }

        void HasTwoCriticalPoints(Ray3<T> const& ray, Circle3<T> const& circle,
            Critical const& critical, Result& result)
        {
            T const& t0 = critical.parameter[0];
            T const& t1 = critical.parameter[1];

            T const zero = static_cast<T>(0);
            if (t0 >= zero)
            {
                // The critical points are on the ray. The ray point closest
                // to the circle is the line point closest to the circle. The
                // output remains unchanged. See the green rays of the
                // two-critical-point graphs of figure 7 in the PDF.
                return;
            }

            if (t1 <= zero)
            {
                // The critical points are not on the ray. The ray origin is
                // the ray point closest to the circle. See the red rays of
                // the two-critical-point graphs of figure 7 in the PDF.
                return RayOriginClosest(ray.origin, circle, result);
            }

            // The ray point closest to the circle is either the ray origin or
            // the second critical point, whichever has minimum distance. See
            // the orange and purple rays of the two-critical-point graphs of
            // figure 7 in the PDF.
            SelectClosestPoint(ray.origin, critical.linearPoint[1], circle, result);
        }

        void RayOriginClosest(Vector3<T> const& rayOrigin, Circle3<T> const& circle,
            Result& output)
        {
            PCOutput pcOutput = PCQuery{}(rayOrigin, circle);
            output.numClosestPairs = 1;
            output.linearClosest[0] = rayOrigin;
            output.linearClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
            output.circularClosest[0] = pcOutput.closest[1];
            output.circularClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
            output.distance = pcOutput.distance;
            output.sqrDistance = output.distance * output.distance;
        }

        void SelectClosestPoint(Vector3<T> const& point0, Vector3<T> const& point1,
            Circle3<T> const& circle, Result& output)
        {
            PCOutput pcOutput0 = PCQuery{}(point0, circle);
            PCOutput pcOutput1 = PCQuery{}(point1, circle);
            if (pcOutput0.distance < pcOutput1.distance)
            {
                output.numClosestPairs = 1;
                output.linearClosest[0] = point0;
                output.linearClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
                output.circularClosest[0] = pcOutput0.closest[1];
                output.circularClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
                output.distance = pcOutput0.distance;
                output.sqrDistance = output.distance * output.distance;
            }
            else if (pcOutput0.distance > pcOutput1.distance)
            {
                output.numClosestPairs = 1;
                output.linearClosest[0] = point1;
                output.linearClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
                output.circularClosest[0] = pcOutput1.closest[1];
                output.circularClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
                output.distance = pcOutput1.distance;
                output.sqrDistance = output.distance * output.distance;
            }
            else // pcOutput0.distance = pcOutput1.distance
            {
                output.numClosestPairs = 2;
                output.linearClosest[0] = point0;
                output.linearClosest[1] = point1;
                output.circularClosest[0] = pcOutput0.closest[1];
                output.circularClosest[1] = pcOutput1.closest[1];
                output.distance = pcOutput0.distance;
                output.sqrDistance = output.distance * output.distance;
            }
        }
    };
}
