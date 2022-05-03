// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Documentation for this class is
// https://www.geometrictools.com/Documentation/GTLUtility.pdf#Lattice

#include <GTL/Utility/Exceptions.h>
#include <GTL/Utility/TypeTraits.h>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <tuple>
#include <type_traits>
#include <vector>

// Implementation for a lattice whose sizes are known at compile time.
// The class has no data members. Its member functions use template
// metaprogramming for mapping indices to and from multiindices. The
// Sizes parameter pack has n >= 1 elements and represents the bounds
// (b[0],...,b[n-1]).
namespace gtl
{
    template <bool OrderLtoR, size_t... Sizes>
    class Lattice
    {
    public:
        Lattice()
        {
            static_assert(
                sizeof...(Sizes) >= 1,
                "At least one dimension is required.");
        }

        ~Lattice() = default;

        // The number of dimensions is the number of arguments in the Sizes
        // parameter pack. This is 'n' in the comments about lattices.
        inline size_t constexpr dimensions() const noexcept
        {
            return sizeof...(Sizes);
        }

        // Get the number of elements for dimension d. This is 'b[d]' in the
        // comments about lattices.
        inline size_t constexpr size(size_t d) const noexcept
        {
            std::array<size_t, sizeof...(Sizes)> sizes{};
            MetaAssignSize<0, Sizes...>(sizes.data());
            return sizes[d];
        }

        // Get the number of elements. This is 'product{d=0}^{n-1} b[d]' in
        // the comments about lattices.
        inline size_t constexpr size() const noexcept
        {
            return MetaProduct<Sizes...>::value;
        }

        // *** Conversions for left-to-right ordering.

        // Convert from an n-dimensional index to a 1-dimensional index for
        // left-to-right ordering.
        template <typename... IndexTypes, bool _OrderLtoR = OrderLtoR, TraitSelector<_OrderLtoR> = 0>
        size_t index(IndexTypes... ntuple) const noexcept
        {
            static_assert(
                sizeof...(IndexTypes) == sizeof...(Sizes),
                "Invalid number of arguments.");

            return MetaGetIndexLtoR(ntuple...);
        }

        template <typename... IndexTypes, bool _OrderLtoR = OrderLtoR, TraitSelector<_OrderLtoR> = 0>
        size_t index(std::array<size_t, sizeof...(Sizes)> const& coordinate) const noexcept
        {
            return GetIndexLtoR(coordinate);
        }

        // Convert from a 1-dimension index to an n-dimensional index for
        // left-to-right ordering.
        template <typename... IndexTypes, bool _OrderLtoR = OrderLtoR, TraitSelector<_OrderLtoR> = 0>
        std::array<size_t, sizeof...(Sizes)> coordinate(size_t i) const noexcept
        {
            // i = x[0] + b[0] * (x[1] + b[1] * (x[2] + ...)
            // tuple = (x[0], ..., x[n-1])
            std::array<size_t, sizeof...(Sizes)> tuple{};
            for (size_t d = 0; d < sizeof...(Sizes); ++d)
            {
                size_t const bound = size(d), j = i;
                i /= bound;
                tuple[d] = j - bound * i;
            }
            return tuple;
        }

        // *** Conversions for right-to-left ordering.

        // Convert from an n-dimensional index to a 1-dimensional index for
        // right-to-left ordering.
        template <typename... IndexTypes, bool _OrderLtoR = OrderLtoR, TraitSelector<!_OrderLtoR> = 0>
        size_t index(IndexTypes... ntuple) const noexcept
        {
            static_assert(
                sizeof...(IndexTypes) == sizeof...(Sizes),
                "Invalid number of arguments.");

            using Type = typename std::tuple_element<0, std::tuple<IndexTypes...>>::type;
            return MetaGetIndexRtoL(static_cast<Type>(0), ntuple...);
        }

