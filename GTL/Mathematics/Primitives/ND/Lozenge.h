// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// A lozenge is the set of points that are equidistant from a rectangle, the
// common distance called the radius.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <GTL/Mathematics/Primitives/ND/Rectangle.h>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class Lozenge
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all member to zero.
        Lozenge()
            :
            rectangle{},
            radius(C_<T>(0))
        {
        }

        Lozenge(Rectangle<T, N> const& inRectangle, T const& inRadius)
            :
            rectangle(inRectangle),
            radius(inRadius)
        {
        }

        Rectangle<T, N> rectangle;
        T radius;

    private:
        friend class UnitTestLozenge;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(Lozenge<T, N> const& ssr0, Lozenge<T, N> const& ssr1)
    {
        return ssr0.rectangle == ssr1.rectangle
            && ssr0.radius == ssr1.radius;
    }

    template <typename T, size_t N>
    bool operator!=(Lozenge<T, N> const& ssr0, Lozenge<T, N> const& ssr1)
    {
        return !operator==(ssr0, ssr1);
    }

    template <typename T, size_t N>
    bool operator<(Lozenge<T, N> const& ssr0, Lozenge<T, N> const& ssr1)
    {
        if (ssr0.rectangle < ssr1.rectangle)
        {
            return true;
        }

        if (ssr0.rectangle > ssr1.rectangle)
        {
            return false;
        }

        return ssr0.radius < ssr1.radius;
    }

    template <typename T, size_t N>
    bool operator<=(Lozenge<T, N> const& ssr0, Lozenge<T, N> const& ssr1)
    {
        return !operator<(ssr1, ssr0);
    }

    template <typename T, size_t N>
    bool operator>(Lozenge<T, N> const& ssr0, Lozenge<T, N> const& ssr1)
    {
        return operator<(ssr1, ssr0);
    }

    template <typename T, size_t N>
    bool operator>=(Lozenge<T, N> const& ssr0, Lozenge<T, N> const& ssr1)
    {
        return !operator<(ssr0, ssr1);
    }

    // Template alias for convenience.
    template <typename T> using Lozenge3 = Lozenge<T, 3>;
}
