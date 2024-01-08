// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.02.28

#pragma once

// The circle containing the arc is represented as |X-C| = r where C is the
// center and r is the radius. The arc is defined by two points E0 and E1 on
// the circle so that E1 is obtained from E0 by traversing counterclockwise.
// The application is responsible for ensuring that E0 and E1 are on the
// circle and that they are properly ordered.

#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>

namespace gte
{
    template <typename T>
    class Arc2
    {
    public:
        // Construction and destruction. The default constructor sets the
        // center to (0,0), radius to 1, end0 to (1,0), and end1 to (0,1).
        Arc2()
            :
            center(Vector2<T>::Zero()),
            radius(static_cast<T>(1)),
            end{ Vector2<T>::Zero(), Vector2<T>::Zero() }
        {
        }

        Arc2(Vector2<T> const& C, T r, Vector2<T>const& E0, Vector2<T>const& E1)
            :
            center(C),
            radius(r),
            end{ E0, E1 }
        {
        }

        // Test whether P is on the arc.
        // 
        // Formulated for real arithmetic, |P-C| - r = 0 is necessary for P to
        // be on the circle of the arc. If P is on the circle, then P is on
        // the arc from E0 to E1 when it is on the side of the line containing
        // E0 with normal Perp(E1-E0) where Perp(u,v) = (v,-u). This test
        // works for any angle between E0-C and E1-C, even if the angle is
        // larger or equal to pi radians.
        //
        // Formulated for floating-point or rational types, rounding errors
        // cause |P-C| - r rarely to be 0 when P is on (or numerically near)
        // the circle. To allow for this, choose a small and nonnegative
        // tolerance epsilon. The test concludes that P is on the circle when
        // ||P-C| - r| <= epsilon;otherwise, P is not on the circle. If P is
        // on the circle (in the epsilon-tolerance sense), the side-of-line
        // test of the previous/ paragraph is applied.
        bool Contains(Vector2<T> const& P, T const& epsilon) const
        {
            // If epsilon is negative, the tolerance behaves as if a value of
            // zero was passed for epsilon.

            T length = Length(P - center);
            if (std::fabs(length - radius) <= epsilon)
            {
                Vector2<T> diffPE0 = P - end[0];
                Vector2<T> diffE1E0 = end[1] - end[0];
                T dotPerp = DotPerp(diffPE0, diffE1E0);
                return dotPerp >= static_cast<T>(0);
            }
            else
            {
                return false;
            }
        }

        // This function assumes P is on the circle containing the arc (with
        // possibly a small amount of floating-point rounding error).
        bool Contains(Vector2<T> const& P) const
        {
            Vector2<T> diffPE0 = P - end[0];
            Vector2<T> diffE1E0 = end[1] - end[0];
            T dotPerp = DotPerp(diffPE0, diffE1E0);
            return dotPerp >= static_cast<T>(0);
        }

        Vector2<T> center;
        T radius;
        std::array<Vector2<T>, 2> end;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Arc2 const& arc) const
        {
            return center == arc.center && radius == arc.radius
                && end[0] == arc.end[0] && end[1] == arc.end[1];
        }

        bool operator!=(Arc2 const& arc) const
        {
            return !operator==(arc);
        }

        bool operator< (Arc2 const& arc) const
        {
            if (center < arc.center)
            {
                return true;
            }

            if (center > arc.center)
            {
                return false;
            }

            if (radius < arc.radius)
            {
                return true;
            }

            if (radius > arc.radius)
            {
                return false;
            }

            if (end[0] < arc.end[0])
            {
                return true;
            }

            if (end[0] > arc.end[0])
            {
                return false;
            }

            return end[1] < arc.end[1];
        }

        bool operator<=(Arc2 const& arc) const
        {
            return !arc.operator<(*this);
        }

        bool operator> (Arc2 const& arc) const
        {
            return arc.operator<(*this);
        }

        bool operator>=(Arc2 const& arc) const
        {
            return !operator<(arc);
        }
    };
}
