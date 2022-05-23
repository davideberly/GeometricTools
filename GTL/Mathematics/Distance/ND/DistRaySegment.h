// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a ray and a segment in nD.
// 
// The ray is P[0] + s[0] * D[0] for s[0] >= 0. D[0] is not required to be
// unit length. 
// 
// The segment is Q[0] + s[1] * (Q[1] - Q[0]) for 0 <= s[1 <= 1. The
// direction D = Q[1] - Q[0] is generally not unit length.
// 
// The closest point on the ray is stored in closest[0] with parameter[0]
// storing s[0]. The closest point on the segment is stored in closest[1] with
// parameter[1] storing s[1]. When there are infinitely many choices for the
// pair of closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Ray.h>
#include <GTL/Mathematics/Primitives/ND/Segment.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Ray<T, N>, Segment<T, N>>
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

        Output operator()(Ray<T, N> const& ray, Segment<T, N> const& segment)
        {
            Output output{};

            Vector<T, N> segDirection = segment.p[1] - segment.p[0];
            Vector<T, N> diff = ray.origin - segment.p[0];
            T a00 = Dot(ray.direction, ray.direction);
            T a01 = -Dot(ray.direction, segDirection);
            T a11 = Dot(segDirection, segDirection);
            T b0 = Dot(ray.direction, diff);
            T det = std::max(a00 * a11 - a01 * a01, C_<T>(0));
            T s0{}, s1{};

            if (det > C_<T>(0))
            {
                // The ray and segment are not parallel.
                T b1 = -Dot(segDirection, diff);
                s0 = a01 * b1 - a11 * b0;
                s1 = a01 * b0 - a00 * b1;

                if (s0 >= C_<T>(0))
                {
                    if (s1 >= C_<T>(0))
                    {
                        if (s1 <= det)  // region 0
                        {
                            // The minimum occurs at interior points of the
                            // ray and the segment.
                            s0 /= det;
                            s1 /= det;
                        }
                        else  // region 1
                        {
                            // The endpoint Q1 of the segment and an interior
                            // point of the line are closest.
                            s0 = -(a01 + b0) / a00;
                            s1 = C_<T>(1);
                        }
                    }
                    else  // region 5
                    {
                        // The endpoint Q0 of the segment and an interior
                        // point of the line are closest.
                        s0 = -b0 / a00;
                        s1 = C_<T>(0);
                    }
                }
                else  // s0 < 0
                {
                    if (s1 <= C_<T>(0))  // region 4
                    {
                        s0 = -b0;
                        if (s0 > C_<T>(0))
                        {
                            s0 /= a00;
                            s1 = C_<T>(0);
                        }
                        else
                        {
                            s0 = C_<T>(0);
                            s1 = -b1;
                            if (s1 < C_<T>(0))
                            {
                                s1 = C_<T>(0);
                            }
                            else if (s1 > a11)
                            {
                                s1 = C_<T>(1);
                            }
                            else
                            {
                                s1 /= a11;
                            }
                        }
                    }
                    else if (s1 <= det)  // region 3
                    {
                        s0 = C_<T>(0);
                        s1 = -b1;
                        if (s1 < C_<T>(0))
                        {
                            s1 = C_<T>(0);
                        }
                        else if (s1 > a11)
                        {
                            s1 = C_<T>(1);
                        }
                        else
                        {
                            s1 /= a11;
                        }
                    }
                    else  // region 2
                    {
                        s0 = -(a01 + b0);
                        if (s0 > C_<T>(0))
                        {
                            s0 /= a00;
                            s1 = C_<T>(1);
                        }
                        else
                        {
                            s0 = C_<T>(0);
                            s1 = -b1;
                            if (s1 < C_<T>(0))
                            {
                                s1 = C_<T>(0);
                            }
                            else if (s1 > a11)
                            {
                                s1 = C_<T>(1);
                            }
                            else
                            {
                                s1 /= a11;
                            }
                        }
                    }
                }
            }
            else
            {
                // The ray and segment are parallel.
                if (a01 > C_<T>(0))
                {
                    // Opposite direction vectors.
                    s0 = -b0 / a00;
                    s1 = C_<T>(0);
                }
                else
                {
                    // Same direction vectors.
                    s0 = -(a01 + b0) / a00;
                    s1 = C_<T>(1);
                }
            }

            output.parameter[0] = s0;
            output.parameter[1] = s1;
            output.closest[0] = ray.origin + s0 * ray.direction;
            output.closest[1] = segment.p[0] + s1 * segDirection;
            diff = output.closest[0] - output.closest[1];
            output.sqrDistance = Dot(diff, diff);
            output.distance = std::sqrt(output.sqrDistance);
            return output;
        }
    };
}
