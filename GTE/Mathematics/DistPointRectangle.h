// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Rectangle.h>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Vector<N, T>, Rectangle<N, T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                cartesian{ static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector<N, T>::Zero(), Vector<N, T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 2> cartesian;
            std::array<Vector<N, T>, 2> closest;
        };

        Result operator()(Vector<N, T> const& point, Rectangle<N, T> const& rectangle)
        {
            Result result{};

            Vector<N, T> diff = point - rectangle.center;
            result.closest[0] = point;
            result.closest[1] = rectangle.center;
            for (int32_t i = 0; i < 2; ++i)
            {
                result.cartesian[i] = Dot(rectangle.axis[i], diff);
                if (result.cartesian[i] < -rectangle.extent[i])
                {
                    result.cartesian[i] = -rectangle.extent[i];
                }
                else if (result.cartesian[i] > rectangle.extent[i])
                {
                    result.cartesian[i] = rectangle.extent[i];
                }
                result.closest[1] += result.cartesian[i] * rectangle.axis[i];
            }

            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }
    };
}
