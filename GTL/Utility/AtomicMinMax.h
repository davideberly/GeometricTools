// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.04.06

#pragma once

// Documentation for this class is
// https://www.geometrictools.com/Documentation/GTLUtility.pdf#AtomicMinMax

#include <algorithm>
#include <atomic>

namespace gtl
{
    template <typename T>
    T AtomicMin(std::atomic<T>& v0, T const& v1)
    {
        T vInitial, vMin;
        do
        {
            vInitial = v0;
            vMin = std::min(vInitial, v1);
        }
        while (!std::atomic_compare_exchange_weak(&v0, &vInitial, vMin));

        // On return, v0 = min(v0, v1) and vInitial is the original value of
        // v0 that was passed to the function.
        return vInitial;
    }

    template <typename T>
    T AtomicMax(std::atomic<T>& v0, T const& v1)
    {
        T vInitial, vMax;
        do
        {
            vInitial = v0;
            vMax = std::max(vInitial, v1);
        }
        while (!std::atomic_compare_exchange_weak(&v0, &vInitial, vMax));

        // On return, v0 = max(v0, v1) and vInitial is the original value of
        // v0 that was passed to the function.
        return vInitial;
    }
}
