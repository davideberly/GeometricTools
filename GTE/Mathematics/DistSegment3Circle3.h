// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.2.2024.08.25

#pragma once

// The 3D segment-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document. The circle has
// center C and the plane of the circle has unit-length normal N. The segment
// has endpoints P0 and P1. The parameterization is P(t) = P0 + t * (P1 - P0)
// = P0 + t * M, where M is generally not a unit-length vector. The type T can
// be a floating-point type or a rational type.

#include <Mathematics/DistLine3Circle3.h>
#include <Mathematics/DistPoint3Circle3.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Segment3<T>, Circle3<T>>
    {
    public:
        // THe file DistLine3Circle3.h contains the definition of
        // DCPQuery<T,Line3<T>,Circle3<T>>::Result. This structure is also
        // used by DCPQuery<T,Segment3<T>,Circle3<T>> to avoid having a
        // separate structure with segmentClosest[] rather than lineClosest[].
        // This avoids copying data between two equivalent structures.
        using LCQuery = DCPQuery<T, Line3<T>, Circle3<T>>;
        using Result = typename LCQuery::Result;

        Result operator()(Segment3<T> const& segment, Circle3<T> const& circle)
        {
            Result result{};
            Critical critical{};
            Execute(segment, circle, result, critical);
            return result;
        }

    private:
        using Critical = typename LCQuery::Critical;
        using PCQuery = DCPQuery<T, Vector3<T>, Circle3<T>>;
        using PCOutput = typename PCQuery::Result;

        void Execute(Segment3<T> const& segment, Circle3<T> const& circle,
            Result& result, Critical& critical)
        {
            // Compute the line points closest to the circle. The line is
            // L(t) = P + t * D for any real-valued t. The segment restricts
            // 0 <= t <= 1 and has endpoints P0 = L(0) and P1 = L(1) with
            // D = P1 - P0.
            Line3<T> line(segment.p[0], segment.p[1] - segment.p[0]);
            LCQuery{}.Execute(line, circle, result, critical);

            // Clamp the query output to the ray domain.
            if (critical.numPoints == 1)
            {
                HasOneCriticalPoint(segment, circle, critical, result);
            }
            else
            {
                HasTwoCriticalPoints(segment, circle, critical, result);
            }
        }

        void HasOneCriticalPoint(Segment3<T> const& segment, Circle3<T> const& circle,
            Critical const& critical, Result& result)
        {
            T const& t0 = critical.parameter[0];

            T const one = static_cast<T>(1);
            if (t0 >= one)
            {
                // The critical point is not on the segment except possibly
                // the first critical point being the right endpoint of the
                // segment. The right endpoint is the segment point closest to
                // circle. See the left red segment of the one-critical-point
                // graph of figure 8 in the PDF.
                return SegmentEndpointClosest(segment.p[1], circle, result);
            }

            T const zero = static_cast<T>(0);
            if (t0 <= zero)
            {
                // The critical points are not on the segment except possibly
                // the critical point being the left endpoint of the segment.
                // The left endpoint is the segment point closest to the
                // circle. See the right red segment of the one-critical-point
                // graph of figure 8 in the PDF.
                return SegmentEndpointClosest(segment.p[0], circle, result);
            }
            
            // At this time, 0 < t0 < 1. The closest line-circle pair is the
            // closest segment-circle pair. The output does not need to be
            // modified. See the green segment of the one-critical-point graph
            // of figure 8 in the PDF.
        }

        void HasTwoCriticalPoints(Segment3<T> const& segment, Circle3<T> const& circle,
            Critical const& critical, Result& result)
        {
            T const& t0 = critical.parameter[0];
            T const& t1 = critical.parameter[1];

            T const one = static_cast<T>(1);
            if (t0 >= one)
            {
                // The critical points are not on the segment except possibly
                // the first critical point being the right endpoint of the
                // segment. The right endpoint is the segment point closest to
                // the circle. See the left red segment of the
                // two-point-critical graphs of figure 8 in the PDF.
                return SegmentEndpointClosest(segment.p[1], circle, result);
            }

            T const zero = static_cast<T>(0);
            if (t1 <= zero)
            {
                // The critical points are not on the segment except possibly
                // the second critical point being the left endpoint of the
                // segment. The left endpoint is the segment point closest to
                // the circle. See the right red segment of the
                // two-point-critical graphs of figure 8 in the PDF.
                return SegmentEndpointClosest(segment.p[0], circle, result);
            }

            // At this time, t0 < 1 and t1 > 0.
            if (zero <= t0 && t1 <= one)
            {
                // At this time, 0 <= t0 < t1 <= 1. The critical points are on
                // the segment, so the closest segment-circle pairs are the
                // closest line-circle pairs. The output does not need to be
                // modified. See the green segment of the two-critical-point
                // graphs of figure 8 in the PDF.
                return;
            }

            // At this time, t0 < 0 or t1 > 1. At most one critical point is
            // on the segment.
            if (t0 < zero)
            {
                if (t1 >= one)
                {
                    // At this time, t0 < 0 < 1 <= t1. The critical points are
                    // not on the segment except possibly the second critical
                    // point is the right endpoint. See the orange segment of
                    // the two-critical-point graphs of figure 8 in the PDF.
                    SelectClosestPoint(segment.p[0], segment.p[1], circle, result);
                }
                else // t1 < 1
                {
                    // At this time, t0 < 0 < t1 < 1. The critical point at t1
                    // is on the segment but is not an endpoint. See the
                    // purple segment of the two-critical-point graphs of
                    // figure 8 in the PDF.
                    SelectClosestPoint(segment.p[0], critical.linearPoint[1], circle, result);
                }
            }
            else // t1 > 1
            {
                if (t0 <= zero)
                {
                    // At this time, t0 <= 0 < 1 < t1. The critical points are
                    // not on the segment except possibly the first critical
                    // point is the left endpoint. See the orange segment of
                    // the two-critical-point graphs of figure 8 in the PDF.
                    SelectClosestPoint(segment.p[0], segment.p[1], circle, result);
                }
                else // t0 > 0
                {
                    // At this time, 0 < t0 < 1 < t1. The critical point at t0
                    // is on the segment but is not an endpoint. See the gold
                    // segment of the two-critical-point graphs of figure 8 in
                    // the PDF.
                    SelectClosestPoint(segment.p[1], critical.linearPoint[0], circle, result);
                }
            }
        }

        void SegmentEndpointClosest(Vector3<T> const& segmentEndpoint,
            Circle3<T> const& circle, Result& result)
        {
            PCOutput pcOutput = PCQuery{}(segmentEndpoint, circle);
            result.numClosestPairs = 1;
            result.linearClosest[0] = segmentEndpoint;
            result.linearClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
            result.circularClosest[0] = pcOutput.closest[1];
            result.circularClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
            result.distance = pcOutput.distance;
            result.sqrDistance = result.distance * result.distance;
        }

        void SelectClosestPoint(Vector3<T> const& point0, Vector3<T> const& point1,
            Circle3<T> const& circle, Result& result)
        {
            PCOutput pcOutput0 = PCQuery{}(point0, circle);
            PCOutput pcOutput1 = PCQuery{}(point1, circle);
            if (pcOutput0.distance < pcOutput1.distance)
            {
                result.numClosestPairs = 1;
                result.linearClosest[0] = point0;
                result.linearClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
                result.circularClosest[0] = pcOutput0.closest[1];
                result.circularClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
                result.distance = pcOutput0.distance;
                result.sqrDistance = result.distance * result.distance;
            }
            else if (pcOutput0.distance > pcOutput1.distance)
            {
                result.numClosestPairs = 1;
                result.linearClosest[0] = point1;
                result.linearClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
                result.circularClosest[0] = pcOutput1.closest[1];
                result.circularClosest[1] = { static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
                result.distance = pcOutput1.distance;
                result.sqrDistance = result.distance * result.distance;
            }
            else // pcOutput0.distance = pcOutput1.distance
            {
                result.numClosestPairs = 2;
                result.linearClosest[0] = point0;
                result.linearClosest[1] = point1;
                result.circularClosest[0] = pcOutput0.closest[1];
                result.circularClosest[1] = pcOutput1.closest[1];
                result.distance = pcOutput0.distance;
                result.sqrDistance = result.distance * result.distance;
            }
        }
    };
}
