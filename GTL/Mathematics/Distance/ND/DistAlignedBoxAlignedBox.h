// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/AlignedBox.h>
#include <GTL/Mathematics/Intersection/IntrIntervals.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, AlignedBox<T, N>, AlignedBox<T, N>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<AlignedBox<T, N>, 2> closest;
        };

        Output operator()(AlignedBox<T, N> const& box0, AlignedBox<T, N> const& box1)
        {
            Output output{};

            output.sqrDistance = C_<T>(0);
            for (size_t i = 0; i < N; ++i)
            {
                if (box0.min[i] > box1.max[i])
                {
                    T delta = box0.min[i] - box1.max[i];
                    output.sqrDistance += delta * delta;
                    output.closest[0].min[i] = box0.min[i];
                    output.closest[0].max[i] = box0.min[i];
                    output.closest[1].min[i] = box1.max[i];
                    output.closest[1].max[i] = box1.max[i];
                }
                else if (box1.min[i] > box0.max[i])
                {
                    T delta = box1.min[i] - box0.max[i];
                    output.sqrDistance += delta * delta;
                    output.closest[0].min[i] = box0.max[i];
                    output.closest[0].max[i] = box0.max[i];
                    output.closest[1].min[i] = box1.min[i];
                    output.closest[1].max[i] = box1.min[i];
                }
                else  // box0.min[i] <= box1.max[i] and box1.min[i] <= box0.max[i]
                {
                    std::array<T, 2> interval0 = { box0.min[i], box0.max[i] };
                    std::array<T, 2> interval1 = { box1.min[i], box1.max[i] };
                    FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                    auto iiOutput = iiQuery(interval0, interval1);
                    for (size_t j = 0; j < 2; ++j)
                    {
                        output.closest[j].min[i] = iiOutput.overlap[0];
                        output.closest[j].max[i] = iiOutput.overlap[1];
                    }
                }
            }
            output.distance = std::sqrt(output.sqrDistance);

            return output;
        }
    };
}
