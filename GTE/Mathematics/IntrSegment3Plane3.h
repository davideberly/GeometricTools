// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/IntrLine3Plane3.h>
#include <Mathematics/Segment.h>
#include <cmath>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment3<T>, Plane3<T>>
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

        Result operator()(Segment3<T> const& segment, Plane3<T> const& plane)
        {
            Result result{};

            // Compute the (signed) distance from the segment endpoints to the
            // plane.
            DCPQuery<T, Vector3<T>, Plane3<T>> vpQuery{};
            T sdistance0 = vpQuery(segment.p[0], plane).signedDistance;
            if (sdistance0 == (T)0)
            {
                // Endpoint p[0] is on the plane.
                result.intersect = true;
                return result;
            }

            T sdistance1 = vpQuery(segment.p[1], plane).signedDistance;
            if (sdistance1 == (T)0)
            {
                // Endpoint p[1] is on the plane.
                result.intersect = true;
                return result;
            }

            // Test whether the segment transversely intersects the plane.
            result.intersect = (sdistance0 * sdistance1 < (T)0);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Segment3<T>, Plane3<T>>
        :
        public FIQuery<T, Line3<T>, Plane3<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line3<T>, Plane3<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line3<T>, Plane3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Segment3<T> const& segment, Plane3<T> const& plane)
        {
            Vector3<T> segOrigin{}, segDirection{};
            T segExtent{};
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result{};
            DoQuery(segOrigin, segDirection, segExtent, plane, result);
            if (result.intersect)
            {
                result.point = segOrigin + result.parameter * segDirection;
            }
            return result;
        }

    protected:
        void DoQuery(Vector3<T> const& segOrigin,
            Vector3<T> const& segDirection, T segExtent,
            Plane3<T> const& plane, Result& result)
        {
            FIQuery<T, Line3<T>, Plane3<T>>::DoQuery(segOrigin,
                segDirection, plane, result);

            if (result.intersect)
            {
                // The line intersects the plane in a point that might not be
                // on the segment.
                if (std::fabs(result.parameter) > segExtent)
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
        }
    };
}
