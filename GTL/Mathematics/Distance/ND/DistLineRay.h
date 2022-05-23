// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a line and a ray in nD.
// 
// The line is P[0] + s[0] * D[0] and the ray is P[1] + s[1] * D[1] for
// s[1] >= 0. The D[i] are not required to be unit length.
// 
// The closest point on the line is stored in closest[0] with parameter[0]
// storing s[0]. The closest point on the ray is stored in closest[1] with
// parameter[1] storing s[1]. When there are infinitely many choices for the
// pair of closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Line.h>
#include <GTL/Mathematics/Primitives/ND/Ray.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Line<T, N>, Ray<T, N>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                parameter{ C_<T>(0), C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<T, 2> parameter;
            std::array<Vector<T, N>, 2> closest;
        };

        Output operator()(Line<T, N> const& line, Ray<T, N> const& ray)
        {
            Output output{};

            Vector<T, N> diff = line.origin - ray.origin;
            T a00 = Dot(line.direction, line.direction);
            T a01 = -Dot(line.direction, ray.direction);
            T a11 = Dot(ray.direction, ray.direction);
            T b0 = Dot(line.direction, diff);
            T det = std::max(a00 * a11 - a01 * a01, C_<T>(0));
            T s0{}, s1{};

            if (det > C_<T>(0))
            {
                // The line and ray are not parallel.
                T b1 = -Dot(ray.direction, diff);
                s1 = a01 * b0 - a00 * b1;

                if (s1 >= C_<T>(0))
                {
                    // Two interior points are closest, one on the line and
                    // one on the ray.
                    s0 = (a01 * b1 - a11 * b0) / det;
                    s1 /= det;
                }
                else
                {
                    // The origin of the ray is the closest ray point.
                    s0 = -b0 / a00;
                    s1 = C_<T>(0);
                }
            }
            else
            {
                // The line and ray are parallel. Select the pair of closest
                // points where the closest ray point is the ray origin.
                s0 = -b0 / a00;
                s1 = C_<T>(0);
            }

            output.parameter[0] = s0;
            output.parameter[1] = s1;
            output.closest[0] = line.origin + s0 * line.direction;
            output.closest[1] = ray.origin + s1 * ray.direction;
            diff = output.closest[0] - output.closest[1];
            output.sqrDistance = Dot(diff, diff);
            output.distance = std::sqrt(output.sqrDistance);
            return output;
        }
    };
}
