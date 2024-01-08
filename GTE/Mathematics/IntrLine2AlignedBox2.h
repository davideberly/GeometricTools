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

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line2<T>, AlignedBox2<T>>
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

        Result operator()(Line2<T> const& line, AlignedBox2<T> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<T>::Unit(d).
            Vector2<T> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the line to the aligned-box coordinate system.
            Vector2<T> lineOrigin = line.origin - boxCenter;

            Result result{};
            DoQuery(lineOrigin, line.direction, boxExtent, result);
            return result;
        }

    protected:
        void DoQuery(Vector2<T> const& lineOrigin,
            Vector2<T> const& lineDirection, Vector2<T> const& boxExtent,
            Result& result)
        {
            T LHS = std::fabs(DotPerp(lineDirection, lineOrigin));
            T RHS =
                boxExtent[0] * std::fabs(lineDirection[1]) +
                boxExtent[1] * std::fabs(lineDirection[0]);
            result.intersect = (LHS <= RHS);
        }
    };

    template <typename T>
    class FIQuery<T, Line2<T>, AlignedBox2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                parameter{ (T)0, (T)0 },
                point{ Vector2<T>::Zero(), Vector2<T>::Zero() }
            {
            }

            bool intersect;
            int32_t numIntersections;
            std::array<T, 2> parameter;
            std::array<Vector2<T>, 2> point;
        };

        Result operator()(Line2<T> const& line, AlignedBox2<T> const& box)
        {
            // Get the centered form of the aligned box.  The axes are
            // implicitly Axis[d] = Vector2<T>::Unit(d).
            Vector2<T> boxCenter, boxExtent;
            box.GetCenteredForm(boxCenter, boxExtent);

            // Transform the line to the aligned-box coordinate system.
            Vector2<T> lineOrigin = line.origin - boxCenter;

            Result result{};
            DoQuery(lineOrigin, line.direction, boxExtent, result);
            for (int32_t i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = line.origin + result.parameter[i] * line.direction;
            }
            return result;
        }

    protected:
        void DoQuery(Vector2<T> const& lineOrigin,
            Vector2<T> const& lineDirection, Vector2<T> const& boxExtent,
            Result& result)
        {
            // The line t-values are in the interval (-infinity,+infinity).
            // Clip the line against all four planes of an aligned box in
            // centered form.  The result.numPoints is
            //  0, no intersection
            //  1, intersect in a single point (t0 is line parameter of point)
            //  2, intersect in a segment (line parameter interval is [t0,t1])
            T t0 = -std::numeric_limits<T>::max();
            T t1 = std::numeric_limits<T>::max();
            if (Clip(+lineDirection[0], -lineOrigin[0] - boxExtent[0], t0, t1) &&
                Clip(-lineDirection[0], +lineOrigin[0] - boxExtent[0], t0, t1) &&
                Clip(+lineDirection[1], -lineOrigin[1] - boxExtent[1], t0, t1) &&
                Clip(-lineDirection[1], +lineOrigin[1] - boxExtent[1], t0, t1))
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
                    result.parameter[1] = t0;  // Used by derived classes.
                }
                return;
            }

            result.intersect = false;
            result.numIntersections = 0;
        }

    private:
        // Test whether the current clipped segment intersects the current
        // test plane.  If the return value is 'true', the segment does
        // intersect the plane and is clipped; otherwise, the segment is
        // culled (no intersection with box).
        static bool Clip(T denom, T numer, T& t0, T& t1)
        {
            if (denom > (T)0)
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
            else if (denom < (T)0)
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
                return numer <= (T)0;
            }
        }
    };
}
