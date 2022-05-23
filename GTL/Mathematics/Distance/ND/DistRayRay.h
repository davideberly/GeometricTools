// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between two rays in nD.
//
// The rays are P[i] + s[i] * D[i] for s[i] >= 0, where D[i] is not required
// to be unit length.
// 
// The closest point on ray[i] is stored in closest[i] with parameter[i]
// storing s[i]. When there are infinitely many choices for the pair of
// closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Ray.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Ray<T, N>, Ray<T, N>>
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

        Output operator()(Ray<T, N> const& ray0, Ray<T, N> const& ray1)
        {
            Output output{};

            Vector<T, N> diff = ray0.origin - ray1.origin;
            T a00 = Dot(ray0.direction, ray0.direction);
            T a01 = -Dot(ray0.direction, ray1.direction);
            T a11 = Dot(ray1.direction, ray1.direction);
            T b0 = Dot(ray0.direction, diff);
            T det = std::max(a00 * a11 - a01 * a01, C_<T>(0));
            T s0{}, s1{};

            if (det > C_<T>(0))
            {
                // The rays are not parallel.
                T b1 = -Dot(ray1.direction, diff);
                s0 = a01 * b1 - a11 * b0;
                s1 = a01 * b0 - a00 * b1;

                if (s0 >= C_<T>(0))
                {
                    if (s1 >= C_<T>(0))  // region 0 (interior)
                    {
                        // The minimum occurs at two interior points of
                        // the rays.
                        s0 /= det;
                        s1 /= det;
                    }
                    else  // region 3 (side)
                    {
                        if (b0 >= C_<T>(0))
                        {
                            s0 = C_<T>(0);
                        }
                        else
                        {
                            s0 = -b0 / a00;
                        }
                        s1 = C_<T>(0);
                    }
                }
                else
                {
                    if (s1 >= C_<T>(0))  // region 1 (side)
                    {
                        s0 = C_<T>(0);
                        if (b1 >= C_<T>(0))
                        {
                            s1 = C_<T>(0);
                        }
                        else
                        {
                            s1 = -b1 / a11;
                        }
                    }
                    else  // region 2 (corner)
                    {
                        if (b0 < C_<T>(0))
                        {
                            s0 = -b0 / a00;
                            s1 = C_<T>(0);
                        }
                        else
                        {
                            s0 = C_<T>(0);
                            if (b1 >= C_<T>(0))
                            {
                                s1 = C_<T>(0);
                            }
                            else
                            {
                                s1 = -b1 / a11;
                            }
                        }
                    }
                }
            }
            else
            {
                // The rays are parallel.
                if (a01 > C_<T>(0))
                {
                    // Opposite direction vectors.
                    s1 = C_<T>(0);
                    if (b0 >= C_<T>(0))
                    {
                        s0 = C_<T>(0);
                    }
                    else
                    {
                        s0 = -b0 / a00;
                    }
                }
                else
                {
                    // Same direction vectors.
                    if (b0 >= C_<T>(0))
                    {
                        T b1 = -Dot(ray1.direction, diff);
                        s0 = C_<T>(0);
                        s1 = -b1 / a11;
                    }
                    else
                    {
                        s0 = -b0 / a00;
                        s1 = C_<T>(0);
                    }
                }
            }

            output.parameter[0] = s0;
            output.parameter[1] = s1;
            output.closest[0] = ray0.origin + s0 * ray0.direction;
            output.closest[1] = ray1.origin + s1 * ray1.direction;
            diff = output.closest[0] - output.closest[1];
            output.sqrDistance = Dot(diff, diff);
            output.distance = std::sqrt(output.sqrDistance);

            return output;
        }
    };
}
