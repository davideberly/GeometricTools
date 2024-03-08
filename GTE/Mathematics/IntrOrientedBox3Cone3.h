// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Test for intersection of a box and a cone.  The cone can be infinite
//   0 <= minHeight < maxHeight = std::numeric_limits<Real>::max()
// or finite (cone frustum)
//   0 <= minHeight < maxHeight < std::numeric_limits<Real>::max().
// The algorithm is described in
//   https://www.geometrictools.com/Documentation/IntersectionBoxCone.pdf
// and reports an intersection only when th intersection set has positive
// volume.  For example, let the box be outside the cone.  If the box is
// below the minHeight plane at the cone vertex and just touches the cone
// vertex, no intersection is reported.  If the box is above the maxHeight
// plane and just touches the disk capping the cone, either at a single
// point, a line segment of points or a polygon of points, no intersection
// is reported.

// TODO: These queries were designed when an infinite cone was defined
// by choosing maxHeight of std::numeric_limits<Real>::max(). The Cone<N,Real>
// class has been redesigned not to use std::numeric_limits to allow for
// arithmetic systems that do not have representations for infinities
// (such as BSNumber and BSRational).  The intersection queries need to be
// rewritten for the new class design.  FOR NOW, the queries will work with
// float/double when you create a cone using the cone-frustum constructor
// Cone(ray, angle, minHeight, std::numeric_limits<Real>::max()).

#include <Mathematics/Cone.h>
#include <Mathematics/OrientedBox.h>
#include <Mathematics/IntrAlignedBox3Cone3.h>
#include <cstdint>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, OrientedBox3<Real>, Cone3<Real>>
        :
        public TIQuery<Real, AlignedBox3<Real>, Cone3<Real>>
    {
    public:
        struct Result
            :
            public TIQuery<Real, AlignedBox3<Real>, Cone3<Real>>::Result
        {
            Result()
                :
                TIQuery<Real, AlignedBox3<Real>, Cone3<Real>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(OrientedBox3<Real> const& box, Cone3<Real> const& cone)
        {
            // Transform the cone and box so that the cone vertex is at the
            // origin and the box is axis aligned.  This allows us to call the
            // base class operator()(...).
            Vector3<Real> diff = box.center - cone.ray.origin;
            Vector3<Real> xfrmBoxCenter
            {
                Dot(box.axis[0], diff),
                Dot(box.axis[1], diff),
                Dot(box.axis[2], diff)
            };
            AlignedBox3<Real> xfrmBox{};
            xfrmBox.min = xfrmBoxCenter - box.extent;
            xfrmBox.max = xfrmBoxCenter + box.extent;

            Cone3<Real> xfrmCone = cone;
            for (int32_t i = 0; i < 3; ++i)
            {
                xfrmCone.ray.origin[i] = (Real)0;
                xfrmCone.ray.direction[i] = Dot(box.axis[i], cone.ray.direction);
            }

            // Test for intersection between the aligned box and the cone.
            auto bcResult = TIQuery<Real, AlignedBox3<Real>, Cone3<Real>>::operator()(xfrmBox, xfrmCone);
            Result result{};
            result.intersect = bcResult.intersect;
            return result;
        }
    };

    // Template alias for convenience.
    template <typename Real>
    using TIOrientedBox3Cone3 = TIQuery<Real, OrientedBox3<Real>, Cone3<Real>>;
}
