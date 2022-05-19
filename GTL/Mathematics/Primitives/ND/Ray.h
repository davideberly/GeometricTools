// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The ray is represented as P+t*D, where P is the ray origin, D is a nonzero
// nonzero direction vector and t >= 0. Usually D is chosen to be unit length,
// but for exact rational arithmetic it can be chosen not to be unit length.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class Ray
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Ray()
            :
            origin{},
            direction{}
        {
        }

        Ray(Vector<T, N> const& inOrigin, Vector<T, N> const& inDirection)
            :
            origin(inOrigin),
            direction(inDirection)
        {
        }

        Vector<T, N> origin, direction;

    private:
        friend class UnitTestRay;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(Ray<T, N> const& ray0, Ray<T, N> const& ray1)
    {
        return ray0.origin == ray1.origin
            && ray0.direction == ray1.direction;
    }

    template <typename T, size_t N>
    bool operator!=(Ray<T, N> const& ray0, Ray<T, N> const& ray1)
    {
        return !operator==(ray0, ray1);
    }

    template <typename T, size_t N>
    bool operator<(Ray<T, N> const& ray0, Ray<T, N> const& ray1)
    {
        if (ray0.origin < ray1.origin)
        {
            return true;
        }

        if (ray0.origin > ray1.origin)
        {
            return false;
        }

        return ray0.direction < ray1.direction;
    }

    template <typename T, size_t N>
    bool operator<=(Ray<T, N> const& ray0, Ray<T, N> const& ray1)
    {
        return !operator<(ray1, ray0);
    }

    template <typename T, size_t N>
    bool operator>(Ray<T, N> const& ray0, Ray<T, N> const& ray1)
    {
        return operator<(ray1, ray0);
    }

    template <typename T, size_t N>
    bool operator>=(Ray<T, N> const& ray0, Ray<T, N> const& ray1)
    {
        return !operator<(ray0, ray1);
    }

    // Template aliases for convenience.
    template <typename T> using Ray2 = Ray<T, 2>;
    template <typename T> using Ray3 = Ray<T, 3>;
}