        template <typename... IndexTypes, bool _OrderLtoR = OrderLtoR, TraitSelector<!_OrderLtoR> = 0>
        size_t index(std::array<size_t, sizeof...(Sizes)> const& coordinate) const noexcept
        {
            return GetIndexRtoL(coordinate);
        }

        // Convert from a 1-dimension index to an n-dimensional index for
        // right-to-left ordering.
        template <typename... IndexTypes, bool _OrderLtoR = OrderLtoR, TraitSelector<!_OrderLtoR> = 0>
        std::array<size_t, sizeof...(Sizes)> coordinate(size_t i) const noexcept
        {
            // i = x[n-1] + b[n-1] * (x[n-2] + b[n-2] * (x[n-3] + ...)
            // tuple = (x[0], ..., x[n-1])
            std::array<size_t, sizeof...(Sizes)> tuple{};
            for (size_t k = 0, d = sizeof...(Sizes) - 1; k < sizeof...(Sizes); ++k, --d)
            {
                size_t const bound = size(d), j = i;
                i /= bound;
                tuple[d] = j - bound * i;
            }
            return tuple;
        }

    private:
        // Metaprogramming support for selecting an argument from the
        // Sizes parameter pack.
        template <size_t i, size_t f, size_t... r>
        struct MetaArgument
        {
            static size_t const value = MetaArgument<i - 1, r...>::value;
        };

        template <size_t f, size_t... r>
        struct MetaArgument<0, f, r...>
        {
            static size_t const value = f;
        };

        // Metaprogramming support for assigning the Sizes parameter pack
        // arguments to an array.
        template <size_t i, size_t f, size_t... r>
        void constexpr MetaAssignSize(size_t* sizes) const noexcept
        {
            sizes[i] = f;
            MetaAssignSize<i + 1, r...>(sizes);
        }

        template <size_t numElements>
        void constexpr MetaAssignSize(size_t*) const noexcept
        {
        }

        // Metaprogramming support for computing the product of the Sizes
        // parameter pack arguments.
        template <size_t...>
        struct MetaProduct
            :
            std::integral_constant<size_t, 1>
        {
        };

        template <size_t f, size_t... r>
        struct MetaProduct<f, r...>
            :
            std::integral_constant<size_t, f* MetaProduct<r...>::value>
        {
        };

        // Metaprogramming support for index(IndexTypes...) using
        // left-to-right ordering.
        template <typename First, typename... Successors>
        size_t constexpr MetaGetIndexLtoR(First first, Successors... successors) const noexcept
        {
            static_assert(
                std::is_integral<First>::value && !std::is_same<First, bool>::value,
                "Arguments must be integer type.");

            size_t constexpr size = MetaArgument<sizeof...(Sizes) - 1 - sizeof...(Successors), Sizes...>::value;
            return static_cast<size_t>(first) + size * MetaGetIndexLtoR(successors...);
        }

        template <typename Last>
        size_t constexpr MetaGetIndexLtoR(Last last) const noexcept
        {
            static_assert(
                std::is_integral<Last>::value && !std::is_same<Last, bool>::value,
                "Arguments must be integer type.");

            return static_cast<size_t>(last);
        }

        // Metaprogramming support for index(std::array<*,*)> const&) using
        // left-to-right ordering.
        template <size_t numDimensions = sizeof...(Sizes), TraitSelector<(numDimensions > 1)> = 0>
        size_t GetIndexLtoR(std::array<size_t, sizeof...(Sizes)> const& coordinate) const noexcept
        {
            std::array<size_t, sizeof...(Sizes)> sizes{};
            MetaAssignSize<0, Sizes...>(sizes.data());
            size_t d = coordinate.size() - 1;
            size_t indexLtoR = coordinate[d];
            --d;
            for (size_t k = 1; k < sizeof...(Sizes); ++k, --d)
            {
                indexLtoR = sizes[d] * indexLtoR + coordinate[d];
            }
            return indexLtoR;
        }

