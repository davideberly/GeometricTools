// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a point and a rectangle in nD.
// 
// The rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
// 
// The input point is stored in closest[0]. The closest point on the
// rectangle is stored in closest[1] with W-coordinates (s[0],s[1]). When
// there are infinitely many choices for the pair of closest points, only one
// of them is returned.
//
// TODO: Modify to support non-unit-length W[].

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Rectangle.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Vector<T, N>, Rectangle<T, N>>
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
            std::array<Vector<T, N>, 2> closest;
        };

        Output operator()(Vector<T, N> const& point, Rectangle<T, N> const& rectangle)
        {
            Output output{};

            Vector<T, N> diff = point - rectangle.center;
            output.closest[0] = point;
            output.closest[1] = rectangle.center;
            for (size_t i = 0; i < 2; ++i)
            {
                output.cartesian[i] = Dot(rectangle.axis[i], diff);
                if (output.cartesian[i] < -rectangle.extent[i])
                {
                    output.cartesian[i] = -rectangle.extent[i];
                }
                else if (output.cartesian[i] > rectangle.extent[i])
                {
                    output.cartesian[i] = rectangle.extent[i];
                }
                output.closest[1] += output.cartesian[i] * rectangle.axis[i];
            }

            diff = output.closest[0] - output.closest[1];
            output.sqrDistance = Dot(diff, diff);
            output.distance = std::sqrt(output.sqrDistance);
            return output;
        }
    };
}
