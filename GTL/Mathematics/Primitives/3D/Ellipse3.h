// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The plane containing ellipse is Dot(N,X-C) = 0 where X is any point in the
// plane, C is the ellipse center, and N is a unit-length normal to the plane.
// Vectors A0, A1, and N form an orthonormal right-handed set. The ellipse in
// the plane is parameterized by X = C + e0*cos(t)*A0 + e1*sin(t)*A1, where A0
// is the major axis, A1 is the minor axis, and e0 and e1 are the extents
// along those axes. The angle t is in [-pi,pi) and e0 >= e1 > 0.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>

namespace gtl
{
    template <typename T>
    class Ellipse3
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all members to zero.
        Ellipse3()
            :
            center{},
            normal{},
            axis{},
            extent{}
        {
        }

        Ellipse3(Vector3<T> const& inCenter, Vector3<T> const& inNormal,
            std::array<Vector3<T>, 2> const& inAxis, Vector2<T> const& inExtent)
            :
            center(inCenter),
            normal(inNormal),
            axis(inAxis),
            extent(inExtent)
        {
        }

        // Public member access.
        Vector3<T> center, normal;
        std::array<Vector3<T>, 2> axis;
        Vector2<T> extent;

    private:
        friend class UnitTestEllipse3;
    };

    // Comparisons to support sorted containers.
    template <typename T>
    bool operator==(Ellipse3<T> const& ellipse0, Ellipse3<T> const& ellipse1)
    {
        return ellipse0.center == ellipse1.center
            && ellipse0.normal == ellipse1.normal
            && ellipse0.axis == ellipse1.axis
            && ellipse0.extent == ellipse1.extent;
    }

    template <typename T>
    bool operator!=(Ellipse3<T> const& ellipse0, Ellipse3<T> const& ellipse1)
    {
        return !operator==(ellipse0, ellipse1);
    }

    template <typename T>
    bool operator<(Ellipse3<T> const& ellipse0, Ellipse3<T> const& ellipse1)
    {
        if (ellipse0.center < ellipse1.center)
        {
            return true;
        }

        if (ellipse0.center > ellipse1.center)
        {
            return false;
        }

        if (ellipse0.normal < ellipse1.normal)
        {
            return true;
        }

        if (ellipse0.normal > ellipse1.normal)
        {
            return false;
        }

        if (ellipse0.axis < ellipse1.axis)
        {
            return true;
        }

        if (ellipse0.axis > ellipse1.axis)
        {
            return false;
        }

        return ellipse0.extent < ellipse1.extent;
    }

    template <typename T>
    bool operator<=(Ellipse3<T> const& ellipse0, Ellipse3<T> const& ellipse1)
    {
        return !operator<(ellipse1, ellipse0);
    }

    template <typename T>
    bool operator>(Ellipse3<T> const& ellipse0, Ellipse3<T> const& ellipse1)
    {
        return operator<(ellipse1, ellipse0);
    }

    template <typename T>
    bool operator>=(Ellipse3<T> const& ellipse0, Ellipse3<T> const& ellipse1)
    {
        return !operator<(ellipse0, ellipse1);
    }
}
