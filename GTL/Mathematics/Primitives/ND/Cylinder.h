// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The cylinder axis is a line specified by a center point C and a unit-length
// direction D. The cylinder wall is at a distance R units from the axis. An
// infinite cylinder has infinite height. A finite cylinder has a finite
// height H, and the bounding disks have centers C-(H/2)*D and C+(H/2)*D and
// radius R.
//
// NOTE: Some of the geometric queries involve infinite cylinders. To support
// exact arithmetic, it is necessary to avoid std::numeric_limits members
// such as infinity() and max(). Instead, the queries require you to set
// the infinite cylinder 'height' to -1.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <cstddef>

namespace gtl
{
    template <typename T, size_t N>
    class Cylinder
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Cylinder()
            :
            center{},
            direction{},
            radius(C_<T>(0)),
            height(C_<T>(0))
        {
        }

        Cylinder(Vector<T, N> const& inCenter, Vector<T, N> const& inDirection,
            T const& inRadius, T const& inHeight)
            :
            center(inCenter),
            direction(inDirection),
            radius(inRadius),
            height(inHeight)
        {
        }

        // Please read the mpte at the beginning of this file about setting
        // the 'height' member for infinite cylinders.
        inline void MakeInfiniteCylinder()
        {
            height = -C_<T>(1);
        }

        inline void MakeFiniteCylinder(T const& inHeight)
        {
            if (inHeight >= C_<T>(0))
            {
                height = inHeight;
            }
        }

        inline bool IsFinite() const
        {
            return height >= C_<T>(0);
        }

        inline bool IsInfinite() const
        {
            return height < C_<T>(0);
        }

        Vector<T, N> center, direction;
        T radius, height;

    private:
        friend class UnitTestCylinder;
    };

    // Comparisons to support sorted containers.
    template <typename T, size_t N>
    bool operator==(Cylinder<T, N> const& cylinder0, Cylinder<T, N> const& cylinder1)
    {
        return cylinder0.center == cylinder1.center
            && cylinder0.direction == cylinder1.direction
            && cylinder0.radius == cylinder1.radius
            && cylinder0.height == cylinder1.height;
    }

    template <typename T, size_t N>
    bool operator!=(Cylinder<T, N> const& cylinder0, Cylinder<T, N> const& cylinder1)
    {
        return !operator==(cylinder0, cylinder1);
    }

    template <typename T, size_t N>
    bool operator<(Cylinder<T, N> const& cylinder0, Cylinder<T, N> const& cylinder1)
    {
        if (cylinder0.center < cylinder1.center)
        {
            return true;
        }

        if (cylinder0.center > cylinder1.center)
        {
            return false;
        }

        if (cylinder0.direction < cylinder1.direction)
        {
            return true;
        }

        if (cylinder0.direction > cylinder1.direction)
        {
            return false;
        }

        if (cylinder0.radius < cylinder1.radius)
        {
            return true;
        }

        if (cylinder0.radius > cylinder1.radius)
        {
            return false;
        }

        return cylinder0.height < cylinder1.height;
    }

    template <typename T, size_t N>
    bool operator<=(Cylinder<T, N> const& cylinder0, Cylinder<T, N> const& cylinder1)
    {
        return !operator<(cylinder1, cylinder0);
    }

    template <typename T, size_t N>
    bool operator>(Cylinder<T, N> const& cylinder0, Cylinder<T, N> const& cylinder1)
    {
        return operator<(cylinder1, cylinder0);
    }

    template <typename T, size_t N>
    bool operator>=(Cylinder<T, N> const& cylinder0, Cylinder<T, N> const& cylinder1)
    {
        return !operator<(cylinder0, cylinder1);
    }

    // Template alias for convenience.
    template <typename T> using Cylinder3 = Cylinder<T, 3>;
}
