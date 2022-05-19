// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The triangle is represented as an array of three vertices.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class Triangle
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Triangle()
            :
            v{}
        {
        }

        Triangle(Vector<T, N> const& v0, Vector<T, N> const& v1, Vector<T, N> const& v2)
            :
            v{ v0, v1, v2 }
        {
        }


        Triangle(std::array<Vector<T, N>, 3> const& inV)
            :
            v(inV)
        {
        }

        std::array<Vector<T, N>, 3> v;

    private:
        friend class UnitTestTriangle;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(Triangle<T, N> const& triangle0, Triangle<T, N> const& triangle1)
    {
        return triangle0.v == triangle1.v;
    }

    template <typename T, size_t N>
    bool operator!=(Triangle<T, N> const& triangle0, Triangle<T, N> const& triangle1)
    {
        return !operator==(triangle0, triangle1);
    }

    template <typename T, size_t N>
    bool operator<(Triangle<T, N> const& triangle0, Triangle<T, N> const& triangle1)
    {
        return triangle0.v < triangle1.v;
    }

    template <typename T, size_t N>
    bool operator<=(Triangle<T, N> const& triangle0, Triangle<T, N> const& triangle1)
    {
        return !operator<(triangle1, triangle0);
    }

    template <typename T, size_t N>
    bool operator>(Triangle<T, N> const& triangle0, Triangle<T, N> const& triangle1)
    {
        return operator<(triangle1, triangle0);
    }

    template <typename T, size_t N>
    bool operator>=(Triangle<T, N> const& triangle0, Triangle<T, N> const& triangle1)
    {
        return !operator<(triangle0, triangle1);
    }

    // Template aliases for convenience.
    template <typename T> using Triangle2 = Triangle<T, 2>;
    template <typename T> using Triangle3 = Triangle<T, 3>;
}
