// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// An axis-angle representation for a rotation. The axis is unit-length and
// the angle is in radians.

#include <GTL/Mathematics/Algebra/Vector.h>

namespace gtl
{
    template <typename T>
    class AxisAngle
    {
    public:
        using value_type = T;

        // The axis initializes its elements to zero (invalid axis).
        AxisAngle()
            :
            axis{},
            angle(C_<T>(0))
        {
        }

        // Set the axis and angle to the specified values. The input vector
        // must be unit length.
        AxisAngle(Vector3<T> const& inAxis, T const& inAngle)
            :
            axis(inAxis),
            angle(inAngle)
        {
        }

        Vector3<T> axis;
        T angle;

    private:
        friend class UnitTestAxisAngle;
    };
}
