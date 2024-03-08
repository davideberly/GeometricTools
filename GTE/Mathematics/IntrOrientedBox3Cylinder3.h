// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The query is for finite cylinders. The cylinder and box are considered to
// be solids. The cylinder has center C, unit-length axis direction D, radius
// r and height h. The oriented box is converted to a canonical box after
// which a test-intersection query is performed on the finite cylinder and the
// canonical box. See the comments in IntrCanonicalBox3Cylinder3.h for a brief
// description. The details are in
//   https://www.geometrictools.com/Documentation/IntersectionBoxCylinder.pdf

#include <Mathematics/OrientedBox.h>
#include <Mathematics/IntrCanonicalBox3Cylinder3.h>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, OrientedBox3<T>, Cylinder3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(OrientedBox3<T> const& box, Cylinder3<T> const& cylinder)
        {
            LogAssert(
                cylinder.IsFinite(),
                "Infinite cylinders are not yet supported.");

            // Convert the problem to one involving a finite cylinder and a
            // canonical box. This involves translating the box center to the
            // origin and then rotating the box axes to the standard coordinate
            // axes. The cylinder center must also be translated and rotated
            // accordingly.
            CanonicalBox3<T> cbox(box.extent);
            Vector3<T> diff = cylinder.axis.origin - box.center;
            Cylinder3<T> transformedCylinder{};
            transformedCylinder.radius = cylinder.radius;
            transformedCylinder.height = cylinder.height;
            for (int32_t i = 0; i < 3; ++i)
            {
                transformedCylinder.axis.origin[i] = Dot(box.axis[i], diff);
                transformedCylinder.axis.direction[i] = Dot(box.axis[i], cylinder.axis.direction);
            }

            TIQuery<T, CanonicalBox3<T>, Cylinder3<T>> bcQuery{};
            auto bcResult = bcQuery(cbox, transformedCylinder);
            Result result{};
            result.intersect = bcResult.intersect;
            return result;
        }
    };
}
