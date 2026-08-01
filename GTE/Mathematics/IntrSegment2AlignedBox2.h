// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2026.07.31

#pragma once

// The queries consider the box to be a solid.
//
// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the four
// edges of the box.

#include <Mathematics/ContAlignedBox.h>
#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2AlignedBox2.h>
#include <Mathematics/Segment.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment2<T>, AlignedBox2<T>>
        :
        public TIQuery<T, Line2<T>, AlignedBox2<T>>
    {
    public:
        struct Result
            :
            public TIQuery<T, Line2<T>, AlignedBox2<T>>::Result
        {
            Result()
                :
                TIQuery<T, Line2<T>, AlignedBox2<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Segment2<T> const& segment, AlignedBox2<T> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<T>::Unit(d).
            Vector2<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the segment to a centered form in the aligned-box
            // coordinate system.
            Vector2<T> transformedP0 = segment.p[0] - boxCenter;
            Vector2<T> transformedP1 = segment.p[1] - boxCenter;
            Segment2<T> transformedSegment(transformedP0, transformedP1);
            Vector2<T> segOrigin{}, segDirection{};
            T segExtent{};
            transformedSegment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result{};
            DoQuery(segOrigin, segDirection, segExtent, boxExtent, result);
            return result;
        }

    protected:
        void DoQuery(Vector2<T> const& segOrigin,
            Vector2<T> const& segDirection, T segExtent,
            Vector2<T> const& boxExtent, Result& result)
        {
            for (int32_t i = 0; i < 2; ++i)
            {
                T lhs = std::fabs(segOrigin[i]);
                T rhs = boxExtent[i] + segExtent * std::fabs(segDirection[i]);
                if (lhs > rhs)
                {
                    result.intersect = false;
                    return;
                }
            }

            TIQuery<T, Line2<T>, AlignedBox2<T>>::DoQuery(segOrigin,
                segDirection, boxExtent, result);
        }
    };

    template <typename T>
    class FIQuery<T, Segment2<T>, AlignedBox2<T>>
        :
        public FIQuery<T, Line2<T>, AlignedBox2<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line2<T>, AlignedBox2<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line2<T>, AlignedBox2<T>>::Result{},
                cdeParameter{ (T)0, (T)0 }
            {
            }

            // The base class parameter[] values are t-values for the
            // segment parameterization (1-t)*p[0] + t*p[1], where t in [0,1].
            // The values in this class are s-values for the centered form
            // C + s * D, where s in [-e,e] and e is the extent of the
            // segment.
            std::array<T, 2> cdeParameter;
        };

        Result operator()(Segment2<T> const& segment, AlignedBox2<T> const& box)
        {
            // The default result indicates no intersection.
            Result result{};

            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<T>::Unit(d).
            Vector2<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the segment to a centered form in the aligned-box
            // coordinate system.
            Vector2<T> transformedP0 = segment.p[0] - boxCenter;
            Vector2<T> transformedP1 = segment.p[1] - boxCenter;
            Segment2<T> transformedSegment(transformedP0, transformedP1);
            Vector2<T> segOrigin{}, segDirection{};
            T segExtent{};
            transformedSegment.GetCenteredForm(segOrigin, segDirection, segExtent);

            if (segExtent > static_cast<T>(0))
            {
                DoQuery(segOrigin, segDirection, segExtent, boxExtent, result);
                for (int32_t i = 0; i < result.numIntersections; ++i)
                {
                    // Compute the segment in the aligned-box coordinate system
                    // and then translate it back to the original coordinates
                    // using the box cener.
                    result.point[i] = boxCenter + (segOrigin + result.parameter[i] * segDirection);
                    result.cdeParameter[i] = result.parameter[i];

                    // Convert the parameters from the centered form to the
                    // endpoint form.
                    result.parameter[i] = (result.parameter[i] / segExtent + (T)1) * (T)0.5;
                }
            }
            else
            {
                // The segment is degenerate, representing a single point.
                // Report an intersection when this point is contained by the
                // box.
                if (InContainer(segment.p[0], box))
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.parameter[0] = static_cast<T>(0);
                    result.parameter[1] = static_cast<T>(0);
                    result.cdeParameter = result.cdeParameter;
                    result.point[0] = segment.p[0];
                    result.point[1] = segment.p[1];
                }
            }

            return result;
        }

    protected:
        void DoQuery(Vector2<T> const& segOrigin,
            Vector2<T> const& segDirection, T segExtent,
            Vector2<T> const& boxExtent, Result& result)
        {
            FIQuery<T, Line2<T>, AlignedBox2<T>>::DoQuery(segOrigin,
                segDirection, boxExtent, result);

            if (result.intersect)
            {
                // The line containing the segment intersects the box; the
                // t-interval is [t0,t1].  The segment intersects the box as
                // long as [t0,t1] overlaps the segment t-interval
                // [-segExtent,+segExtent].
                std::array<T, 2> segInterval = { -segExtent, segExtent };
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                auto iiResult = iiQuery(result.parameter, segInterval);
                result.intersect = iiResult.intersect;
                result.numIntersections = iiResult.numIntersections;
                result.parameter = iiResult.overlap;

                // If a segment intersects a box at an endpoint, and if that
                // endpoint is the only point of intersection, ensure the
                // caller computes 2 points of intersection for a degenerate
                // line segment representing a single point.
                if (result.numIntersections == 1)
                {
                    result.numIntersections = 2;
                }
            }
        }
    };
}
