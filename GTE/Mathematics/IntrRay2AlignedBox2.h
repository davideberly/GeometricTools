// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the box to be a solid.
//
// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the four
// edges of the box.

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2AlignedBox2.h>
#include <Mathematics/Ray.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray2<T>, AlignedBox2<T>>
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

        Result operator()(Ray2<T> const& ray, AlignedBox2<T> const& box)
        {
            // Get the centered form of the aligned box. The axes are
            // implicitly Axis[d] = Vector2<T>::Unit(d).
            Vector2<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the ray to the aligned-box coordinate system.
            Vector2<T> rayOrigin = ray.origin - boxCenter;

            Result result{};
            DoQuery(rayOrigin, ray.direction, boxExtent, result);
            return result;
        }

    protected:
        void DoQuery(Vector2<T> const& rayOrigin,
            Vector2<T> const& rayDirection, Vector2<T> const& boxExtent,
            Result& result)
        {
            for (int32_t i = 0; i < 2; ++i)
            {
                if (std::fabs(rayOrigin[i]) > boxExtent[i] &&
                    rayOrigin[i] * rayDirection[i] >= (T)0)
                {
                    result.intersect = false;
                    return;
                }
            }

            TIQuery<T, Line2<T>, AlignedBox2<T>>::DoQuery(rayOrigin,
                rayDirection, boxExtent, result);
        }
    };

    template <typename T>
    class FIQuery<T, Ray2<T>, AlignedBox2<T>>
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
                FIQuery<T, Line2<T>, AlignedBox2<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray2<T> const& ray, AlignedBox2<T> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<T>::Unit(d).
            Vector2<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the ray to the aligned-box coordinate system.
            Vector2<T> rayOrigin = ray.origin - boxCenter;

            Result result{};
            DoQuery(rayOrigin, ray.direction, boxExtent, result);
            for (int32_t i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = ray.origin + result.parameter[i] * ray.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector2<T> const& rayOrigin,
            Vector2<T> const& rayDirection, Vector2<T> const& boxExtent,
            Result& result)
        {
            FIQuery<T, Line2<T>, AlignedBox2<T>>::DoQuery(rayOrigin,
                rayDirection, boxExtent, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the box; the
                // t-interval is [t0,t1].  The ray intersects the box as long
                // as [t0,t1] overlaps the ray t-interval [0,+infinity).
                std::array<T, 2> rayInterval =
                    { (T)0, std::numeric_limits<T>::max() };
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                auto iiResult = iiQuery(result.parameter, rayInterval);
                result.intersect = iiResult.intersect;
                result.numIntersections = iiResult.numIntersections;
                result.parameter = iiResult.overlap;
            }
        }
    };
}
