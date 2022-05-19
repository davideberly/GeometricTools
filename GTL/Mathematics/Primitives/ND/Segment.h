// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The segment is represented by (1-t)*P0 + t*P1, where P0 and P1 are the
// endpoints of the segment and 0 <= t <= 1. Some algorithms prefer a
// centered representation that is similar to how oriented bounding boxes are
// defined. This representation is C + s*D, where C = (P0 + P1)/2 is the
// center of the segment, D = (P1 - P0)/|P1 - P0| is a unit-length direction
// vector for the segment, and |t| <= e. The value e = |P1 - P0|/2 is the
// extent (or radius or half-length) of the segment.

// TODO: Remove the center-direction-extent manipulation of segments. Use
// only the 2-point formulation. This will propagate to any code using
// Segment<T,N>.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class Segment
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Segment()
            :
            p{}
        {
        }

        Segment(Vector<T, N> const& p0, Vector<T, N> const& p1)
            :
            p{ p0, p1 }
        {
        }

        Segment(std::array<Vector<T, N>, 2> const& inP)
            :
            p(inP)
        {
        }

        Segment(Vector<T, N> const& center, Vector<T, N> const& direction, T const& extent)
        {
            SetCenteredForm(center, direction, extent);
        }

        // Manipulation via the centered form. If you set p0 and p1; compute
        // C, D and e and then recompute q0 = C-e*D and q1 = C+e*D, numerical
        // round-off errors can lead to q0 not exactly equal to p0 and q1 not
        // exactly equal to p1.
        void SetCenteredForm(Vector<T, N> const& center,
            Vector<T, N> const& direction, T const& extent)
        {
            p[0] = center - extent * direction;
            p[1] = center + extent * direction;
        }

        void GetCenteredForm(Vector<T, N>& center,
            Vector<T, N>& direction, T& extent) const
        {
            center = C_<T>(1, 2) * (p[0] + p[1]);
            direction = p[1] - p[0];
            extent = C_<T>(1, 2) * Normalize(direction);
        }

        std::array<Vector<T, N>, 2> p;

    private:
        friend class UnitTestSegment;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(Segment<T, N> const& segment0, Segment<T, N> const& segment1)
    {
        return segment0.p == segment1.p;
    }

    template <typename T, size_t N>
    bool operator!=(Segment<T, N> const& segment0, Segment<T, N> const& segment1)
    {
        return !operator==(segment0, segment1);
    }

    template <typename T, size_t N>
    bool operator<(Segment<T, N> const& segment0, Segment<T, N> const& segment1)
    {
        return segment0.p < segment1.p;
    }

    template <typename T, size_t N>
    bool operator<=(Segment<T, N> const& segment0, Segment<T, N> const& segment1)
    {
        return !operator<(segment1, segment0);
    }

    template <typename T, size_t N>
    bool operator>(Segment<T, N> const& segment0, Segment<T, N> const& segment1)
    {
        return operator<(segment1, segment0);
    }

    template <typename T, size_t N>
    bool operator>=(Segment<T, N> const& segment0, Segment<T, N> const& segment1)
    {
        return !operator<(segment0, segment1);
    }

    // Template aliases for convenience.
    template <typename T> using Segment2 = Segment<T, 2>;
    template <typename T> using Segment3 = Segment<T, 3>;
}
