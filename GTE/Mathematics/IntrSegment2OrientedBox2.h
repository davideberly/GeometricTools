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

#include <Mathematics/ContOrientedBox2.h>
#include <Mathematics/IntrSegment2AlignedBox2.h>
#include <Mathematics/OrientedBox.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment2<T>, OrientedBox2<T>>
        :
        public TIQuery<T, Segment2<T>, AlignedBox2<T>>
    {
    public:
        struct Result
            :
            public TIQuery<T, Segment2<T>, AlignedBox2<T>>::Result
        {
            Result()
                :
                TIQuery<T, Segment2<T>, AlignedBox2<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Segment2<T> const& segment, OrientedBox2<T> const& box)
        {
            // Transform the segment to the oriented-box coordinate system.
            Vector2<T> tmpOrigin{}, tmpDirection{};
            T segExtent{};
            segment.GetCenteredForm(tmpOrigin, tmpDirection, segExtent);
            Vector2<T> diff = tmpOrigin - box.center;
            Vector2<T> segOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1])
            };
            Vector2<T> segDirection
            {
                Dot(tmpDirection, box.axis[0]),
                Dot(tmpDirection, box.axis[1])
            };

            Result result{};
            this->DoQuery(segOrigin, segDirection, segExtent, box.extent, result);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Segment2<T>, OrientedBox2<T>>
        :
        public FIQuery<T, Segment2<T>, AlignedBox2<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Segment2<T>, AlignedBox2<T>>::Result
        {
            Result()
                :
                FIQuery<T, Segment2<T>, AlignedBox2<T>>::Result{},
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

        Result operator()(Segment2<T> const& segment, OrientedBox2<T> const& box)
        {
            // The default result indicates no intersection.
            Result result{};

            // Transform the segment to the oriented-box coordinate system.
            Vector2<T> tmpOrigin{}, tmpDirection{};
            T segExtent{};
            segment.GetCenteredForm(tmpOrigin, tmpDirection, segExtent);
            Vector2<T> diff = tmpOrigin - box.center;
            Vector2<T> segOrigin
            {
                Dot(diff, box.axis[0]),
                Dot(diff, box.axis[1])
            };
            Vector2<T> segDirection
            {
                Dot(tmpDirection, box.axis[0]),
                Dot(tmpDirection, box.axis[1])
            };

            if (segExtent > static_cast<T>(0))
            {
                this->DoQuery(segOrigin, segDirection, segExtent, box.extent, result);
                for (int32_t i = 0; i < result.numIntersections; ++i)
                {
                    // Compute the segment in the aligned-box coordinate system
                    // and then translate it back to the original coordinates
                    // using the box cener.
                    result.point[i] = box.center + (segOrigin + result.parameter[i] * segDirection);
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
                    result.numIntersections = 2;
                    result.parameter[0] = static_cast<T>(0);
                    result.parameter[1] = static_cast<T>(0);
                    result.cdeParameter = result.cdeParameter;
                    result.point[0] = segment.p[0];
                    result.point[1] = segment.p[1];
                }
            }

            return result;
        }
    };
}
