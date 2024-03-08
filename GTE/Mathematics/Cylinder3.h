// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The cylinder axis is a line. The origin of the cylinder is chosen to be
// the line origin. The cylinder wall is at a distance R units from the axis.
// An infinite cylinder has infinite height. A finite cylinder has center C
// at the line origin and has a finite height H. The segment for the finite
// cylinder has endpoints C-(H/2)*D and C+(H/2)*D where D is a unit-length
// direction of the line.
//
// NOTE: Some of the geometric queries involve infinite cylinders. To support
// exact arithmetic, it is necessary to avoid std::numeric_limits members
// such as infinity() and max(). Instead, the queries require you to set
// the infinite cylinder 'height' to -1.

#include <Mathematics/Line.h>

namespace gte
{
    template <typename T>
    class Cylinder3
    {
    public:
        // Construction and destruction. The default constructor sets axis
        // to (0,0,1), radius to 1, and height to 1.
        Cylinder3()
            :
            axis(Line3<T>()),
            radius((T)1),
            height((T)1)
        {
        }

        Cylinder3(Line3<T> const& inAxis, T inRadius, T inHeight)
            :
            axis(inAxis),
            radius(inRadius),
            height(inHeight)
        {
        }

        // Please read the NOTE at the beginning of this file about setting
        // the 'height' member for infinite cylinders.
        inline void MakeInfiniteCylinder()
        {
            height = static_cast<T>(-1);
        }

        inline void MakeFiniteCylinder(T const& inHeight)
        {
            if (inHeight >= static_cast<T>(0))
            {
                height = inHeight;
            }
        }

        inline bool IsFinite() const
        {
            return height >= static_cast<T>(0);
        }

        inline bool IsInfinite() const
        {
            return height < static_cast<T>(0);
        }

        Line3<T> axis;
        T radius, height;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Cylinder3 const& cylinder) const
        {
            return axis == cylinder.axis
                && radius == cylinder.radius
                && height == cylinder.height;
        }

        bool operator!=(Cylinder3 const& cylinder) const
        {
            return !operator==(cylinder);
        }

        bool operator< (Cylinder3 const& cylinder) const
        {
            if (axis < cylinder.axis)
            {
                return true;
            }

            if (axis > cylinder.axis)
            {
                return false;
            }

            if (radius < cylinder.radius)
            {
                return true;
            }

            if (radius > cylinder.radius)
            {
                return false;
            }

            return height < cylinder.height;
        }

        bool operator<=(Cylinder3 const& cylinder) const
        {
            return !cylinder.operator<(*this);
        }

        bool operator> (Cylinder3 const& cylinder) const
        {
            return cylinder.operator<(*this);
        }

        bool operator>=(Cylinder3 const& cylinder) const
        {
            return !operator<(cylinder);
        }
    };
}