        template <size_t numDimensions = sizeof...(Sizes), TraitSelector<numDimensions == 1> = 0>
        size_t GetIndexLtoR(std::array<size_t, 1> const& coordinate) const noexcept
        {
            return coordinate[0];
        }

        // Metaprogramming support for index(IndexTypes...) using
        // right-to-left ordering.
        template <typename Term, typename First, typename... Successors>
        size_t constexpr MetaGetIndexRtoL(Term t, First first, Successors... successors) const noexcept
        {
            static_assert(
                std::is_integral<First>::value && !std::is_same<First, bool>::value,
                "Arguments must be integer type.");

            size_t constexpr size = MetaArgument<sizeof...(Sizes) - sizeof...(Successors), Sizes...>::value;
            return MetaGetIndexRtoL(size * static_cast<size_t>(first + t), successors...);
        }

        template <typename Term, typename First>
        size_t constexpr MetaGetIndexRtoL(Term t, First first) const noexcept
        {
            static_assert(
                std::is_integral<First>::value && !std::is_same<First, bool>::value,
                "Arguments must be integer type.");

            return static_cast<size_t>(first + t);
        }

        // Metaprogramming support for index(std::array<*,*)> const&) using
        // left-to-right ordering.
        template <size_t numDimensions = sizeof...(Sizes), TraitSelector<(numDimensions > 1)> = 0>
        size_t GetIndexRtoL(std::array<size_t, sizeof...(Sizes)> const& coordinate) const noexcept
        {
            std::array<size_t, sizeof...(Sizes)> sizes{};
            MetaAssignSize<0, Sizes...>(sizes.data());
            size_t d = 0;
            size_t indexRtoL = coordinate[d];
            for (++d; d < sizeof...(Sizes); ++d)
            {
                indexRtoL = sizes[d] * indexRtoL + coordinate[d];
            }
            return indexRtoL;
        }

        template <size_t numDimensions = sizeof...(Sizes), TraitSelector<numDimensions == 1> = 0>
        size_t GetIndexRtoL(std::array<size_t, 1> const& coordinate) const noexcept
        {
            return coordinate[0];
        }

    public:
        static size_t constexpr NumDimensions = sizeof...(Sizes);
        static size_t constexpr NumElements = MetaProduct<Sizes...>::value;
    };
}

// Implementation for a lattice whose sizes are known only at run time.
// The class stores (b[0],...,b[n-1]) in mSizes and the product of
// bounds in mNumElements.
namespace gtl
{
    template <bool OrderLtoR>
    class Lattice<OrderLtoR>
    {
    public:
        // The lattice has no elements.
        Lattice()
            :
            mNumElements(0),
            mSizes{}
        {
        }

        // The lattice has the specified sizes.
        Lattice(std::vector<size_t> const& sizes)
            :
            mNumElements(0),
            mSizes{}
        {
            InternalReset(sizes);
        }

        // The lattice has the specified sizes.
        Lattice(std::initializer_list<size_t> const& sizes)
            :
            mNumElements(0),
            mSizes{}
        {
            InternalReset(sizes);
        }

        // Support for deferred construction where the initial lattice is
        // created by the default constructor. During later execution, the
        // lattice sizes can be set as needed.
        void reset(std::vector<size_t> const& sizes)
        {
            InternalReset(sizes);
        }

        void reset(std::initializer_list<size_t> const& sizes)
        {
            InternalReset(sizes);
        }

        ~Lattice() = default;

        // Copy semantics.
        Lattice(Lattice const& other)
            :
            mNumElements(0),
            mSizes{}
        {
            *this = other;
        }

        Lattice& operator=(Lattice const& other)
        {
            mNumElements = other.mNumElements;
            mSizes = other.mSizes;
            return *this;
        }

        // Move semantics.
        Lattice(Lattice&& other) noexcept
            :
            mNumElements(0),
            mSizes{}
        {
            *this = std::move(other);
        }

