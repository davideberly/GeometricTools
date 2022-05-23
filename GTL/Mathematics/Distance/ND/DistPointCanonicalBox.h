// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance from a point to a solid canonical box in nD.
// 
// The canonical box has center at the origin and is aligned with the
// coordinate axes. The extents are E = (e[0],e[1],...,e[n-1]). A box
// point is Y = (y[0],y[1],...,y[n-1]) with |y[i]| <= e[i] for all i.
// 
// The input point P is stored in closest[0]. The closest point on the box
// is stored in closest[1]. When there are infinitely many choices for the
// pair of closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/CanonicalBox.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Vector<T, N>, CanonicalBox<T, N>>
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
            std::array<Vector<T, N>, 2> closest;
        };

        Output operator()(Vector<T, N> const& point, CanonicalBox<T, N> const& box)
        {
            Output output{};

            output.closest[0] = point;
            output.closest[1] = point;
            output.sqrDistance = C_<T>(0);
            for (size_t i = 0; i < N; ++i)
            {
                if (point[i] < -box.extent[i])
                {
                    T delta = output.closest[1][i] + box.extent[i];
                    output.sqrDistance += delta * delta;
                    output.closest[1][i] = -box.extent[i];
                }
                else if (point[i] > box.extent[i])
                {
                    T delta = output.closest[1][i] - box.extent[i];
                    output.sqrDistance += delta * delta;
                    output.closest[1][i] = box.extent[i];
                }
            }
            output.distance = std::sqrt(output.sqrDistance);

            return output;
        }
    };
}
