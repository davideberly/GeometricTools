// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// A box has center C, axis directions U[i], and extents e[i].  The set
// {U[0],...,U[N-1]} is orthonormal, which means the vectors are
// unit-length and mutually perpendicular.  The extents are nonnegative;
// zero is allowed, meaning the box is degenerate in the corresponding
// direction.  A point X is represented in box coordinates by
// X = C + y[0]*U[0] + y[1]*U[1].  This point is inside or on the
// box whenever |y[i]| <= e[i] for all i.

#include <Mathematics/Vector.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class OrientedBox
    {
    public:
        // Construction and destruction.  The default constructor sets the
        // center to (0,...,0), axis d to Vector<N,T>::Unit(d) and
        // extent d to +1.
        OrientedBox()
        {
            center.MakeZero();
            for (int32_t i = 0; i < N; ++i)
            {
                axis[i].MakeUnit(i);
                extent[i] = (T)1;
            }
        }

        OrientedBox(Vector<N, T> const& inCenter,
            std::array<Vector<N, T>, N> const& inAxis,
            Vector<N, T> const& inExtent)
            :
            center(inCenter),
            axis(inAxis),
            extent(inExtent)
        {
        }

        // Compute the vertices of the box.  If index i has the bit pattern
        // i = b[N-1]...b[0], then
        // vertex[i] = center + sum_{d=0}^{N-1} sign[d] * extent[d] * axis[d]
        // where sign[d] = 2*b[d] - 1.
        void GetVertices(std::array<Vector<N, T>, (1 << N)>& vertex) const
        {
            std::array<Vector<N, T>, N> product;
            for (int32_t d = 0; d < N; ++d)
            {
                product[d] = extent[d] * axis[d];
            }

            int32_t const imax = (1 << N);
            for (int32_t i = 0; i < imax; ++i)
            {
                vertex[i] = center;
                for (int32_t d = 0, mask = 1; d < N; ++d, mask <<= 1)
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

        // Public member access.  It is required that extent[i] >= 0.
        Vector<N, T> center;
        std::array<Vector<N, T>, N> axis;
        Vector<N, T> extent;

    public:
        // Comparisons to support sorted containers.
        bool operator==(OrientedBox const& box) const
        {
            return center == box.center && axis == box.axis && extent == box.extent;
        }

        bool operator!=(OrientedBox const& box) const
        {
            return !operator==(box);
        }

        bool operator< (OrientedBox const& box) const
        {
            if (center < box.center)
            {
                return true;
            }

            if (center > box.center)
            {
                return false;
            }

            if (axis < box.axis)
            {
                return true;
            }

            if (axis > box.axis)
            {
                return false;
            }

            return extent < box.extent;
        }

        bool operator<=(OrientedBox const& box) const
        {
            return !box.operator<(*this);
        }

        bool operator> (OrientedBox const& box) const
        {
            return box.operator<(*this);
        }

        bool operator>=(OrientedBox const& box) const
        {
            return !operator<(box);
        }
    };

    // Template aliases for convenience.
    template <typename T>
    using OrientedBox2 = OrientedBox<2, T>;

    template <typename T>
    using OrientedBox3 = OrientedBox<3, T>;
}
