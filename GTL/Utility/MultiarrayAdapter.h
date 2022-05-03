// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Documentation for this class is
// https://www.geometrictools.com/Documentation/GTLUtility.pdf#MultiarrayAdapter

#include <GTL/Utility/Lattice.h>
#include <array>
#include <vector>

// Implementation for multiarray adapters whose sizes are known at compile
// time.
namespace gtl
{
    template <typename T, bool OrderLtoR, size_t... Sizes>
    class MultiarrayAdapter : public Lattice<OrderLtoR, Sizes...>
    {
    public:
        // All elements of the multiarray are uninitialized for native data
        // but are initialized when the default T constructor initializes
        // its data.
        MultiarrayAdapter(T* container = nullptr)
            :
            mContainer(container)
        {
        }

        // Support for deferred construction where the initial multiarray is
        // created by the default constructor whose input is the null pointer.
        // During later execution, the pointer can be set to point to an
        // actual block of memory.
        void reset(T* container)
        {
            GTL_ARGUMENT_ASSERT(
                container != nullptr,
                "The container must exist.");

            mContainer = container;
        }

        ~MultiarrayAdapter() = default;

        // Disallow copy semantics and move semantics.
        MultiarrayAdapter(MultiarrayAdapter const&) = delete;
        MultiarrayAdapter& operator=(MultiarrayAdapter const&) = delete;
        MultiarrayAdapter(MultiarrayAdapter&&) = delete;
        MultiarrayAdapter& operator=(MultiarrayAdapter&&) = delete;

        // Get a pointer to the array of elements.
        inline T const* data() const noexcept
        {
            return mContainer;
        }

        inline T* data() noexcept
        {
            return mContainer;
        }

        // Access the elements at the specified index.
        T const& at(size_t i) const
        {
            GTL_ARGUMENT_ASSERT(
                i < this->size(),
                "Invalid index.");

            return mContainer[i];
        }

        T& at(size_t i)
        {
            GTL_ARGUMENT_ASSERT(
                i < this->size(),
                "Invalid index.");

            return mContainer[i];
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
        void fill(T const& value)
        {
            for (size_t i = 0; i < this->size(); ++i)
            {
                mContainer[i] = value;
            }
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

        // Support for sorting and comparing MultiarrayAdapter objects.
        inline bool operator==(MultiarrayAdapter const& other) const
        {
            if (mContainer != nullptr)
            {
                if (other.mContainer != nullptr)
                {
                    for (size_t i = 0; i < this->size(); ++i)
                    {
                        if (mContainer[i] != other.mContainer[i])
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return other.mContainer == nullptr;
            }
        }

        inline bool operator!=(MultiarrayAdapter const& other) const
        {
            return !operator==(other);
        }

        inline bool operator< (MultiarrayAdapter const& other) const
        {
            if (other.mContainer != nullptr)
            {
                if (mContainer != nullptr)
                {
                    for (size_t i = 0; i < this->size(); ++i)
                    {
                        if (mContainer[i] < other.mContainer[i])
                        {
                            return true;
                        }

                        if (mContainer[i] > other.mContainer[i])
                        {
                            return false;
                        }
                    }
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else
            {
                return false;
            }
        }

        inline bool operator<=(MultiarrayAdapter const& other) const
        {
            return !other.operator<(*this);
        }

        inline bool operator> (MultiarrayAdapter const& other) const
        {
            return other.operator<(*this);
        }

        inline bool operator>=(MultiarrayAdapter const& other) const
        {
            return !operator<(other);
        }

    private:
        // The pointer must be to a block of memory storing
        // Lattice<OrderLtoR, Sizes...>::size() T-objects.
        T* mContainer;
    };
}

// Implementation for multiarrays whose sizes are known only at run time.
namespace gtl
{
    template <typename T, bool OrderLtoR>
    class MultiarrayAdapter<T, OrderLtoR> : public Lattice<OrderLtoR>
    {
    public:
        // The multiarray has no elements.
        MultiarrayAdapter()
            :
            Lattice<OrderLtoR>{},
            mContainer(nullptr)
        {
        }

        // The multiarray has the specified sizes and all elements are
        // uninitialized.
        MultiarrayAdapter(std::vector<size_t> const& sizes, T* container)
            :
            Lattice<OrderLtoR>(sizes),
            mContainer(container)
        {
            GTL_ARGUMENT_ASSERT(
                container != nullptr,
                "The container must exist.");
        }

        // Support for deferred construction where the initial multiarray is
        // created by the default constructor whose input is the null pointer.
        // During later execution, the lattice sizes can be set as needed.
        void reset(std::vector<size_t> const& sizes, T* container)
        {
            GTL_ARGUMENT_ASSERT(
                container != nullptr,
                "The container must exist.");

            Lattice<OrderLtoR>::reset(sizes);
            mContainer = container;
        }

        ~MultiarrayAdapter() = default;

        // Disallow copy semantics and move semantics.
        MultiarrayAdapter(MultiarrayAdapter const&) = delete;
        MultiarrayAdapter& operator=(MultiarrayAdapter const&) = delete;
        MultiarrayAdapter(MultiarrayAdapter&&) = delete;
        MultiarrayAdapter& operator=(MultiarrayAdapter&&) = delete;

        // Get a pointer to the array of elements.
        inline T const* data() const noexcept
        {
            return mContainer;
        }

        inline T* data() noexcept
        {
            return mContainer;
        }

        // Access the elements at the specified index.
        T const& at(size_t i) const
        {
            GTL_ARGUMENT_ASSERT(
                i < this->size(),
                "Invalid index.");

            return mContainer[i];
        }

        T& at(size_t i)
        {
            GTL_ARGUMENT_ASSERT(
                i < this->size(),
                "Invalid index.");

            return mContainer[i];
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
        void fill(T const& value)
        {
            for (size_t i = 0; i < this->size(); ++i)
            {
                mContainer[i] = value;
            }
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

        // Support for sorting and comparing MultiarrayAdapter objects.
        bool operator==(MultiarrayAdapter const& other) const
        {
            if (Lattice<OrderLtoR>::operator!=(other))
            {
                return false;
            }

            if (mContainer != nullptr)
            {
                if (other.mContainer != nullptr)
                {
                    for (size_t i = 0; i < this->size(); ++i)
                    {
                        if (mContainer[i] != other.mContainer[i])
                        {
                            return false;
                        }
                    }
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return other.mContainer == nullptr;
            }
        }

        bool operator!=(MultiarrayAdapter const& other) const
        {
            return !operator==(other);
        }

        bool operator< (MultiarrayAdapter const& other) const
        {
            if (Lattice<OrderLtoR>::operator<(other))
            {
                return true;
            }

            if (Lattice<OrderLtoR>::operator>(other))
            {
                return false;
            }

            if (other.mContainer != nullptr)
            {
                if (mContainer != nullptr)
                {
                    for (size_t i = 0; i < this->size(); ++i)
                    {
                        if (mContainer[i] < other.mContainer[i])
                        {
                            return true;
                        }

                        if (mContainer[i] > other.mContainer[i])
                        {
                            return false;
                        }
                    }
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else
            {
                return false;
            }
        }

        bool operator<=(MultiarrayAdapter const& other) const
        {
            return !(other < *this);
        }

        bool operator> (MultiarrayAdapter const& other) const
        {
            return other < *this;
        }

        bool operator>=(MultiarrayAdapter const& other) const
        {
            return !operator<(other);
        }

    private:
        // The pointer must be to a block of memory storing
        // Lattice<OrderLtoR>::size() T-objects.
        T* mContainer;
    };
}
