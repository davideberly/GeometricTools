// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// A canonical box has center at the origin and is aligned with the standard
// Euclidean basis vectors. It has E = (e[0],e[1],...,e[N-1]) with e[i] >= 0
// for all i. A zero extent is allowed, meaning the box is degenerate in the
// corresponding direction. A box point is X = (x[0],x[1],...,x[N-1]) with
// |x[i]| <= e[i] for all i.

#include <Mathematics/Vector.h>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class CanonicalBox
    {
    public:
        // Construction. The default constructor sets all members to zero.
        CanonicalBox()
            :
            extent{}
        {
        }

        CanonicalBox(Vector<N, T> const& inExtent)
            :
            extent(inExtent)
        {
        }

        // Compute the vertices of the box. If index i has the bit pattern
        // i = b[N-1]...b[0], then the corner at index i is
        //   vertex[i] = center + sum_{d=0}^{N-1} sign[d]*extent[d]*axis[d]
        // where sign[d] = 2*b[d] - 1.
        void GetVertices(std::array<Vector<N, T>, (1 << N)>& vertex) const
        {
            int32_t const imax = (1 << N);
            for (int32_t i = 0; i < imax; ++i)
            {
                vertex[i].MakeZero();
                for (int32_t d = 0, mask = 1; d < N; ++d, mask <<= 1)
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
        Vector<N, T> extent;

    public:
        // Comparisons to support sorted containers.
        bool operator==(CanonicalBox<N, T> const& box) const
        {
            return extent == box.extent;
        }

        bool operator!=(CanonicalBox<N, T> const& box) const
        {
            return !operator==(box);
        }

        bool operator<(CanonicalBox<N, T> const& box) const
        {
            return extent < box.extent;
        }

        bool operator<=(CanonicalBox<N, T> const& box) const
        {
            return !box.operator<(*this);
        }

        bool operator>(CanonicalBox<N, T> const& box) const
        {
            return box.operator<(*this);
        }

        bool operator>=(CanonicalBox<N, T> const& box) const
        {
            return !operator<(box);
        }
    };

    // Template aliases for convenience.
    template <typename T> using CanonicalBox2 = CanonicalBox<2, T>;
    template <typename T> using CanonicalBox3 = CanonicalBox<3, T>;
}
