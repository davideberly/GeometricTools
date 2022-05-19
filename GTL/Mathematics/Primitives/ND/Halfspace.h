// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The halfspace is represented as Dot(M,X) >= c where M is a nonzero
// normal vector, c is the plane constant and X is any point in space.
// Usually M is chosen to be unit length, but for exact rational
// arithmetic it can be chosen not to be unit length.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class Halfspace
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Halfspace()
            :
            normal{},
            constant(C_<T>(0))
        {
        }

        // Specify M and c directly.
        Halfspace(Vector<T, N> const& inNormal, T const& inConstant)
            :
            normal(inNormal),
            constant(inConstant)
        {
        }

        Vector<T, N> normal;
        T constant;

    private:
        friend class UnitTestHalfspace;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(Halfspace<T, N> const& halfspace0, Halfspace<T, N> const& halfspace1)
    {
        return halfspace0.normal == halfspace1.normal
            && halfspace0.constant == halfspace1.constant;
    }

    template <typename T, size_t N>
    bool operator!=(Halfspace<T, N> const& halfspace0, Halfspace<T, N> const& halfspace1)
    {
        return !operator==(halfspace0, halfspace1);
    }

    template <typename T, size_t N>
    bool operator<(Halfspace<T, N> const& halfspace0, Halfspace<T, N> const& halfspace1)
    {
        if (halfspace0.normal < halfspace1.normal)
        {
            return true;
        }

        if (halfspace0.normal > halfspace1.normal)
        {
            return false;
        }

        return halfspace0.constant < halfspace1.constant;
    }

    template <typename T, size_t N>
    bool operator<=(Halfspace<T, N> const& halfspace0, Halfspace<T, N> const& halfspace1)
    {
        return !operator<(halfspace1, halfspace0);
    }

    template <typename T, size_t N>
    bool operator>(Halfspace<T, N> const& halfspace0, Halfspace<T, N> const& halfspace1)
    {
        return operator<(halfspace1, halfspace0);
    }

    template <typename T, size_t N>
    bool operator>=(Halfspace<T, N> const& halfspace0, Halfspace<T, N> const& halfspace1)
    {
        return !operator<(halfspace0, halfspace1);
    }

    // Template aliases for convenience.
    template <typename T> using Halfspace2 = Halfspace<T, 2>;
    template <typename T> using Halfspace3 = Halfspace<T, 3>;
}