        Lattice& operator=(Lattice&& other) noexcept
        {
            mNumElements = other.mNumElements;
            mSizes = std::move(other.mSizes);
            other.mNumElements = 0;
            return *this;
        }

        // The number of dimensions is the number of elements of mSizes. This
        // is 'n' in the comments about lattices.
        inline size_t dimensions() const noexcept
        {
            return mSizes.size();
        }

        // Get the number of elements for dimension d. This is 'b[d]' in the
        // comments about lattices.
        inline size_t size(size_t d) const
        {
            GTL_ARGUMENT_ASSERT(
                d < mSizes.size(),
                "Invalid dimension.");

            return mSizes[d];
        }

        // Get the number of elements. This is 'product{d=0}^{n-1} b[d]' in
        // the comments about lattices.
        inline size_t size() const noexcept
        {
            return mNumElements;
        }

        // Convert from an n-dimensional index to a 1-dimensional index for
        // left-to-right ordering.
        template <typename... IndexTypes, bool _OrderLtoR = OrderLtoR, TraitSelector<_OrderLtoR> = 0>
        size_t index(IndexTypes... ntuple) const
        {
            GTL_ARGUMENT_ASSERT(
                mSizes.size() > 0 && mSizes.size() == sizeof...(IndexTypes),
                "Invalid arguments to index.");

            return MetaGetIndexLtoR(ntuple...);
        }

        template <bool _OrderLtoR = OrderLtoR, TraitSelector<_OrderLtoR> = 0>
        size_t index(std::vector<size_t> const& coordinate) const
        {
            GTL_ARGUMENT_ASSERT(
                mSizes.size() > 0 && coordinate.size() == mSizes.size(),
                "Invalid argument to index.");

            size_t d = coordinate.size() - 1;
            size_t indexLtoR = coordinate[d];
            --d;
            for (size_t k = 1; k < coordinate.size(); ++k, --d)
            {
                indexLtoR = mSizes[d] * indexLtoR + coordinate[d];
            }
            return indexLtoR;
        }

        // Convert from an n-dimensional index to a 1-dimensional index for
        // right-to-left ordering.
        template <typename... IndexTypes, bool _OrderLtoR = OrderLtoR, TraitSelector<!_OrderLtoR> = 0>
        size_t index(IndexTypes... ntuple) const
        {
            GTL_ARGUMENT_ASSERT(
                mSizes.size() > 0 && mSizes.size() == sizeof...(IndexTypes),
                "Invalid arguments to index.");

            using Type = typename std::tuple_element<0, std::tuple<IndexTypes...>>::type;
            return MetaGetIndexRtoL(static_cast<Type>(0), ntuple...);
        }

        template <bool _OrderLtoR = OrderLtoR, TraitSelector<!_OrderLtoR> = 0>
        size_t index(std::vector<size_t> const& coordinate) const
        {
            GTL_ARGUMENT_ASSERT(
                mSizes.size() > 0 && coordinate.size() == mSizes.size(),
                "Invalid argument to index.");

            size_t d = 0;
            size_t indexRtoL = coordinate[d];
            for (++d; d < coordinate.size(); ++d)
            {
                indexRtoL = mSizes[d] * indexRtoL + coordinate[d];
            }
            return indexRtoL;
        }

        // Convert from a 1-dimension index to an n-dimensional index for
        // left-to-right ordering.
        template <bool _OrderLtoR = OrderLtoR, TraitSelector<_OrderLtoR> = 0>
        std::vector<size_t> coordinate(size_t i) const
        {
            // i = x[0] + b[0] * (x[1] + b[1] * (x[2] + ...)
            // tuple = (x[0], ..., x[n-1])
            size_t const numDimensions = dimensions();
            std::vector<size_t> tuple(numDimensions);
            for (size_t d = 0; d < numDimensions; ++d)
            {
                size_t const bound = size(d), j = i;
                i /= bound;
                tuple[d] = j - bound * i;
            }
            return tuple;
        }

