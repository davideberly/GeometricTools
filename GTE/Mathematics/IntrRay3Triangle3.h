// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Ray.h>
#include <Mathematics/Triangle.h>
#include <Mathematics/Vector3.h>
#include <array>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray3<T>, Triangle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Ray3<T> const& ray, Triangle3<T> const& triangle)
        {
            Result result{};

            // Compute the offset origin, edges, and normal.
            Vector3<T> diff = ray.origin - triangle.v[0];
            Vector3<T> edge1 = triangle.v[1] - triangle.v[0];
            Vector3<T> edge2 = triangle.v[2] - triangle.v[0];
            Vector3<T> normal = Cross(edge1, edge2);

            // Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = ray direction,
            // E1 = edge1, E2 = edge2, N = Cross(E1,E2)) by
            //   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
            //   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
            //   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
            T DdN = Dot(ray.direction, normal);
            T sign = (T)0;
            if (DdN > (T)0)
            {
                sign = (T)1;
            }
            else if (DdN < (T)0)
            {
                sign = (T)-1;
                DdN = -DdN;
            }
            else
            {
                // Ray and triangle are parallel, call it a "no intersection"
                // even if the ray does intersect.
                result.intersect = false;
                return result;
            }

            T DdQxE2 = sign * DotCross(ray.direction, diff, edge2);
            if (DdQxE2 >= (T)0)
            {
                T DdE1xQ = sign * DotCross(ray.direction, edge1, diff);
                if (DdE1xQ >= (T)0)
                {
                    if (DdQxE2 + DdE1xQ <= DdN)
                    {
                        // Line intersects triangle, check whether ray does.
                        T QdN = -sign * Dot(diff, normal);
                        if (QdN >= (T)0)
                        {
                            // Ray intersects triangle.
                            result.intersect = true;
                            return result;
                        }
                        // else: t < 0, no intersection
                    }
                    // else: b1+b2 > 1, no intersection
                }
                // else: b2 < 0, no intersection
            }
            // else: b1 < 0, no intersection

            result.intersect = false;
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Ray3<T>, Triangle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                parameter((T)0),
                triangleBary{ (T)0, (T)0, (T)0 },
                point(Vector3<T>::Zero())
            {
            }

            bool intersect;
            T parameter;
            std::array<T, 3> triangleBary;
            Vector3<T> point;
        };

        Result operator()(Ray3<T> const& ray, Triangle3<T> const& triangle)
        {
            Result result{};

            // Compute the offset origin, edges, and normal.
            Vector3<T> diff = ray.origin - triangle.v[0];
            Vector3<T> edge1 = triangle.v[1] - triangle.v[0];
            Vector3<T> edge2 = triangle.v[2] - triangle.v[0];
            Vector3<T> normal = Cross(edge1, edge2);

            // Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = ray direction,
            // E1 = edge1, E2 = edge2, N = Cross(E1,E2)) by
            //   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
            //   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
            //   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
            T DdN = Dot(ray.direction, normal);
            T sign = (T)0;
            if (DdN > (T)0)
            {
                sign = (T)1;
            }
            else if (DdN < (T)0)
            {
                sign = (T)-1;
                DdN = -DdN;
            }
            else
            {
                // Ray and triangle are parallel, call it a "no intersection"
                // even if the ray does intersect.
                result.intersect = false;
                return result;
            }

            T DdQxE2 = sign * DotCross(ray.direction, diff, edge2);
            if (DdQxE2 >= (T)0)
            {
                T DdE1xQ = sign * DotCross(ray.direction, edge1, diff);
                if (DdE1xQ >= (T)0)
                {
                    if (DdQxE2 + DdE1xQ <= DdN)
                    {
                        // Line intersects triangle, check whether ray does.
                        T QdN = -sign * Dot(diff, normal);
                        if (QdN >= (T)0)
                        {
                            // Ray intersects triangle.
                            result.intersect = true;
                            result.parameter = QdN / DdN;
                            result.triangleBary[1] = DdQxE2 / DdN;
                            result.triangleBary[2] = DdE1xQ / DdN;
                            result.triangleBary[0] =
                                (T)1 - result.triangleBary[1] - result.triangleBary[2];
                            result.point = ray.origin + result.parameter * ray.direction;
                            return result;
                        }
                        // else: t < 0, no intersection
                    }
                    // else: b1+b2 > 1, no intersection
                }
                // else: b2 < 0, no intersection
            }
            // else: b1 < 0, no intersection

            result.intersect = false;
            return result;
        }
    };
}
