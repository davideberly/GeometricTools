// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Type traits to support std::enable_if conditional compilation for
// numerical computations.

#include <cstddef>
#include <type_traits>

namespace gte
{
    // The is_arbitrary_precision<T>::value for type T of float, double or
    // long double is 'false'. The value is 'true' for BSNumber, BSRational
    // and QFNumber, all implemented in their header files.
    template <typename T>
    struct _is_arbitrary_precision_internal : std::false_type {};

    template <typename T>
    struct is_arbitrary_precision : _is_arbitrary_precision_internal<T>::type {};

    // The trait has_division_operator<T> for type T of float, double or
    // long double generates has_division_operator<T>::value of true. The
    // implementations for arbitrary-precision arithmetic are found in
    // ArbitraryPrecision.h.
    template <typename T>
    struct _has_division_operator_internal : std::false_type {};

    template <typename T>
    struct has_division_operator : _has_division_operator_internal<std::remove_cv_t<T>>::type {};

    template <>
    struct _has_division_operator_internal<float> : std::true_type {};

    template <>
    struct _has_division_operator_internal<double> : std::true_type {};

    template <>
    struct _has_division_operator_internal<long double> : std::true_type {};

    // Template alias for template-based conditional compilation (SFINAE)
    // in classes with template parameters 'Parameters, bool Condition'.
    // Example usage is
    //   template <Parameters, bool _Condition = Condition,
    //       TraitSelector<_Condition> = 0>
    //   ReturnType MemberFunction(inputs)
    //       { implementation for Condition = true; }
    //
    //   template <Parameters, bool _Condition = Condition,
    //       TraitSelector<!_Condition> = 0>
    //   ReturnType MemberFunction(inputs)
    //       { implementation for Condition = false; }
    template <bool condition>
    using TraitSelector = std::enable_if_t<condition, size_t>;

    // Template aliases for template-based conditional compilation (SFINAE)
    // in classes having a numeric template parameter. The selection is based
    // on whether the numeric type is floating-point or arbitrary precision.
    // Example usage is
    //   template <typename Numeric, IsFPType<Numeric> = 0>
    //   Numeric MemberFunction(Numeric inputs)
    //       { floating-point computations }
    //
    //   template <typename Numeric, IsAPType<Numeric> = 0>
    //   Numeric MemberFunction(Numeric inputs)
    //       { arbitrary-precision computations }
    template <typename T>
    using IsFPType = std::enable_if_t<!is_arbitrary_precision<T>::value, size_t>;

    template <typename T>
    using IsAPType = std::enable_if_t<is_arbitrary_precision<T>::value, size_t>;

    // Template aliases for template-based conditional compilation (SFINAE)
    // in classes having a numeric template parameter. The selection is based
    // on whether or not the numeric type supports division.
    // Example usage is
    //   template <typename Numeric, IsDivisionType<Numeric> = 0>
    //   Numeric MemberFunction(Numeric inputs)
    //       { Numeric computations that use divisions }
    //
    //   template <typename Numeric, IsNotDivisionType<Numeric> = 0>
    //   Numeric MemberFunction(Numeric inputs)
    //       { Numeric computations without divisions }
    // The prototypical examples are when Numeric is BSNumber<*> (does not
    // support division) or when Numeric is BSRational<*> (supports division).
    template <typename T>
    using IsDivisionType = std::enable_if_t<has_division_operator<T>::value, size_t>;

    template <typename T>
    using IsNotDivisionType = std::enable_if_t<!has_division_operator<T>::value, size_t>;
}
