// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.5.2023.08.08

#pragma once

// Compute the smallest-area axis-aligned box containing an arc. Let the arc
// have endpoints E[0] and E[1] and live on a circle with center C and radius
// r. The extreme circle points in the axis directions are P[0] = C+(r,0),
// P[1] = C-(r,0), P[2] = C+(0,r) and P[3] = C-(0,r). The box is supported by
// the E0 and E1 and points P[i] that are on the arc.

#include <Mathematics/AlignedBox.h>
#include <Mathematics/Arc2.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <typename T>
    bool GetContainer(Arc2<T> const& arc, AlignedBox2<T>& box)
    {
        // Store the arc endpoints.
        int32_t numPoints = 2;
        std::array<Vector2<T>, 6> points{};
        points[0] = arc.end[0];
        points[1] = arc.end[1];

        // Store the circle points that are on the arc.
        points[numPoints] = { arc.center[0] + arc.radius, arc.center[1] };
        if (arc.Contains(points[numPoints]))
        {
            ++numPoints;
        }

        points[numPoints] = { arc.center[0] - arc.radius, arc.center[1] };
        if (arc.Contains(points[numPoints]))
        {
            ++numPoints;
        }

        points[numPoints] = { arc.center[0], arc.center[1] + arc.radius };
        if (arc.Contains(points[numPoints]))
        {
            ++numPoints;
        }

        points[numPoints] = { arc.center[0], arc.center[1] - arc.radius };
        if (arc.Contains(points[numPoints]))
        {
            ++numPoints;
        }

        if (0 < numPoints && numPoints <= 6)
        {
            // Compute the aligned bounding box.
            return ComputeExtremes(numPoints, points.data(), box.min, box.max);
        }

        // Code execution should never reach this point.
        return false;
    }
}