        // Convert from a 1-dimension index to an n-dimensional index for
        // right-to-left ordering.
        template <bool _OrderLtoR = OrderLtoR, TraitSelector<!_OrderLtoR> = 0>
        std::vector<size_t> coordinate(size_t i) const
        {
            // i = x[n-1] + b[n-1] * (x[n-2] + b[n-2] * (x[n-3] + ...)
            // tuple = (x[0], ..., x[n-1])
            size_t const numDimensions = dimensions();
            std::vector<size_t> tuple(numDimensions);
            for (size_t k = 0, d = numDimensions - 1; k < numDimensions; ++k, --d)
            {
                size_t const bound = size(d), j = i;
                i /= bound;
                tuple[d] = j - bound * i;
            }
            return tuple;
        }

        // Support for sorting and comparing Lattice objects.
        inline bool operator==(Lattice const& other) const
        {
            return mSizes == other.mSizes;
        }

        inline bool operator!=(Lattice const& other) const
        {
            return mSizes != other.mSizes;
        }

        inline bool operator<(Lattice const& other) const
        {
            return mSizes < other.mSizes;
        }

        inline bool operator<=(Lattice const& other) const
        {
            return mSizes <= other.mSizes;
        }

        inline bool operator>(Lattice const& other) const
        {
            return mSizes < other.mSizes;
        }

        inline bool operator>=(Lattice const& other) const
        {
            return mSizes >= other.mSizes;
        }

    private:
        template <typename Container>
        void InternalReset(Container const& container)
        {
            GTL_ARGUMENT_ASSERT(
                container.size() > 0,
                "The number of dimensions must be positive.");

            mNumElements = 1;
            mSizes.resize(container.size());
            size_t d = 0;
            for (auto const& size : container)
            {
                GTL_ARGUMENT_ASSERT(
                    size > 0,
                    "The dimension must be positive");

                mNumElements *= size;
                mSizes[d++] = size;
            }
        }

        // Metaprogramming support for index(IndexTypes...) using
        // left-to-right ordering.
        template <typename First, typename... Successors>
        size_t MetaGetIndexLtoR(First first, Successors... successors) const
        {
            static_assert(
                std::is_integral<First>::value && !std::is_same<First, bool>::value,
                "Arguments must be integer type.");

            return static_cast<size_t>(first) +
                mSizes[mSizes.size() - 1 - sizeof...(Successors)] *
                MetaGetIndexLtoR(successors...);
        }

        template <typename Last>
        size_t MetaGetIndexLtoR(Last last) const
        {
            static_assert(
                std::is_integral<Last>::value && !std::is_same<Last, bool>::value,
                "Arguments must be integer type.");

            return static_cast<size_t>(last);
        }

        // Metaprogramming support for index(IndexTypes...) using
        // right-to-left ordering.
        template <typename Term, typename First, typename... Successors>
        size_t MetaGetIndexRtoL(Term t, First first, Successors... successors) const
        {
            static_assert(
                std::is_integral<First>::value && !std::is_same<First, bool>::value,
                "Arguments must be integer type.");

            size_t size = mSizes[mSizes.size() - sizeof...(Successors)];
            return MetaGetIndexRtoL(size * static_cast<size_t>(first + t), successors...);
        }

        template <typename Term, typename First>
        size_t MetaGetIndexRtoL(Term t, First first) const
        {
            static_assert(
                std::is_integral<First>::value && !std::is_same<First, bool>::value,
                "Arguments must be integer type.");

            return static_cast<size_t>(first + t);
        }

        template <typename First>
        size_t MetaGetIndexRtoL(First first) const
        {
            static_assert(
                std::is_integral<First>::value && !std::is_same<First, bool>::value,
                "Arguments must be integer type.");

            return static_cast<size_t>(first);
        }

    protected:
        size_t mNumElements;
        std::vector<size_t> mSizes;
    };
}
