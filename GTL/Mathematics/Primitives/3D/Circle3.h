// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The circle is the intersection of the sphere |X-C|^2 = r^2 and the
// plane Dot(N,X-C) = 0, where C is the circle center, r is the radius,
// and N is a unit-length plane normal.

#include <GTL/Mathematics/Algebra/Vector.h>

namespace gtl
{
    template <typename T>
    class Circle3
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Circle3()
            :
            center{},
            normal{},
            radius(C_<T>(0))
        {
        }

        Circle3(Vector3<T> const& inCenter, Vector3<T> const& inNormal,
            T const& inRadius)
            :
            center(inCenter),
            normal(inNormal),
            radius(inRadius)
        {
        }

        Vector3<T> center, normal;
        T radius;

    private:
        friend class UnitTestCircle3;
    };

    // Comparisons to support sorted containers.
    template <typename T>
    bool operator==(Circle3<T> const& circle0, Circle3<T> const& circle1)
    {
        return circle0.center == circle1.center
            && circle0.normal == circle1.normal
            && circle0.radius == circle1.radius;
    }

    template <typename T>
    bool operator!=(Circle3<T> const& circle0, Circle3<T> const& circle1)
    {
        return !operator==(circle0, circle1);
    }

    template <typename T>
    bool operator<(Circle3<T> const& circle0, Circle3<T> const& circle1)
    {
        if (circle0.center < circle1.center)
        {
            return true;
        }

        if (circle0.center > circle1.center)
        {
            return false;
        }

        if (circle0.normal < circle1.normal)
        {
            return true;
        }

        if (circle0.normal > circle1.normal)
        {
            return false;
        }

        return circle0.radius < circle1.radius;
    }

    template <typename T>
    bool operator<=(Circle3<T> const& circle0, Circle3<T> const& circle1)
    {
        return !operator<(circle1, circle0);
    }

    template <typename T>
    bool operator>(Circle3<T> const& circle0, Circle3<T> const& circle1)
    {
        return operator<(circle1, circle0);
    }

    template <typename T>
    bool operator>=(Circle3<T> const& circle0, Circle3<T> const& circle1)
    {
        return !operator<(circle0, circle1);
    }
}
