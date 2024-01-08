// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The test-intersection queries use the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// The find-intersection queries use parametric clipping against the six
// faces of the box. The find-intersection queries use Liang-Barsky
// clipping. The queries consider the box to be a solid. The algorithms
// are described in
// https://www.geometrictools.com/Documentation/IntersectionLineBox.pdf

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine3AlignedBox3.h>
#include <Mathematics/Ray.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray3<T>, AlignedBox3<T>>
        :
        public TIQuery<T, Line3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
            :
            public TIQuery<T, Line3<T>, AlignedBox3<T>>::Result
        {
            Result()
                :
                TIQuery<T, Line3<T>, AlignedBox3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray3<T> const& ray, AlignedBox3<T> const& box)
        {
            // Get the centered form of the aligned box. The axes are
            // implicitly axis[d] = Vector3<T>::Unit(d).
            Vector3<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the ray to the aligned-box coordinate system.
            Vector3<T> rayOrigin = ray.origin - boxCenter;

            Result result{};
            DoQuery(rayOrigin, ray.direction, boxExtent, result);
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector3<T> const& rayOrigin, Vector3<T> const& rayDirection,
            Vector3<T> const& boxExtent, Result& result)
        {
            T const zero = static_cast<T>(0);
            for (int32_t i = 0; i < 3; ++i)
            {
                if (std::fabs(rayOrigin[i]) > boxExtent[i] &&
                    rayOrigin[i] * rayDirection[i] >= zero)
                {
                    result.intersect = false;
                    return;
                }
            }

            TIQuery<T, Line3<T>, AlignedBox3<T>>::DoQuery(
                rayOrigin, rayDirection, boxExtent, result);
        }
    };

    template <typename T>
    class FIQuery<T, Ray3<T>, AlignedBox3<T>>
        :
        public FIQuery<T, Line3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line3<T>, AlignedBox3<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line3<T>, AlignedBox3<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Ray3<T> const& ray, AlignedBox3<T> const& box)
        {
            // Get the centered form of the aligned box. The axes are
            // implicitly axis[d] = Vector3<T>::Unit(d).
            Vector3<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the ray to the aligned-box coordinate system.
            Vector3<T> rayOrigin = ray.origin - boxCenter;

            Result result{};
            DoQuery(rayOrigin, ray.direction, boxExtent, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = ray.origin + result.parameter[i] * ray.direction;
                }
            }
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector3<T> const& rayOrigin, Vector3<T> const& rayDirection,
            Vector3<T> const& boxExtent, Result& result)
        {
            FIQuery<T, Line3<T>, AlignedBox3<T>>::DoQuery(
                rayOrigin, rayDirection, boxExtent, result);

            if (result.intersect)
            {
                // The line containing the ray intersects the box; the
                // t-interval is [t0,t1]. The ray intersects the box as long
                // as [t0,t1] overlaps the ray t-interval (0,+infinity).
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery;
                auto iiResult = iiQuery(result.parameter, static_cast<T>(0), true);
                if (iiResult.intersect)
                {
                    result.numIntersections = iiResult.numIntersections;
                    result.parameter = iiResult.overlap;
                }
                else
                {
                    // The line containing the ray does not intersect the box.
                    result = Result{};
                }
            }
        }
    };
}
