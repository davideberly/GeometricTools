// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2023.09.17
#pragma once

// Documentation for this class is
// https://www.geometrictools.com/Documentation/GTLUtility.pdf#Multiarray

#include <GTL/Utility/Lattice.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

// Implementation for multiarrays whose sizes are known at compile time.
namespace gtl
{
    template <typename T, bool OrderLtoR, size_t... Sizes>
    class Multiarray : public Lattice<OrderLtoR, Sizes...>
    {
    public:
        // All elements of the multiarray are uninitialized for native data
        // but are initialized when the default T constructor initializes
        // its data.
        Multiarray()
            :
            mContainer{}
        {
        }

        ~Multiarray() = default;

        // Copy semantics.
        Multiarray(Multiarray const& other)
            :
            mContainer{}
        {
            *this = other;
        }

        Multiarray& operator=(Multiarray const& other)
        {
            mContainer = other.mContainer;
            return *this;
        }

        // Move semantics.
        Multiarray(Multiarray&& other) noexcept
            :
            mContainer{}
        {
            *this = std::move(other);
        }

        Multiarray& operator=(Multiarray&& other) noexcept
        {
            mContainer = std::move(other.mContainer);
            return *this;
        }

        // Get a pointer to the array of elements.
        inline T const* data() const noexcept
        {
            return mContainer.data();
        }

        inline T* data() noexcept
        {
            return mContainer.data();
        }

        // Access the elements at the specified index.
        inline T const& at(size_t i) const
        {
            return mContainer.at(i);
        }

        inline T& at(size_t i)
        {
            return mContainer.at(i);
        }

        inline T const& operator[](size_t i) const
        {
            return mContainer[i];
        }

        inline T& operator[](size_t i)
        {
            return mContainer[i];
        }

        // Set all elements to the specified value.
        inline void fill(T const& value)
        {
            mContainer.fill(value);
        }

        // Get the element corresponding to the n-dimensional parameter pack
        // of indices.
        template <typename... IndexTypes>
        T const& operator()(IndexTypes... ntuple) const
        {
            return mContainer[this->index(ntuple...)];
        }

        template <typename... IndexTypes>
        T& operator()(IndexTypes... ntuple)
        {
            return mContainer[this->index(ntuple...)];
        }

        // Get the element corresponding to the n-dimensional coordinate.
        T const& operator()(std::array<size_t, sizeof...(Sizes)> const& coordinate) const
        {
            return mContainer[this->index(coordinate)];
        }

        T& operator()(std::array<size_t, sizeof...(Sizes)> const& coordinate)
        {
            return mContainer[this->index(coordinate)];
        }

        // Support for sorting and comparing Multiarray objects.
        inline bool operator==(Multiarray const& other) const
        {
            return mContainer == other.mContainer;
        }

        inline bool operator!=(Multiarray const& other) const
        {
            return mContainer != other.mContainer;
        }

        inline bool operator< (Multiarray const& other) const
        {
            return mContainer < other.mContainer;
        }

        inline bool operator<=(Multiarray const& other) const
        {
            return mContainer <= other.mContainer;
        }

        inline bool operator> (Multiarray const& other) const
        {
            return mContainer > other.mContainer;
        }

        inline bool operator>=(Multiarray const& other) const
        {
            return mContainer >= other.mContainer;
        }

    private:
        std::array<T, Lattice<OrderLtoR, Sizes...>::NumElements> mContainer;

    private:
        friend class UnitTestMultiarray;
    };
}

// Implementation for multiarrays whose sizes are known only at run time.
namespace gtl
{
    template <typename T, bool OrderLtoR>
    class Multiarray<T, OrderLtoR> : public Lattice<OrderLtoR>
    {
    public:
        // The multiarray has no elements.
        Multiarray()
            :
            Lattice<OrderLtoR>{},
            mContainer{}
        {
        }

