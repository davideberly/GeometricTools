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
    // sp0 == sp1
    template <typename T>
    struct SharedPtrEQ
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return (sp0 ? (sp1 ? *sp0 == *sp1 : false) : !sp1);
        }
    };

    // sp0 != sp1
    template <typename T>
    struct SharedPtrNE
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return !SharedPtrEQ<T>()(sp0, sp1);
        }
    };

    // sp0 < sp1
    template <typename T>
    struct SharedPtrLT
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return (sp1 ? (!sp0 || *sp0 < *sp1) : false);
        }
    };

    // sp0 <= sp1
    template <typename T>
    struct SharedPtrLE
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return !SharedPtrLT<T>()(sp1, sp0);
        }
    };

    // sp0 > sp1
    template <typename T>
    struct SharedPtrGT
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return SharedPtrLT<T>()(sp1, sp0);
        }
    };

    // sp0 >= sp1
    template <typename T>
    struct SharedPtrGE
    {
        bool operator()(std::shared_ptr<T> const& sp0, std::shared_ptr<T> const& sp1) const
        {
            return !SharedPtrLT<T>()(sp0, sp1);
        }
    };
}
