// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The circle containing the arc is represented as |X-C| = R where C is the
// center and R is the radius. The arc is defined by two points end0 and
// end1 on the circle so that end1 is obtained from end0 by traversing
// counterclockwise. The application is responsible for ensuring that end0
// and end1 are on the circle and that they are properly ordered.

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

        // Test whether P is on the arc. The application must ensure that P
        // is on the circle; that is, |P-C| = R. A point-on-circle test is
        // not used here because floating-point rounding errors will usually
        // prevent exact equality of |P-C| and R. This test works for any
        // angle between end1-C and end0-C, not just those between 0 and pi
        // radians.
        bool Contains(Vector2<T> const& p) const
        {
            // Assert: |P-C| = R where P is the input point, C is the circle
            // center and R is the circle radius. For P to be on the arc from
            // A to B, it must be on the side of the plane containing A with
            // normal N = Perp(B-A) where Perp(u,v) = (v,-u).
            Vector2<T> PmC = p - center;
            Vector2<T> diffPE0 = p - end[0];
            Vector2<T> diffE1E0 = end[1] - end[0];
            T dotPerp = DotPerp(diffPE0, diffE1E0);
            return dotPerp >= C_<T>(0);
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
