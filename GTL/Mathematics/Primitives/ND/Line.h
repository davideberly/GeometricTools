// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The line is represented by P+t*D, where P is an origin point, D is a
// nonzero direction vector and t is any real number. Usually D is chosen to
// be unit length, but for exact rational arithmetic it can be chosen not to
// be unit length.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class Line
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Line()
            :
            origin{},
            direction{}
        {
        }

        Line(Vector<T, N> const& inOrigin, Vector<T, N> const& inDirection)
            :
            origin(inOrigin),
            direction(inDirection)
        {
        }

        Vector<T, N> origin, direction;

    private:
        friend class UnitTestLine;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(Line<T, N> const& line0, Line<T, N> const& line1)
    {
        return line0.origin == line1.origin
            && line0.direction == line1.direction;
    }

    template <typename T, size_t N>
    bool operator!=(Line<T, N> const& line0, Line<T, N> const& line1)
    {
        return !operator==(line0, line1);
    }

    template <typename T, size_t N>
    bool operator<(Line<T, N> const& line0, Line<T, N> const& line1)
    {
        if (line0.origin < line1.origin)
        {
            return true;
        }

        if (line0.origin > line1.origin)
        {
            return false;
        }

        return line0.direction < line1.direction;
    }

    template <typename T, size_t N>
    bool operator<=(Line<T, N> const& line0, Line<T, N> const& line1)
    {
        return !operator<(line1, line0);
    }

    template <typename T, size_t N>
    bool operator>(Line<T, N> const& line0, Line<T, N> const& line1)
    {
        return operator<(line1, line0);
    }

    template <typename T, size_t N>
    bool operator>=(Line<T, N> const& line0, Line<T, N> const& line1)
    {
        return !operator<(line0, line1);
    }

    // Template aliases for convenience.
    template <typename T> using Line2 = Line<T, 2>;
    template <typename T> using Line3 = Line<T, 3>;
}
