// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a rectangle and a solid canonical box in 3D.
// 
// The rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
//
// The canonical box has center at the origin and is aligned with the
// coordinate axes. The extents are E = (e[0],e[1],e[2]). A box point is
// Y = (y[0],y[1],y[2]) with |y[i]| <= e[i] for all i.
// 
// The closest point on the rectangle is stored in closest[0] with
// W-coordinates (s[0],s[1]). The closest point on the box is stored in
// closest[1]. When there are infinitely many choices for the pair of closest
// points, only one of them is returned.
//
// TODO: Modify to support non-unit-length W[].

#include <Mathematics/DistPlane3CanonicalBox3.h>
#include <Mathematics/DistSegment3CanonicalBox3.h>
#include <Mathematics/Rectangle.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Rectangle3<T>, CanonicalBox3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                cartesian{ static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 2> cartesian;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Rectangle3<T> const& rectangle, CanonicalBox3<T> const& box)
        {
            Result result{};

            using PBQuery = DCPQuery<T, Plane3<T>, CanonicalBox3<T>>;
            PBQuery pbQuery{};
            Vector3<T> normal = Cross(rectangle.axis[0], rectangle.axis[1]);
            Plane3<T> plane(normal, rectangle.center);
            auto pbOutput = pbQuery(plane, box);
            Vector3<T> delta = pbOutput.closest[0] - rectangle.center;
            result.cartesian[0] = Dot(rectangle.axis[0], delta);
            result.cartesian[1] = Dot(rectangle.axis[1], delta);

            if (std::fabs(result.cartesian[0]) <= rectangle.extent[0] &&
                std::fabs(result.cartesian[1]) <= rectangle.extent[1])
            {
                result.distance = pbOutput.distance;
                result.sqrDistance = pbOutput.sqrDistance;
                result.closest = pbOutput.closest;
            }
            else
            {
                // The closest plane point is outside the rectangle, although
                // it is possible there are points inside the rectangle that
                // also are closest points to the box. Regardless, locate a
                // point on an edge of the rectangle that is closest to the
                // box. TODO: Will clamping work as is the case for distances
                // between line segments and objects?
                using SBQuery = DCPQuery<T, Segment3<T>, CanonicalBox3<T>>;
                SBQuery sbQuery{};
                typename SBQuery::Result sbOutput{};
                Segment3<T> segment{};

                T const one = static_cast<T>(1);
                T const negOne = static_cast<T>(-1);
                T const two = static_cast<T>(2);
                T const invalid = static_cast<T>(-1);
                result.distance = invalid;
                result.sqrDistance = invalid;

                std::array<T, 4> const sign{ negOne, one, negOne, one };
                std::array<int32_t, 4> j0{ 0, 0, 1, 1 };
                std::array<int32_t, 4> j1{ 1, 1, 0, 0 };
                std::array<std::array<size_t, 2>, 4> const edges
                {{
                    { 0, 1 }, { 2, 3 },  // horizontal edges
                    { 0, 2 }, { 1, 3 }   // vertical edges
                }};
                std::array<Vector3<T>, 4> vertices{};
                rectangle.GetVertices(vertices);

                for (size_t i = 0; i < 4; ++i)
                {
                    auto const& edge = edges[i];
                    segment.p[0] = vertices[edge[0]];
                    segment.p[1] = vertices[edge[1]];

                    sbOutput = sbQuery(segment, box);
                    if (result.sqrDistance == invalid ||
                        sbOutput.sqrDistance < result.sqrDistance)
                    {
                        result.distance = sbOutput.distance;
                        result.sqrDistance = sbOutput.sqrDistance;
                        result.closest = sbOutput.closest;

                        T const scale = two * sbOutput.parameter - one;
                        result.cartesian[j0[i]] = scale * rectangle.extent[j0[i]];
                        result.cartesian[j1[i]] = sign[i] * rectangle.extent[j1[i]];
                    }
                }
            }

            return result;
        }
    };
}
