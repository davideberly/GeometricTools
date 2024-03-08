// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the ellipsoid to be a solid.
//
// The ellipsoid is (X-C)^T*M*(X-C)-1 = 0. The segment has endpoints P0 and
// P1. The segment origin (center) is P = (P0+P1)/2, the segment direction is
// D = (P1-P0)/|P1-P0| and the segment extent (half the segment length) is
// e = |P1-P0|/2. The segment is X = P+t*D for t in [-e,e]. Substitute the
// segment equation into the ellipsoid equation to obtain a quadratic equation
// Q(t) = a2*t^2 + 2*a1*t + a0 = 0, where a2 = D^T*M*D,
// a1 = (P1-P0)^T*M*(P0-C) and a0 = (P0-C)^T*M*(P0-C)-r^2. The algorithm
// involves an analysis of the real-valued roots of Q(t) for -e <= t <= e.

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3Ellipsoid3.h>
#include <Mathematics/Segment.h>
#include <Mathematics/Matrix3x3.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment3<T>, Ellipsoid3<T>>
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

        Result operator()(Segment3<T> const& segment, Ellipsoid3<T> const& ellipsoid)
        {
            Result result{};

            Vector3<T> segOrigin{}, segDirection{};
            T segExtent{};
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Matrix3x3<T> M{};
            ellipsoid.GetM(M);
            T const zero = static_cast<T>(0);
            Vector3<T> diff = segOrigin - ellipsoid.center;
            Vector3<T> matDir = M * segDirection;
            Vector3<T> matDiff = M * diff;
            T a0 = Dot(diff, matDiff) - static_cast<T>(1);
            T a1 = Dot(segDirection, matDiff);
            T a2 = Dot(segDirection, matDir);
            T discr = a1 * a1 - a0 * a2;
            if (discr < zero)
            {
                // Q(t) has no real-valued roots. The segment does not
                // intersect the ellipsoid.
                result.intersect = false;
                return result;
            }

            // Q(-e) = a2*e^2 - 2*a1*e + a0, Q(e) = a2*e^2 + 2*a1*e + a0
            T a2e = a2 * segExtent;
            T tmp0 = a2e * segExtent + a0;  // a2*e^2 + a0
            T tmp1 = static_cast<T>(2) * a1 * segExtent;  // 2*a1*e
            T qm = tmp0 - tmp1;  // Q(-e)
            T qp = tmp0 + tmp1;  // Q(e)
            if (qm * qp <= zero)
            {
                // Q(t) has a root on the interval [-e,e]. The segment
                // intesects the ellipsoid.
                result.intersect = true;
                return result;
            }

            // Either (Q(-e) > 0 and Q(e) > 0) or (Q(-e) < 0 and Q(e) < 0).
            // When Q at the endpoints is negative, Q(t) < 0 for all t in
            // [-e,e] and the segment does not intersect the ellipsoid.
            // Otherwise, Q(-e) > 0 [and Q(e) > 0]. The minimum of Q(t)
            // occurs at t = -a1/a2. We know that discr >= 0, so Q(t) has a
            // root on (-e,e) when -a1/2 is in (-e,e). The combined test for
            // intersection is (Q(-e) > 0 and |a1| < a3*e).
            result.intersect = (qm > zero && std::fabs(a1) < a2e);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Segment3<T>, Ellipsoid3<T>>
        :
        public FIQuery<T, Line3<T>, Ellipsoid3<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line3<T>, Ellipsoid3<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line3<T>, Ellipsoid3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Segment3<T> const& segment, Ellipsoid3<T> const& ellipsoid)
        {
            Vector3<T> segOrigin{}, segDirection{};
            T segExtent{};
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result{};
            DoQuery(segOrigin, segDirection, segExtent, ellipsoid, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = segOrigin + result.parameter[i] * segDirection;
                }
            }
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector3<T> const& segOrigin,
            Vector3<T> const& segDirection, T segExtent,
            Ellipsoid3<T> const& ellipsoid, Result& result)
        {
            FIQuery<T, Line3<T>, Ellipsoid3<T>>::DoQuery(
                segOrigin, segDirection, ellipsoid, result);

            if (result.intersect)
            {
                // The line containing the segment intersects the ellipsoid;
                // the t-interval is [t0,t1]. The segment intersects the
                // ellipsoid as long as [t0,t1] overlaps the segment
                // t-interval [-segExtent,+segExtent].
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                std::array<T, 2> segInterval = { -segExtent, segExtent };
                auto iiResult = iiQuery(result.parameter, segInterval);
                if (iiResult.intersect)
                {
                    result.numIntersections = iiResult.numIntersections;
                    result.parameter = iiResult.overlap;
                }
                else
                {
                    // The line containing the segment does not intersect
                    // the ellipsoid.
                    result = Result{};
                }
            }
        }
    };
}
