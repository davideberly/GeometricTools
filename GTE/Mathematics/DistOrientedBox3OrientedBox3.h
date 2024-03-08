// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between two solid oriented boxes in 3D.
// 
// Each oriented box has center C, unit-length axis directions U[i], and
// extents e[i] for all i. A box point is X = C + sum_i y[i] * U[i], where
// |y[i]| <= e[i] for all i.
// 
// The closest point of the first oriented box is stored in closest[0]. The 
// closest point of the second oriented box is stored in closest[1]. When
// there are infinitely many choices for the pair of closest points, only one
// of them is returned.

#include <Mathematics/DistRectangle3OrientedBox3.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, OrientedBox3<T>, OrientedBox3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(OrientedBox3<T> const& box0, OrientedBox3<T> const& box1)
        {
            Result result{};

            DCPQuery<T, Rectangle3<T>, OrientedBox3<T>> rbQuery{};
            typename DCPQuery<T, Rectangle3<T>, OrientedBox3<T>>::Result rbOutput{};
            Rectangle3<T> rectangle{};

            T const invalid = static_cast<T>(-1);
            result.distance = invalid;
            result.sqrDistance = invalid;

            // Compare faces of box0 to box1.
            for (int32_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
            {
                rectangle.axis[0] = box0.axis[i0];
                rectangle.axis[1] = box0.axis[i1];
                rectangle.extent[0] = box0.extent[i0];
                rectangle.extent[1] = box0.extent[i1];

                Vector3<T> scaledAxis = box0.extent[i2] * box0.axis[i2];
                rectangle.center = box0.center + scaledAxis;
                rbOutput = rbQuery(rectangle, box1);
                if (result.sqrDistance == invalid ||
                    rbOutput.sqrDistance < result.sqrDistance)
                {
                    result.distance = rbOutput.distance;
                    result.sqrDistance = rbOutput.sqrDistance;
                    result.closest = rbOutput.closest;
                }

                rectangle.center = box0.center - scaledAxis;
                rbOutput = rbQuery(rectangle, box1);
                if (result.sqrDistance == invalid ||
                    rbOutput.sqrDistance < result.sqrDistance)
                {
                    result.distance = rbOutput.distance;
                    result.sqrDistance = rbOutput.sqrDistance;
                    result.closest = rbOutput.closest;
                }
            }

            // Compare faces of box1 to box0.
            for (int32_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
            {
                rectangle.axis[0] = box1.axis[i0];
                rectangle.axis[1] = box1.axis[i1];
                rectangle.extent[0] = box1.extent[i0];
                rectangle.extent[1] = box1.extent[i1];

                Vector3<T> scaledAxis = box1.extent[i2] * box1.axis[i2];
                rectangle.center = box1.center + scaledAxis;
                rbOutput = rbQuery(rectangle, box0);
                if (result.sqrDistance == invalid ||
                    rbOutput.sqrDistance < result.sqrDistance)
                {
                    result.distance = rbOutput.distance;
                    result.sqrDistance = rbOutput.sqrDistance;
                    result.closest[0] = rbOutput.closest[1];
                    result.closest[1] = rbOutput.closest[0];
                }

                rectangle.center = box1.center - scaledAxis;
                rbOutput = rbQuery(rectangle, box0);
                if (result.sqrDistance == invalid ||
                    rbOutput.sqrDistance < result.sqrDistance)
                {
                    result.distance = rbOutput.distance;
                    result.sqrDistance = rbOutput.sqrDistance;
                    result.closest[0] = rbOutput.closest[1];
                    result.closest[1] = rbOutput.closest[0];
                }
            }

            return result;
        }
    };
}
