// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// Points are R(s0,s1) = C + s0*A0 + s1*A1, where C is the center of the
// rectangle and A0 and A1 are nonzero and perpendicular axes. The parameters
// s0 and s1 are constrained by |s0| <= e0 and |s1| <= e1, where e0 > 0 and
// e1 > 0 are the extents of the rectangle. Usually A0 and A1 are chosen to
// be unit length, but for exact rational arithmetic they can be chosen not to
// be unit length.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class Rectangle
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Rectangle()
            :
            center{},
            axis{},
            extent{}
        {
        }

        Rectangle(Vector<T, N> const& inCenter,
            std::array<Vector<T, N>, 2> const& inAxis,
            Vector2<T> const& inExtent)
            :
            center(inCenter),
            axis(inAxis),
            extent(inExtent)
        {
        }

        // Compute the vertices of the rectangle. If index i has the bit
        // pattern i = b[1]b[0], then
        //   vertex[i] = center + sum_{d=0}^{1} sign[d] * extent[d] * axis[d]
        // where sign[d] = 2*b[d] - 1.
        void GetVertices(std::array<Vector<T, N>, 4>& vertex) const
        {
            Vector<T, N> product0 = extent[0] * axis[0];
            Vector<T, N> product1 = extent[1] * axis[1];
            Vector<T, N> sum = product0 + product1;
            Vector<T, N> dif = product0 - product1;

            vertex[0] = center - sum;
            vertex[1] = center + dif;
            vertex[2] = center - dif;
            vertex[3] = center + sum;
        }

        Vector<T, N> center;
        std::array<Vector<T, N>, 2> axis;
        Vector2<T> extent;

    private:
        friend class UnitTestRectangle;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(Rectangle<T, N> const& rectangle0, Rectangle<T, N> const& rectangle1)
    {
        return rectangle0.center == rectangle1.center
            && rectangle0.axis == rectangle1.axis
            && rectangle0.extent == rectangle1.extent;
    }

    template <typename T, size_t N>
    bool operator!=(Rectangle<T, N> const& rectangle0, Rectangle<T, N> const& rectangle1)
    {
        return !operator==(rectangle0, rectangle1);
    }

    template <typename T, size_t N>
    bool operator<(Rectangle<T, N> const& rectangle0, Rectangle<T, N> const& rectangle1)
    {
        if (rectangle0.center < rectangle1.center)
        {
            return true;
        }

        if (rectangle0.center > rectangle1.center)
        {
            return false;
        }

        if (rectangle0.axis < rectangle1.axis)
        {
            return true;
        }

        if (rectangle0.axis > rectangle1.axis)
        {
            return false;
        }

        return rectangle0.extent < rectangle1.extent;
    }

    template <typename T, size_t N>
    bool operator<=(Rectangle<T, N> const& rectangle0, Rectangle<T, N> const& rectangle1)
    {
        return !operator<(rectangle1, rectangle0);
    }

    template <typename T, size_t N>
    bool operator>(Rectangle<T, N> const& rectangle0, Rectangle<T, N> const& rectangle1)
    {
        return operator<(rectangle1, rectangle0);
    }

    template <typename T, size_t N>
    bool operator>=(Rectangle<T, N> const& rectangle0, Rectangle<T, N> const& rectangle1)
    {
        return !operator<(rectangle0, rectangle1);
    }

    // Template aliases for convenience.
    template <typename T> using Rectangle2 = Rectangle<T, 2>;
    template <typename T> using Rectangle3 = Rectangle<T, 3>;
}
