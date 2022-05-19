// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The box is aligned with the standard coordinate axes, which allows us to
// represent it using minimum and maximum values along each axis. Some
// algorithms prefer the centered representation that is used for oriented
// boxes. The center is C and the extents are the half-lengths in each
// coordinate-axis direction.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class AlignedBox
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all member to zero.
        AlignedBox()
            :
            min{},
            max{}
        {
        }

        // Please ensure that inMin[i] <= inMax[i] for all i.
        AlignedBox(Vector<T, N> const& inMin, Vector<T, N> const& inMax)
            :
            min{},
            max{}
        {
            for (size_t i = 0; i < N; ++i)
            {
                min[i] = inMin[i];
                max[i] = inMax[i];
                GTL_ARGUMENT_ASSERT(
                    min[i] <= max[i],
                    "Invalid ordering of min and max.");
            }
        }

        // Compute the centered representation. NOTE: If you set the minimum
        // and maximum values, compute C and extents and then recompute the
        // minimum and maximum values, the numerical round-off errors can lead
        // to results different from what you started with.
        void GetCenteredForm(Vector<T, N>& center, Vector<T, N>& extent) const
        {
            center = C_<T>(1, 2) * (max + min);
            extent = C_<T>(1, 2) * (max - min);
        }

        // Compute the vertices of the box. If index i has the bit pattern
        // i = b[N-1]...b[0], then the corner at index i is vertex[i], where
        // vertex[i] = min[i] whern b[d] = 0 or max[i] when b[d] = 1.
        void GetVertices(std::array<Vector<T, N>, (1 << N)>& vertex) const
        {
            size_t const imax = (static_cast<size_t>(1) << N);
            for (size_t i = 0; i < imax; ++i)
            {
                for (size_t d = 0, mask = 1; d < N; ++d, mask <<= 1)
                {
                    if ((i & mask) > 0)
                    {
                        vertex[i][d] = max[d];
                    }
                    else
                    {
                        vertex[i][d] = min[d];
                    }
                }
            }
        }

        // It is required that min[i] <= max[i].
        Vector<T, N> min, max;

    private:
        friend class UnitTestAlignedBox;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(AlignedBox<T, N> const& box0, AlignedBox<T, N> const& box1)
    {
        return box0.min == box1.min
            && box0.max == box1.max;
    }

    template <typename T, size_t N>
    bool operator!=(AlignedBox<T, N> const& box0, AlignedBox<T, N> const& box1)
    {
        return !operator==(box0, box1);
    }

    template <typename T, size_t N>
    bool operator<(AlignedBox<T, N> const& box0, AlignedBox<T, N> const& box1)
    {
        if (box0.min < box1.min)
        {
            return true;
        }

        if (box0.min > box1.min)
        {
            return false;
        }

        return box0.max < box1.max;
    }

    template <typename T, size_t N>
    bool operator<=(AlignedBox<T, N> const& box0, AlignedBox<T, N> const& box1)
    {
        return !operator<(box1, box0);
    }

    template <typename T, size_t N>
    bool operator>(AlignedBox<T, N> const& box0, AlignedBox<T, N> const& box1)
    {
        return operator<(box1, box0);
    }

    template <typename T, size_t N>
    bool operator>=(AlignedBox<T, N> const& box0, AlignedBox<T, N> const& box1)
    {
        return !operator<(box0, box1);
    }

    // Template aliases for convenience.
    template <typename T> using AlignedBox2 = AlignedBox<T, 2>;
    template <typename T> using AlignedBox3 = AlignedBox<T, 3>;
}
