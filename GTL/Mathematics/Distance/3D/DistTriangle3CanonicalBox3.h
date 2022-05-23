// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a solid triangle and a solid canonical box
// in 3D.
// 
// The triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The canonical box has center at the origin and is aligned with the
// coordinate axes. The extents are E = (e[0],e[1],e[2]). A box point is
// Y = (y[0],y[1],y[2]) with |y[i]| <= e[i] for all i.
// 
// The closest point on the triangle closest is stored in closest[0] with
// barycentric coordinates (b[0],b[1],b[2]). The closest point on the box is
// stored in closest[1]. When there are infinitely many choices for the pair
// of closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/3D/DistPlane3CanonicalBox3.h>
#include <GTL/Mathematics/Distance/3D/DistSegment3CanonicalBox3.h>
#include <GTL/Mathematics/Primitives/ND/Triangle.h>
#include <array>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Triangle3<T>, CanonicalBox3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                barycentric{ C_<T>(0), C_<T>(0), C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<T, 3> barycentric;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Triangle3<T> const& triangle, CanonicalBox3<T> const& box)
        {
            Output output{};

            Vector3<T> E10 = triangle.v[1] - triangle.v[0];
            Vector3<T> E20 = triangle.v[2] - triangle.v[0];
            Vector3<T> K = Cross(E10, E20);
            T sqrLength = Dot(K, K);
            Vector3<T> N = K;
            Vector3<T> U0 = E10;
            Normalize(N);
            Normalize(U0);
            Vector3<T> U1 = Cross(N, U0);

            using PBQuery = DCPQuery<T, Plane3<T>, CanonicalBox3<T>>;
            PBQuery pbQuery{};
            Plane3<T> plane(N, triangle.v[0]);
            auto pbOutput = pbQuery(plane, box);

            // closest[0] = b[0] * V[0] + b[1] * V[1] + b[2] * V[2]
            // = V[0] + b[1] * (V[1] - V[0]) + b[2] * (V[2] - V[0]);
            // delta = closest[0] - V[0] = b[1] * E10 + b[2] * E20
            Vector3<T> delta = pbOutput.closest[0] - triangle.v[0];
            Vector3<T> KxDelta = Cross(K, delta);
            output.barycentric[1] = Dot(E20, KxDelta) / sqrLength;
            output.barycentric[2] = -Dot(E10, KxDelta) / sqrLength;
            output.barycentric[0] = C_<T>(1) - output.barycentric[1] - output.barycentric[2];

            if (C_<T>(0) <= output.barycentric[0] && output.barycentric[0] <= C_<T>(1) &&
                C_<T>(0) <= output.barycentric[1] && output.barycentric[1] <= C_<T>(1) &&
                C_<T>(0) <= output.barycentric[2] && output.barycentric[2] <= C_<T>(1))
            {
                output.distance = pbOutput.distance;
                output.sqrDistance = pbOutput.sqrDistance;
                output.closest = pbOutput.closest;
            }
            else
            {
                // The closest plane point is outside the triangle, although
                // it is possible there are points inside the triangle that
                // also are closest points to the box. Regardless, locate a
                // point on an edge of the triangle that is closest to the
                // box.
                using SBQuery = DCPQuery<T, Segment3<T>, CanonicalBox3<T>>;
                SBQuery sbQuery{};
                typename SBQuery::Output sbOutput{};
                Segment3<T> segment{};

                T const invalid = -C_<T>(1);
                output.distance = invalid;
                output.sqrDistance = invalid;

                // Compare edges of the triangle to the box.
                for (size_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
                {
                    segment.p[0] = triangle.v[i0];
                    segment.p[1] = triangle.v[i1];

                    sbOutput = sbQuery(segment, box);
                    if (output.sqrDistance == invalid ||
                        sbOutput.sqrDistance < output.sqrDistance)
                    {
                        output.distance = sbOutput.distance;
                        output.sqrDistance = sbOutput.sqrDistance;
                        output.barycentric[i0] = C_<T>(1) - sbOutput.parameter;
                        output.barycentric[i1] = sbOutput.parameter;
                        output.barycentric[i2] = C_<T>(0);
                        output.closest = sbOutput.closest;
                    }
                }
            }

            return output;
        }
    };
}
