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

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/Line.h>
#include <Mathematics/Vector3.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line3<T>, AlignedBox3<T>>
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

        Result operator()(Line3<T> const& line, AlignedBox3<T> const& box)
        {
            // Get the centered form of the aligned box. The axes are
            // implicitly axis[d] = Vector3<T>::Unit(d).
            Vector3<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the line to the aligned-box coordinate system.
            Vector3<T> lineOrigin = line.origin - boxCenter;

            Result result{};
            DoQuery(lineOrigin, line.direction, boxExtent, result);
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector3<T> const& lineOrigin, Vector3<T> const& lineDirection,
            Vector3<T> const& boxExtent, Result& result)
        {
            Vector3<T> WxD = Cross(lineDirection, lineOrigin);
            std::array<T, 3> absWdU
            {
                std::fabs(lineDirection[0]),
                std::fabs(lineDirection[1]),
                std::fabs(lineDirection[2])
            };

            if (std::fabs(WxD[0]) > boxExtent[1] * absWdU[2] + boxExtent[2] * absWdU[1])
            {
                return;
            }

            if (std::fabs(WxD[1]) > boxExtent[0] * absWdU[2] + boxExtent[2] * absWdU[0])
            {
                return;
            }

            if (std::fabs(WxD[2]) > boxExtent[0] * absWdU[1] + boxExtent[1] * absWdU[0])
            {
                return;
            }

            result.intersect = true;
        }
    };

    template <typename T>
    class FIQuery<T, Line3<T>, AlignedBox3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                parameter{ static_cast<T>(0), static_cast<T>(0) },
                point{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            bool intersect;
            size_t numIntersections;
            std::array<T, 2> parameter;
            std::array<Vector3<T>, 2> point;
        };

        Result operator()(Line3<T> const& line, AlignedBox3<T> const& box)
        {
            // Get the centered form of the aligned box. The axes are
            // implicitly axis[d] = Vector3<T>::Unit(d).
            Vector3<T> boxCenter{}, boxExtent{};
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the line to the aligned-box coordinate system.
            Vector3<T> lineOrigin = line.origin - boxCenter;

            Result result{};
            DoQuery(lineOrigin, line.direction, boxExtent, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = line.origin + result.parameter[i] * line.direction;
                }
            }
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector3<T> const& lineOrigin, Vector3<T> const& lineDirection,
            Vector3<T> const& boxExtent, Result& result)
        {
            // The line t-values are in the interval (-infinity,+infinity).
            // Clip the line against all six planes of an aligned box in
            // centered form. The result.numIntersections is
            //   0, no intersection
            //   1, intersect in a single point (t0 is line parameter of point)
            //   2, intersect in a segment (line parameter interval is [t0,t1])
            T t0 = -std::numeric_limits<T>::max();
            T t1 = std::numeric_limits<T>::max();
            if (Clip(+lineDirection[0], -lineOrigin[0] - boxExtent[0], t0, t1) &&
                Clip(-lineDirection[0], +lineOrigin[0] - boxExtent[0], t0, t1) &&
                Clip(+lineDirection[1], -lineOrigin[1] - boxExtent[1], t0, t1) &&
                Clip(-lineDirection[1], +lineOrigin[1] - boxExtent[1], t0, t1) &&
                Clip(+lineDirection[2], -lineOrigin[2] - boxExtent[2], t0, t1) &&
                Clip(-lineDirection[2], +lineOrigin[2] - boxExtent[2], t0, t1))
            {
                result.intersect = true;
                if (t1 > t0)
                {
                    result.numIntersections = 2;
                    result.parameter[0] = t0;
                    result.parameter[1] = t1;
                }
                else
                {
                    result.numIntersections = 1;
                    result.parameter[0] = t0;
                    result.parameter[1] = t0;
                }
            }
        }

    private:
        // Test whether the current clipped segment intersects the current
        // test plane.  If the return value is 'true', the segment does
        // intersect the plane and is clipped; otherwise, the segment is
        // culled (no intersection with box).
        static bool Clip(T const& denom, T const& numer, T& t0, T& t1)
        {
            T const zero = static_cast<T>(0);
            if (denom > zero)
            {
                if (numer > denom * t1)
                {
                    return false;
                }
                if (numer > denom * t0)
                {
                    t0 = numer / denom;
                }
                return true;
            }
            else if (denom < zero)
            {
                if (numer > denom * t0)
                {
                    return false;
                }
                if (numer > denom * t1)
                {
                    t1 = numer / denom;
                }
                return true;
            }
            else
            {
                return numer <= zero;
            }
        }
    };
}
