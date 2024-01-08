// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The box is aligned with the standard coordinate axes, which allows us to
// represent it using minimum and maximum values along each axis.  Some
// algorithms prefer the centered representation that is used for oriented
// boxes.  The center is C and the extents are the half-lengths in each
// coordinate-axis direction.

#include <Mathematics/Vector.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class AlignedBox
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // minimum values to -1 and the maximum values to +1.
        AlignedBox()
        {
            T const negOne = static_cast<T>(-1);
            T const one = static_cast<T>(1);
            for (int32_t i = 0; i < N; ++i)
            {
                min[i] = negOne;
                max[i] = one;
            }
        }

        // Please ensure that inMin[i] <= inMax[i] for all i.
        AlignedBox(Vector<N, T> const& inMin, Vector<N, T> const& inMax)
        {
            for (int32_t i = 0; i < N; ++i)
            {
                min[i] = inMin[i];
                max[i] = inMax[i];
            }
        }

        // Compute the centered representation.  NOTE:  If you set the minimum
        // and maximum values, compute C and extents, and then recompute the
        // minimum and maximum values, the numerical round-off errors can lead
        // to results different from what you started with.
        void GetCenteredForm(Vector<N, T>& center, Vector<N, T>& extent) const
        {
            T const half = static_cast<T>(0.5);
            center = (max + min) * half;
            extent = (max - min) * half;
        }

        // Compute the vertices of the box. If index i has the bit pattern
        // i = b[N-1]...b[0], then the corner at index i is vertex[i], where
        // vertex[i][d] = min[d] whern b[d] = 0 or vertex[i][d = max[d] when
        // b[d] = 1.
        void GetVertices(std::array<Vector<N, T>, (1 << N)>& vertex) const
        {
            int32_t const imax = (1 << N);
            for (int32_t i = 0; i < imax; ++i)
            {
                for (int32_t d = 0, mask = 1; d < N; ++d, mask <<= 1)
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

        // Public member access.  It is required that min[i] <= max[i].
        Vector<N, T> min, max;

    public:
        // Comparisons to support sorted containers.
        bool operator==(AlignedBox const& box) const
        {
            return min == box.min && max == box.max;
        }

        bool operator!=(AlignedBox const& box) const
        {
            return !operator==(box);
        }

        bool operator< (AlignedBox const& box) const
        {
            if (min < box.min)
            {
                return true;
            }

            if (min > box.min)
            {
                return false;
            }

            return max < box.max;
        }

        bool operator<=(AlignedBox const& box) const
        {
            return !box.operator<(*this);
        }

        bool operator> (AlignedBox const& box) const
        {
            return box.operator<(*this);
        }

        bool operator>=(AlignedBox const& box) const
        {
            return !operator<(box);
        }
    };

    // Template aliases for convenience.
    template <typename T>
    using AlignedBox2 = AlignedBox<2, T>;

    template <typename T>
    using AlignedBox3 = AlignedBox<3, T>;
}
