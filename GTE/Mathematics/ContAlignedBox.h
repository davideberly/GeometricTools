// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/AlignedBox.h>
#include <algorithm>
#include <cstdint>

namespace gte
{
    // Compute the minimum size aligned bounding box of the points.  The
    // extreme values are the minima and maxima of the point coordinates.
    template <int32_t N, typename Real>
    bool GetContainer(int32_t numPoints, Vector<N, Real> const* points, AlignedBox<N, Real>& box)
    {
        return ComputeExtremes(numPoints, points, box.min, box.max);
    }

    // Test for containment.
    template <int32_t N, typename Real>
    bool InContainer(Vector<N, Real> const& point, AlignedBox<N, Real> const& box)
    {
        for (int32_t i = 0; i < N; ++i)
        {
            Real value = point[i];
            if (value < box.min[i] || value > box.max[i])
            {
                return false;
            }
        }
        return true;
    }

    // Construct an aligned box that contains two other aligned boxes.  The
    // result is the minimum size box containing the input boxes.
    template <int32_t N, typename Real>
    bool MergeContainers(AlignedBox<N, Real> const& box0,
        AlignedBox<N, Real> const& box1, AlignedBox<N, Real>& merge)
    {
        for (int32_t i = 0; i < N; ++i)
        {
            merge.min[i] = std::min(box0.min[i], box1.min[i]);
            merge.max[i] = std::max(box0.max[i], box1.max[i]);
        }
        return true;
    }
}
