// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between two solid triangles in 3D.
// 
// Each triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The closest point on triangle0 is stored in closest[0] with barycentric
// coordinates (b[0],b[1],b[2]) relative to its vertices. The closest point on
// triangle1 is stored in closest[1] with barycentric coordinates relative to
// its vertices. When there are infinitely many choices for the pair of closest
// points, only one of them is returned.

#include <GTL/Mathematics/Distance/3D/DistSegment3Triangle3.h>
#include <array>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Triangle3<T>, Triangle3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                barycentric0{ C_<T>(0), C_<T>(0), C_<T>(0) },
                barycentric1{ C_<T>(0), C_<T>(0), C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<T, 3> barycentric0;
            std::array<T, 3> barycentric1;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Triangle3<T> const& triangle0, Triangle3<T> const& triangle1)
        {
            Output output{};

            DCPQuery<T, Segment3<T>, Triangle3<T>> stQuery{};
            typename DCPQuery<T, Segment3<T>, Triangle3<T>>::Output stOutput{};
            Segment3<T> segment{};

            T const invalid = -C_<T>(1);
            output.distance = invalid;
            output.sqrDistance = invalid;

            // Compare edges of triangle0 to the interior of triangle1.
            for (size_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
            {
                segment.p[0] = triangle0.v[i0];
                segment.p[1] = triangle0.v[i1];

                stOutput = stQuery(segment, triangle1);
                if (output.sqrDistance == invalid ||
                    stOutput.sqrDistance < output.sqrDistance)
                {
                    output.distance = stOutput.distance;
                    output.sqrDistance = stOutput.sqrDistance;
                    output.barycentric0[i0] = C_<T>(1) - stOutput.parameter;
                    output.barycentric0[i1] = stOutput.parameter;
                    output.barycentric0[i2] = C_<T>(0);
                    output.barycentric1 = stOutput.barycentric;
                    output.closest = stOutput.closest;
                }
            }

            // Compare edges of triangle1 to the interior of triangle0.
            for (size_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
            {
                segment.p[0] = triangle1.v[i0];
                segment.p[1] = triangle1.v[i1];

                stOutput = stQuery(segment, triangle0);
                if (output.sqrDistance == invalid ||
                    stOutput.sqrDistance < output.sqrDistance)
                {
                    output.distance = stOutput.distance;
                    output.sqrDistance = stOutput.sqrDistance;
                    output.barycentric0 = stOutput.barycentric;
                    output.barycentric1[i0] = C_<T>(1) - stOutput.parameter;
                    output.barycentric1[i1] = stOutput.parameter;
                    output.barycentric1[i2] = C_<T>(0);
                    output.closest[0] = stOutput.closest[1];
                    output.closest[1] = stOutput.closest[0];
                }
            }

            return output;
        }
    };
}
