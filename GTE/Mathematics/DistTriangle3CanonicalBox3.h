// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DistPlane3CanonicalBox3.h>
#include <Mathematics/DistSegment3CanonicalBox3.h>
#include <Mathematics/Triangle.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Triangle3<T>, CanonicalBox3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                barycentric{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 3> barycentric;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Triangle3<T> const& triangle, CanonicalBox3<T> const& box)
        {
            Result result{};

            Vector3<T> E10 = triangle.v[1] - triangle.v[0];
            Vector3<T> E20 = triangle.v[2] - triangle.v[0];
            Vector3<T> K = Cross(E10, E20);
            T sqrLength = Dot(K, K);
            Vector3<T> N = K;
            Normalize(N);

            using PBQuery = DCPQuery<T, Plane3<T>, CanonicalBox3<T>>;
            PBQuery pbQuery{};
            Plane3<T> plane(N, triangle.v[0]);
            auto pbOutput = pbQuery(plane, box);

            // closest[0] = b[0] * V[0] + b[1] * V[1] + b[2] * V[2]
            // = V[0] + b[1] * (V[1] - V[0]) + b[2] * (V[2] - V[0]);
            // delta = closest[0] - V[0] = b[1] * E10 + b[2] * E20
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            Vector3<T> delta = pbOutput.closest[0] - triangle.v[0];
            Vector3<T> KxDelta = Cross(K, delta);
            result.barycentric[1] = Dot(E20, KxDelta) / sqrLength;
            result.barycentric[2] = -Dot(E10, KxDelta) / sqrLength;
            result.barycentric[0] = one - result.barycentric[1] - result.barycentric[2];

            if (zero <= result.barycentric[0] && result.barycentric[0] <= one &&
                zero <= result.barycentric[1] && result.barycentric[1] <= one &&
                zero <= result.barycentric[2] && result.barycentric[2] <= one)
            {
                result.distance = pbOutput.distance;
                result.sqrDistance = pbOutput.sqrDistance;
                result.closest = pbOutput.closest;
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
                typename SBQuery::Result sbOutput{};
                Segment3<T> segment{};

                T const invalid = static_cast<T>(-1);
                result.distance = invalid;
                result.sqrDistance = invalid;

                // Compare edges of the triangle to the box.
                for (size_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
                {
                    segment.p[0] = triangle.v[i0];
                    segment.p[1] = triangle.v[i1];

                    sbOutput = sbQuery(segment, box);
                    if (result.sqrDistance == invalid ||
                        sbOutput.sqrDistance < result.sqrDistance)
                    {
                        result.distance = sbOutput.distance;
                        result.sqrDistance = sbOutput.sqrDistance;
                        result.barycentric[i0] = one - sbOutput.parameter;
                        result.barycentric[i1] = sbOutput.parameter;
                        result.barycentric[i2] = zero;
                        result.closest = sbOutput.closest;
                    }
                }
            }

            return result;
        }
    };
}