        // The multiarray has the specified sizes and all elements are
        // uninitialized.
        Multiarray(std::vector<size_t> const& sizes)
            :
            Lattice<OrderLtoR>(sizes),
            mContainer{}
        {
            mContainer.resize(this->size());
        }

        Multiarray(std::initializer_list<size_t> const& sizes)
            :
            Lattice<OrderLtoR>(sizes),
            mContainer{}
        {
            mContainer.resize(this->size());
        }

        // Support for deferred construction where the initial multiarray is
        // created by the default constructor. During later execution, the
        // multiarray sizes can be set as needed.
        void reset(std::vector<size_t> const& sizes)
        {
            Lattice<OrderLtoR>::reset(sizes);
            mContainer.resize(this->size());
        }

        void reset(std::initializer_list<size_t> const& sizes)
        {
            Lattice<OrderLtoR>::reset(sizes);
            mContainer.resize(this->size());
        }

        ~Multiarray() = default;

        // Copy semantics.
        Multiarray(Multiarray const& other)
            :
            Lattice<OrderLtoR>{},
            mContainer{}
        {
            *this = other;
        }

        Multiarray& operator=(Multiarray const& other)
        {
            (void)Lattice<OrderLtoR>::operator=(other);
            mContainer = other.mContainer;
            return *this;
        }

        // Move semantics.
        Multiarray(Multiarray&& other) noexcept
            :
            Lattice<OrderLtoR>{},
            mContainer{}
        {
            *this = std::move(other);
        }

        Multiarray& operator=(Multiarray&& other) noexcept
        {
            (void)Lattice<OrderLtoR>::operator=(std::move(other));
            mContainer = std::move(other.mContainer);
            return *this;
        }

        // Get a pointer to the array of elements.
        inline T const* data() const noexcept
        {
            return mContainer.data();
        }

        inline T* data() noexcept
        {
            return mContainer.data();
        }

        // Access the elements at the specified index.
        inline T const& at(size_t i) const
        {
            return mContainer.at(i);
        }

        inline T& at(size_t i)
        {
            return mContainer.at(i);
        }

        // Access the elements at the specified index.
        inline T const& operator[](size_t i) const
        {
            return mContainer[i];
        }

        inline T& operator[](size_t i)
        {
            return mContainer[i];
        }

        // Set all elements to the specified value.
        inline void fill(T const& value)
        {
            std::fill(mContainer.begin(), mContainer.end(), value);
        }

        // Get the element corresponding to the n-dimensional tuple of
        // indices.
        template <typename... IndexTypes>
        T const& operator()(IndexTypes... ntuple) const
        {
            return mContainer[this->index(ntuple...)];
        }

        template <typename... IndexTypes>
        T& operator()(IndexTypes... ntuple)
        {
            return mContainer[this->index(ntuple...)];
        }

        // Get the element corresponding to the n-dimensional coordinate.
        T const& operator()(std::vector<size_t> const& coordinate) const
        {
            return mContainer[this->index(coordinate)];
        }

        T& operator()(std::vector<size_t> const& coordinate)
        {
            return mContainer[this->index(coordinate)];
        }

        // Support for sorting and comparing Multiarray objects.
        bool operator==(Multiarray const& other) const
        {
            return Lattice<OrderLtoR>::operator==(other)
                && mContainer == other.mContainer;
        }

        bool operator!=(Multiarray const& other) const
        {
            return !operator==(other);
        }

        bool operator< (Multiarray const& other) const
        {
            if (Lattice<OrderLtoR>::operator<(other))
            {
                return true;
            }
            if (Lattice<OrderLtoR>::operator>(other))
            {
                return false;
            }
            return  mContainer < other.mContainer;
        }

        bool operator<=(Multiarray const& other) const
        {
            return !(other < *this);
        }

        bool operator> (Multiarray const& other) const
        {
            return other < *this;
        }

        bool operator>=(Multiarray const& other) const
        {
            return !operator<(other);
        }

    private:
        std::vector<T> mContainer;

    private:
        friend class UnitTestMultiarray;
    };
}
