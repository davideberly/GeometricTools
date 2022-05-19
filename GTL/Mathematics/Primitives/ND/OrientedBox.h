// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// A box has center C, axis directions U[i], and extents e[i]. The set
// {U[0],...,U[N-1]} is orthonormal, which means the vectors are unit
// length and mutually perpendicular. The extents are nonnegative; zero is
// allowed, meaning the box is degenerate in the corresponding direction.
// A point X is represented in box coordinates by
// X = C + y[0]*U[0] + y[1]*U[1]. This point is inside or on the box
// whenever |y[i]| <= e[i] for all i.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class OrientedBox
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        OrientedBox()
            :
            center{},
            axis{},
            extent{}
        {
        }

        OrientedBox(Vector<T, N> const& inCenter,
            std::array<Vector<T, N>, N> const& inAxis,
            Vector<T, N> const& inExtent)
            :
            center(inCenter),
            axis(inAxis),
            extent(inExtent)
        {
        }

        // Compute the vertices of the box. If index i has the bit pattern
        // i = b[N-1]...b[0], then the corner at index i is
        //   vertex[i] = center + sum_{d=0}^{N-1} sign[d]*extent[d]*axis[d]
        // where sign[d] = 2*b[d] - 1.
        void GetVertices(std::array<Vector<T, N>, (1 << N)>& vertex) const
        {
            std::array<Vector<T, N>, N> product{};
            for (size_t d = 0; d < N; ++d)
            {
                product[d] = extent[d] * axis[d];
            }

            size_t const imax = (static_cast<size_t>(1) << N);
            for (size_t i = 0; i < imax; ++i)
            {
                vertex[i] = center;
                for (size_t d = 0, mask = 1; d < N; ++d, mask <<= 1)
                {
                    if ((i & mask) > 0)
                    {
                        vertex[i] += product[d];
                    }
                    else
                    {
                        vertex[i] -= product[d];
                    }
                }
            }
        }

        // It is required that extent[i] >= 0.
        Vector<T, N> center;
        std::array<Vector<T, N>, N> axis;
        Vector<T, N> extent;

    private:
        friend class UnitTestOrientedBox;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(OrientedBox<T, N> const& box0, OrientedBox<T, N> const& box1)
    {
        return box0.center == box1.center
            && box0.axis == box1.axis
            && box0.extent == box1.extent;
    }

    template <typename T, size_t N>
    bool operator!=(OrientedBox<T, N> const& box0, OrientedBox<T, N> const& box1)
    {
        return !operator==(box0, box1);
    }

    template <typename T, size_t N>
    bool operator<(OrientedBox<T, N> const& box0, OrientedBox<T, N> const& box1)
    {
        if (box0.center < box1.center)
        {
            return true;
        }

        if (box0.center > box1.center)
        {
            return false;
        }

        if (box0.axis < box1.axis)
        {
            return true;
        }

        if (box0.axis > box1.axis)
        {
            return false;
        }

        return box0.extent < box1.extent;
    }

    template <typename T, size_t N>
    bool operator<=(OrientedBox<T, N> const& box0, OrientedBox<T, N> const& box1)
    {
        return !operator<(box1, box0);
    }

    template <typename T, size_t N>
    bool operator>(OrientedBox<T, N> const& box0, OrientedBox<T, N> const& box1)
    {
        return operator<(box1, box0);
    }

    template <typename T, size_t N>
    bool operator>=(OrientedBox<T, N> const& box0, OrientedBox<T, N> const& box1)
    {
        return !operator<(box0, box1);
    }

    // Template aliases for convenience.
    template <typename T> using OrientedBox2 = OrientedBox<T, 2>;
    template <typename T> using OrientedBox3 = OrientedBox<T, 3>;
}
