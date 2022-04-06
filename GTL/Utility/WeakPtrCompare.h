// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.04.06

#pragma once

// Documentation for this class is
// https://www.geometrictools.com/Documentation/GTLUtility.pdf#PointerComparison

#include <memory>

namespace gtl
{
    // wp0 == wp1
    template <typename T>
    struct WeakPtrEQ
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            auto sp0 = wp0.lock(), sp1 = wp1.lock();
            return (sp0 ? (sp1 ? *sp0 == *sp1 : false) : !sp1);
        }
    };

    // wp0 != wp1
    template <typename T>
    struct WeakPtrNE
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            return !WeakPtrEQ<T>()(wp0, wp1);
        }
    };

    // wp0 < wp1
    template <typename T>
    struct WeakPtrLT
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            auto sp0 = wp0.lock(), sp1 = wp1.lock();
            return (sp1 ? (!sp0 || *sp0 < *sp1) : false);
        }
    };

    // wp0 <= wp1
    template <typename T>
    struct WeakPtrLE
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            return !WeakPtrLT<T>()(wp1, wp0);
        }
    };

    // wp0 > wp1
    template <typename T>
    struct WeakPtrGT
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            return WeakPtrLT<T>()(wp1, wp0);
        }
    };

    // wp0 >= wp1
    template <typename T>
    struct WeakPtrGE
    {
        bool operator()(std::weak_ptr<T> const& wp0, std::weak_ptr<T> const& wp1) const
        {
            return !WeakPtrLT<T>()(wp0, wp1);
        }
    };
}
