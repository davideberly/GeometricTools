// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between two solid aligned boxes in nD.
// 
// Each aligned box has minimum corner A and maximum corner B. A box point is
// X where A <= X <= B; the comparisons are componentwise.
//
// The algorithm computes two aligned boxes of closest points, closest[0] for
// input box0 and closest[1] for input box1. Any choice of P0 in closest[0]
// and any choice of P1 in closest[1] form a pair (P0,P1) of closest points.
// One reasonable choise is
//   Vector<T, N> P0 = (closest[0].min + closest[0].max)/2;
//   Vector<T, N> P1 = (closest[1].min + closest[1].max)/2;

#include <Mathematics/DCPQuery.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/IntrIntervals.h>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, AlignedBox<N, T>, AlignedBox<N, T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<AlignedBox<N, T>, 2> closest;
        };

        Result operator()(AlignedBox<N, T> const& box0, AlignedBox<N, T> const& box1)
        {
            Result result{};

            result.sqrDistance = static_cast<T>(0);
            for (int32_t i = 0; i < N; ++i)
            {
                if (box0.min[i] >= box1.max[i])
                {
                    T delta = box0.min[i] - box1.max[i];
                    result.sqrDistance += delta * delta;
                    result.closest[0].min[i] = box0.min[i];
                    result.closest[0].max[i] = box0.min[i];
                    result.closest[1].min[i] = box1.max[i];
                    result.closest[1].max[i] = box1.max[i];
                }
                else if (box1.min[i] >= box0.max[i])
                {
                    T delta = box1.min[i] - box0.max[i];
                    result.sqrDistance += delta * delta;
                    result.closest[0].min[i] = box0.max[i];
                    result.closest[0].max[i] = box0.max[i];
                    result.closest[1].min[i] = box1.min[i];
                    result.closest[1].max[i] = box1.min[i];
                }
                else  // box0.min[i] <= box1.max[i] and box1.min[i] <= box0.max[i]
                {
                    std::array<T, 2> interval0 = { box0.min[i], box0.max[i] };
                    std::array<T, 2> interval1 = { box1.min[i], box1.max[i] };
                    FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                    auto iiResult = iiQuery(interval0, interval1);
                    for (int32_t j = 0; j < 2; ++j)
                    {
                        result.closest[j].min[i] = iiResult.overlap[0];
                        result.closest[j].max[i] = iiResult.overlap[1];
                    }
                }
            }
            result.distance = std::sqrt(result.sqrDistance);

            return result;
        }
    };
}
