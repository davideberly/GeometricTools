// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.2.2024.10.21

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Parallelogram2.h>
#include <array>
#include <cmath>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Vector2<T>, Parallelogram2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                closest{ Vector2<T>::Zero(), Vector2<T>::Zero() }
            {
            }

            // The point closest[0] is the query point. The point closest[1]
            // is the parallelogram point closest to the query point. The
            // two points are the same when the query point is contained by
            // the parallelogram.
            T distance, sqrDistance;
            std::array<Vector2<T>, 2> closest;
        };

        Result operator()(Vector2<T> const& point, Parallelogram2<T> const& pgm)
        {
            Result result{};

            // For a parallelogram point X, let Y = {Dot(V0,X-C),Dot(V1,X-C)}.
            // Compute the quadratic function q(Y) = (Y-Z)^T * A * (Y-Z) / 2
            // where A = B^T * B is a symmetric matrix.
            T b00 = pgm.axis[0][0];
            T b10 = pgm.axis[0][1];
            T b01 = pgm.axis[1][0];
            T b11 = pgm.axis[1][1];
            T detB = b00 * b11 - b01 * b10;
            T a00 = b00 * b00 + b10 * b10;
            T a01 = b00 * b01 + b10 * b11;
            T a11 = b01 * b01 + b11 * b11;

            // Transform the query point to parallelogram coordinates,
            // Z = Inverse(B) * (P - C).
            Vector2<T> diff = point - pgm.center;
            Vector2<T> Z
            {
                (b11 * diff[0] - b01 * diff[1]) / detB,
                (b00 * diff[1] - b10 * diff[0]) / detB
            };

            // Determine the region containing Z. The point K is the closest
            // point to Z relative to the metric tensor A.
            T const negOne = static_cast<T>(-1);
            T const posOne = static_cast<T>(+1);
            T root{};
            Vector2<T> K{};

            if (Z[1] < negOne)
            {
                // Examine the bottom edge.
                root = Z[0] - a01 * (negOne - Z[1]) / a00;
                K[0] = Clamp(root, negOne, posOne);
                K[1] = negOne;

                if (Z[0] < negOne)
                {
                    if (K[0] == negOne)
                    {
                        // Examine the left edge.
                        root = Z[1] - a01 * (negOne - Z[0]) / a11;
                        K[1] = Clamp(root, negOne, posOne);
                        K[0] = negOne;
                    }
                }
                else if (posOne < Z[0])
                {
                    if (K[0] == posOne)
                    {
                        // Examine the right edge.
                        root = Z[1] - a01 * (posOne - Z[0]) / a11;
                        K[1] = Clamp(root, negOne, posOne);
                        K[0] = posOne;
                    }
                }
            }
            else if (Z[1] <= posOne)
            {
                if (Z[0] < negOne)
                {
                    // Examine the left edge.
                    root = Z[1] - a01 * (negOne - Z[0]) / a11;
                    K[1] = Clamp(root, negOne, posOne);
                    K[0] = negOne;
                }
                else if (Z[0] <= posOne)
                {
                    // The query point is inside the parallelogram.
                    K = Z;
                }
                else
                {
                    // Examine the right edge.
                    root = Z[1] - a01 * (posOne - Z[0]) / a11;
                    K[1] = Clamp(root, negOne, posOne);
                    K[0] = posOne;
                }
            }
            else
            {
                // Examine the top edge.
                root = Z[0] - a01 * (posOne - Z[1]) / a00;
                K[0] = Clamp(root, negOne, posOne);
                K[1] = posOne;

                if (Z[0] < negOne)
                {
                    if (K[0] == negOne)
                    {
                        // Examine the left edge.
                        root = Z[1] - a01 * (negOne - Z[0]) / a11;
                        K[1] = Clamp(root, negOne, posOne);
                        K[0] = negOne;
                    }
                }
                else if (posOne < Z[0])
                {
                    if (K[0] == posOne)
                    {
                        // Examine the right edge.
                        root = Z[1] - a01 * (posOne - Z[0]) / a11;
                        K[1] = Clamp(root, negOne, posOne);
                        K[0] = posOne;
                    }
                }
            }

            result.closest[0] = point;
            result.closest[1] = pgm.center + K[0] * pgm.axis[0] + K[1] * pgm.axis[1];
            diff = result.closest[0] - result.closest[1];
            result.sqrDistance = diff[0] * diff[0] + diff[1] * diff[1];
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

    private:
        inline static T Clamp(T const& u, T const& umin, T const& umax)
        {
            return (u <= umin ? umin : (u >= umax ? umax : u));
        }
    };
}
