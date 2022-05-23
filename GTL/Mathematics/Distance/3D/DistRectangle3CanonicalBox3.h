// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/3D/DistPlane3CanonicalBox3.h>
#include <GTL/Mathematics/Distance/3D/DistSegment3CanonicalBox3.h>
#include <GTL/Mathematics/Primitives/ND/Rectangle.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Rectangle3<T>, CanonicalBox3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                cartesian{ C_<T>(0), C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<T, 2> cartesian;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Rectangle3<T> const& rectangle, CanonicalBox3<T> const& box)
        {
            Output output{};

            using PBQuery = DCPQuery<T, Plane3<T>, CanonicalBox3<T>>;
            PBQuery pbQuery{};
            Vector3<T> normal = Cross(rectangle.axis[0], rectangle.axis[1]);
            Plane3<T> plane(normal, rectangle.center);
            auto pbOutput = pbQuery(plane, box);
            Vector3<T> delta = pbOutput.closest[0] - rectangle.center;
            output.cartesian[0] = Dot(rectangle.axis[0], delta);
            output.cartesian[1] = Dot(rectangle.axis[1], delta);

            if (std::fabs(output.cartesian[0]) <= rectangle.extent[0] &&
                std::fabs(output.cartesian[1]) <= rectangle.extent[1])
            {
                output.distance = pbOutput.distance;
                output.sqrDistance = pbOutput.sqrDistance;
                output.closest = pbOutput.closest;
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
                typename SBQuery::Output sbOutput{};
                Segment3<T> segment{};

                T const invalid = -C_<T>(1);
                output.distance = invalid;
                output.sqrDistance = invalid;

                std::array<T, 4> const sign{ -C_<T>(1), C_<T>(1), -C_<T>(1), C_<T>(1) };
                std::array<size_t, 4> j0{ 0, 0, 1, 1 };
                std::array<size_t, 4> j1{ 1, 1, 0, 0 };
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
                    if (output.sqrDistance == invalid ||
                        sbOutput.sqrDistance < output.sqrDistance)
                    {
                        output.distance = sbOutput.distance;
                        output.sqrDistance = sbOutput.sqrDistance;
                        output.closest = sbOutput.closest;

                        T const scale = C_<T>(2) * sbOutput.parameter - C_<T>(1);
                        output.cartesian[j0[i]] = scale * rectangle.extent[j0[i]];
                        output.cartesian[j1[i]] = sign[i] * rectangle.extent[j1[i]];
                    }
                }
            }

            return output;
        }
    };
}
