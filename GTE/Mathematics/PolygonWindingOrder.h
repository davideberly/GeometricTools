// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

// Determine the winding order of a simple polygon. It is either
// counterclockwise (CCW) or clockwise (CW). If the polygon has
// one ordering, but you need the opposite ordering for your
// application, you have several choices.
//   1. Reverse the order of the elements:
//        std::reverse(polygon.begin(), polygon.end());
//      where std::reverse is defined in <algorithm>.
//   2. If you are managing the loop over the polygon elements, use
//        std::size_t const n = polygon.size();
//        for (std::size_t j = 0, i = n - 1; j < n; ++j, --i)
//          { <do your thing with the vertex polygon[i]> }
//      or
//        for (auto iter = polygon.begin(); iter != polygon.rend(); ++iter
//          { <do your thing with the vertex *iter> }
//      or
//        for (auto const& vertex : gte::reverse(polygon))
//          { <do your thing with the vertex }
//        where gte::reverse is defined in <Mathematics/RangeIteration.h>

#include <Mathematics/Vector2.h>
#include <vector>

namespace gte
{
    template <typename T>
    class PolygonWindingOrder
    {
    public:
        // The polygon vertices must be ordered, either CCW or CW. The
        // function returns true when the ordering is CCW or false when the
        // ordering is CW.
        bool operator()(std::vector<Vector2<T>> const& polygon) const
        {
            std::size_t const n = polygon.size();
            std::size_t lowerLeft = 0;
            for (std::size_t i = 1; i < n; ++i)
            {
                if (polygon[i] < polygon[lowerLeft])
                {
                    lowerLeft = i;
                }
            }

            Vector2<T> const& vLowerLeft = polygon[lowerLeft];
            Vector2<T> const& vNext = polygon[(lowerLeft + 1) % n];
            Vector2<T> const& vPrev = polygon[(lowerLeft + n - 1) % n];
            Vector2<T> diffNext = vNext - vLowerLeft;
            Vector2<T> diffPrev = vPrev - vLowerLeft;
            T dotPerp = DotPerp(diffNext, diffPrev);
            return dotPerp > static_cast<T>(0);
        }
    };
}


