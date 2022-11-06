// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.11.06

#pragma once

// The circle containing the arc is represented as |X-C| = r where C is the
// center and r is the radius. The arc is defined by two points E0 and E1 on
// the circle so that E1 is obtained from E0 by traversing counterclockwise.
// The application is responsible for ensuring that E0 and E1 are on the
// circle and that they are properly ordered.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cstddef>

namespace gtl
{
    template <typename T>
    class Arc2
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Arc2()
            :
            center{},
            radius(C_<T>(0)),
            end{}
        {
        }

        Arc2(Vector2<T> const& inCenter, T const& inRadius,
            std::array<Vector2<T>, 2> const& inEnd)
            :
            center(inCenter),
            radius(inRadius),
            end(inEnd)
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
            // If epsilon is negative, function returns false. Please ensure
            // epsilon is nonnegative.

            T length = Length(P - center);
            if (std::fabs(length - radius) <= epsilon)
            {
                Vector2<T> diffPE0 = P - end[0];
                Vector2<T> diffE1E0 = end[1] - end[0];
                T dotPerp = DotPerp(diffPE0, diffE1E0);
                return dotPerp >= C_<T>(0);
            }
            else
            {
                return false;
            }
        }

        Vector2<T> center;
        T radius;
        std::array<Vector2<T>, 2> end;

    private:
        friend class UnitTestArc2;
    };

    // Comparisons to support sorted containers.
    template <typename T>
    bool operator==(Arc2<T> const& arc0, Arc2<T> const& arc1)
    {
        return arc0.center == arc1.center
            && arc0.radius == arc1.radius
            && arc0.end == arc1.end;
    }

    template <typename T>
    bool operator!=(Arc2<T> const& arc0, Arc2<T> const& arc1)
    {
        return !operator==(arc0, arc1);
    }

    template <typename T>
    bool operator<(Arc2<T> const& arc0, Arc2<T> const& arc1)
    {
        if (arc0.center < arc1.center)
        {
            return true;
        }

        if (arc0.center > arc1.center)
        {
            return false;
        }

        if (arc0.radius < arc1.radius)
        {
            return true;
        }

        if (arc0.radius > arc1.radius)
        {
            return false;
        }

        return arc0.end < arc1.end;
    }

    template <typename T>
    bool operator<=(Arc2<T> const& arc0, Arc2<T> const& arc1)
    {
        return !operator<(arc1, arc0);
    }

    template <typename T>
    bool operator>(Arc2<T> const& arc0, Arc2<T> const& arc1)
    {
        return operator<(arc1, arc0);
    }

    template <typename T>
    bool operator>=(Arc2<T> const& arc0, Arc2<T> const& arc1)
    {
        return !operator<(arc0, arc1);
    }
}
