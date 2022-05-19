// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// A canonical box has center at the origin and is aligned with the standard
// Euclidean basis vectors. It has E = (e[0],e[1],...,e[N-1]) with e[i] >= 0
// for all i. A zero extent is allowed, meaning the box is degenerate in the
// corresponding direction. A box point is X = (x[0],x[1],...,x[N-1]) with
// |x[i]| <= e[i] for all i.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class CanonicalBox
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        CanonicalBox()
            :
            extent{}
        {
        }

        CanonicalBox(Vector<T, N> const& inExtent)
            :
            extent(inExtent)
        {
        }

        // Compute the vertices of the box. If index i has the bit pattern
        // i = b[N-1]...b[0], then the corner at index i is
        //   vertex[i] = center + sum_{d=0}^{N-1} sign[d]*extent[d]*axis[d]
        // where sign[d] = 2*b[d] - 1.
        void GetVertices(std::array<Vector<T, N>, (1 << N)>& vertex) const
        {
            size_t const imax = (static_cast<size_t>(1) << N);
            for (size_t i = 0; i < imax; ++i)
            {
                MakeZero(vertex[i]);
                for (size_t d = 0, mask = 1; d < N; ++d, mask <<= 1)
                {
                    if ((i & mask) > 0)
                    {
                        vertex[i][d] += extent[d];
                    }
                    else
                    {
                        vertex[i][d] -= extent[d];
                    }
                }
            }
        }

        // It is required that extent[i] >= 0.
        Vector<T, N> extent;

    private:
        friend class UnitTestCanonicalBox;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(CanonicalBox<T, N> const& box0, CanonicalBox<T, N> const& box1)
    {
        return box0.extent == box1.extent;
    }

    template <typename T, size_t N>
    bool operator!=(CanonicalBox<T, N> const& box0, CanonicalBox<T, N> const& box1)
    {
        return !operator==(box0, box1);
    }

    template <typename T, size_t N>
    bool operator<(CanonicalBox<T, N> const& box0, CanonicalBox<T, N> const& box1)
    {
        return box0.extent < box1.extent;
    }

    template <typename T, size_t N>
    bool operator<=(CanonicalBox<T, N> const& box0, CanonicalBox<T, N> const& box1)
    {
        return !operator<(box1, box0);
    }

    template <typename T, size_t N>
    bool operator>(CanonicalBox<T, N> const& box0, CanonicalBox<T, N> const& box1)
    {
        return operator<(box1, box0);
    }

    template <typename T, size_t N>
    bool operator>=(CanonicalBox<T, N> const& box0, CanonicalBox<T, N> const& box1)
    {
        return !operator<(box0, box1);
    }

    // Template aliases for convenience.
    template <typename T> using CanonicalBox2 = CanonicalBox<T, 2>;
    template <typename T> using CanonicalBox3 = CanonicalBox<T, 3>;
}
