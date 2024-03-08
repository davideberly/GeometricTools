// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The plane containing ellipse is Dot(N,X-C) = 0 where X is any point in the
// plane, C is the ellipse center, and N is a unit-length normal to the plane.
// Vectors A0, A1, and N form an orthonormal right-handed set.  The ellipse in
// the plane is parameterized by X = C + e0*cos(t)*A0 + e1*sin(t)*A1, where A0
// is the major axis, A1 is the minor axis, and e0 and e1 are the extents
// along those axes.  The angle t is in [-pi,pi) and e0 >= e1 > 0.

#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>
#include <array>

namespace gte
{
    template <typename Real>
    class Ellipse3
    {
    public:
        // Construction and destruction.  The default constructor sets center
        // to (0,0,0), A0 to (1,0,0), A1 to (0,1,0), normal to (0,0,1), e0
        // to 1, and e1 to 1.
        Ellipse3()
            :
            center(Vector3<Real>::Zero()),
            normal(Vector3<Real>::Unit(2)),
            axis{ Vector3<Real>::Unit(0), Vector3<Real>::Unit(1) },
            extent{ (Real)1, (Real)1 }
        {
        }

        Ellipse3(Vector3<Real> const& inCenter, Vector3<Real> const& inNormal,
            std::array<Vector3<Real>, 2> const& inAxis, Vector2<Real> const& inExtent)
            :
            center(inCenter),
            normal(inNormal),
            axis(inAxis),
            extent(inExtent)
        {
        }

        // Public member access.
        Vector3<Real> center, normal;
        std::array<Vector3<Real>, 2> axis;
        Vector2<Real> extent;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Ellipse3 const& ellipse) const
        {
            return center == ellipse.center
                && normal == ellipse.normal
                && axis[0] == ellipse.axis[0]
                && axis[1] == ellipse.axis[1]
                && extent == ellipse.extent;
        }

        bool operator!=(Ellipse3 const& ellipse) const
        {
            return !operator==(ellipse);
        }

        bool operator< (Ellipse3 const& ellipse) const
        {
            if (center < ellipse.center)
            {
                return true;
            }

            if (center > ellipse.center)
            {
                return false;
            }

            if (normal < ellipse.normal)
            {
                return true;
            }

            if (normal > ellipse.normal)
            {
                return false;
            }

            if (axis[0] < ellipse.axis[0])
            {
                return true;
            }

            if (axis[0] > ellipse.axis[0])
            {
                return false;
            }

            if (axis[1] < ellipse.axis[1])
            {
                return true;
            }

            if (axis[1] > ellipse.axis[1])
            {
                return false;
            }

            return extent < ellipse.extent;
        }

        bool operator<=(Ellipse3 const& ellipse) const
        {
            return !ellipse.operator<(*this);
        }

        bool operator> (Ellipse3 const& ellipse) const
        {
            return ellipse.operator<(*this);
        }

        bool operator>=(Ellipse3 const& ellipse) const
        {
            return !operator<(ellipse);
        }
    };
}
