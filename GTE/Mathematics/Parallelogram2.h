// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Vector2.h>
#include <array>

namespace gte
{
    template <typename T>
    class Parallelogram2
    {
    public:
        // The default constructor sets the center to (0,0), axis[0] to (1,0),
        // and axis[1] to (0,1).
        Parallelogram2()
            :
            center(Vector2<T>::Zero()),
            axis{ Vector2<T>::Unit(0), Vector2<T>::Unit(1) }
        {
        }

        // The axes must form a right-handed basis. The axes do not have to be
        // orthogonal. The axis lengths do not have to be unit length.
        Parallelogram2(Vector2<T> const& inCenter, std::array<Vector2<T>, 2> const& inAxis)
            :
            center(inCenter),
            axis(inAxis)
        {
            LogAssert(
                DotPerp(inAxis[0], inAxis[1]) > static_cast<T>(0),
                "The axes must form a right-handed basis.");
        }

        // The vertices are listed in counterclockwise order.
        void GetVertices(std::array<Vector2<T>, 4>& vertices)
        {
            vertices[0] = center - axis[0] - axis[1];
            vertices[1] = center + axis[0] - axis[1];
            vertices[2] = center - axis[0] + axis[1];
            vertices[3] = center + axis[0] + axis[1];
        }

        // Public member access.
        Vector2<T> center;
        std::array<Vector2<T>, 2> axis;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Parallelogram2 const& other) const
        {
            return center == other.center && axis == other.axis;
        }

        bool operator!=(Parallelogram2 const& other) const
        {
            return !operator==(other);
        }

        bool operator< (Parallelogram2 const& other) const
        {
            if (center < other.center)
            {
                return true;
            }

            if (center > other.center)
            {
                return false;
            }

            if (axis < other.axis)
            {
                return true;
            }

            if (axis > other.axis)
            {
                return false;
            }

            return false;
        }

        bool operator<=(Parallelogram2 const& other) const
        {
            return !other.operator<(*this);
        }

        bool operator> (Parallelogram2 const& other) const
        {
            return other.operator<(*this);
        }

        bool operator>=(Parallelogram2 const& other) const
        {
            return !operator<(other);
        }
    };
}


