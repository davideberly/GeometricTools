// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.04.06

#pragma once

// Documentation for this class is
// https://www.geometrictools.com/Documentation/GTLUtility.pdf#ContainerAdapter

#include <GTL/Utility/Exceptions.h>
#include <GTL/Utility/RawIterators.h>
#include <algorithm>
#include <iterator>

namespace gtl
{
    // The primary template for a container that has most of the interface for
    // std::vector, but the source data is a raw pointer. This supports
    // functions that can accept arguments of type std::array<T,N> or of type
    // ContainerAdapter<T,N> for N > 0 (size known at compile time). And it
    // supports functions that can accept arguments of type std::vector<T> or
    // of type ContainerAdapter<T> (size known only at run time).
    template <typename T, size_t...>
    class ContainerAdapter;
}

namespace gtl
{
    template <typename T, size_t N>
    class ContainerAdapter<T, N>
    {
    public:
        using value_type = T;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using const_pointer = T const*;
        using reference = T&;
        using const_reference = T const&;
        using iterator = RawIterator<T>;
        using const_iterator = RawConstIterator<T>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // Construction. The input is a non-null pointer to a contiguous array
        // of elements. The caller must ensure that the number of elements is
        // at least N. You may pass a null pointer, but you are then expected
        // to call 'reset(elements)' with a non-null pointer before using the
        // object. The last default argument allows generic implementation in
        // adapter operations; it is ignored by the constructor.
        ContainerAdapter(T* elements = nullptr, size_t = 0)
            :
            mElements(elements)
        {
            static_assert(
                N > 0,
                "ContainerAdapter<T,N> requires N > 0.");
        }

        // Destruction. Nothing is destroyed because the class does not know
        // anything about the input raw data. If the data was dynamically
        // allocated elsewhere in the application, the caller is responsible
        // for deallocating it.
        ~ContainerAdapter() = default;

        // Copy semantics. The copy constructor is disabled to avoid having
        // the raw pointer shared between objects, a simplified design for
        // the adapter system. Assignment is allowed between objects of the
        // same size.
        ContainerAdapter(ContainerAdapter const&) = delete;

        ContainerAdapter& operator=(ContainerAdapter const& other)
        {
            std::copy(other.begin(), other.end(), begin());
            return *this;
        }

        // Move semantics are disabled.
        ContainerAdapter(ContainerAdapter&&) noexcept = delete;
        ContainerAdapter& operator=(ContainerAdapter&&) noexcept = delete;

        // Call this function with a non-null pointer before manipulating a
        // ContainerAdapter object created with a null pointer. The default
        // argument allows generic implementation in adapter operations; it
        // is ignored by the function.
        inline void reset(T* elements, size_t = 0) noexcept
        {
            mElements = elements;
        }

        // Size and data access.
        size_t constexpr size() const noexcept
        {
            return N;
        }

        inline T* data() noexcept
        {
            return mElements;
        }

        inline T const* data() const noexcept
        {
            return mElements;
        }

        // Element access. The 'at' functions throw std::runtime_error
        // exceptions when the storage pointer is null, and they throw
        // std::out_of_range exceptions for an invalid index. The
        // 'operator[]' functions do not throw exceptions.
        inline T const& at(size_t i) const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            GTL_LENGTH_ASSERT(
                i < N,
                "Index exceeds maximum.");

            return mElements[i];
        }

        inline T& at(size_t i)
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            GTL_LENGTH_ASSERT(
                i < N,
                "Index exceeds maximum.");

            return mElements[i];
        }

        inline T const& operator[](size_t i) const noexcept
        {
            return mElements[i];
        }

        inline T& operator[](size_t i) noexcept
        {
            return mElements[i];
        }

        // Support for iteration.
        inline iterator begin()
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return iterator(mElements, 0);
        }

        inline const_iterator begin() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return const_iterator(mElements, 0);
        }

        inline iterator end()
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return iterator(mElements, N);
        }

        inline const_iterator end() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return const_iterator(mElements, N);
        }

        inline reverse_iterator rbegin()
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return reverse_iterator(end());
        }

        inline const_reverse_iterator rbegin() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return const_reverse_iterator(end());
        }

        inline reverse_iterator rend()
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return reverse_iterator(begin());
        }

        inline const_reverse_iterator rend() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return const_reverse_iterator(begin());
        }

        inline const_iterator cbegin() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return begin();
        }

        inline const_iterator cend() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return end();
        }

        inline const_reverse_iterator crbegin() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return rbegin();
        }

        inline const_reverse_iterator crend() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return rend();
        }

        // Set all elements to the specified value.
        void fill(T const& value)
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            std::fill(begin(), end(), value);
        }

    private:
        T* mElements;
    };
}

namespace gtl
{
    template <typename T>
    class ContainerAdapter<T>
    {
    public:
        using value_type = T;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using const_pointer = T const*;
        using reference = T&;
        using const_reference = T const&;
        using iterator = RawIterator<T>;
        using const_iterator = RawConstIterator<T>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // Construction. Generally, the input is a non-null pointer to a
        // contiguous array of elements. The caller must ensure that the
        // number of elements is at least 'numElements'. You may pass a null
        // pointer, but you are then expected to call
        // 'reset(numElements, elements)' with numElements > 0 and a non-null
        // pointer before using the object.
        ContainerAdapter(T* elements = nullptr, size_t numElements = 0)
            :
            mNumElements(numElements),
            mElements(elements)
        {
            GTL_ARGUMENT_ASSERT(
                (elements && numElements) || (!elements && !numElements),
                "Invalid element state.");
        }

        // Destruction. Nothing is destroyed because the class does not know
        // anything about the input raw data. If this data was dynamically
        // allocated elsewhere in the application, the caller is responsible
        // for deallocating it.
        ~ContainerAdapter() = default;

        // Copy semantics. The copy constructor is disabled to avoid having
        // the raw pointer shared between objects, a simplified design for
        // the adapter system. Assignment is allowed between objects of the
        // same size.
        ContainerAdapter(ContainerAdapter const&) = delete;

        ContainerAdapter& operator=(ContainerAdapter const& other)
        {
            GTL_LENGTH_ASSERT(
                mNumElements == other.mNumElements,
                "Mismatched sizes.");

            std::copy(other.begin(), other.end(), begin());
            return *this;
        }

        // Move semantics are disabled.
        ContainerAdapter(ContainerAdapter&&) noexcept = delete;
        ContainerAdapter& operator=(ContainerAdapter&&) noexcept = delete;

        // Call this function with numElements > 0 and a nonnull pointer
        // before manipulating a ContainerAdapter object created with no
        // elements and a null pointer.
        inline void reset(T* elements, size_t numElements)
        {
            GTL_ARGUMENT_ASSERT(
                (numElements == 0 && !elements) || (numElements > 0 && elements),
                "Invalid element state.");

            mNumElements = numElements;
            mElements = elements;
        }

        // Size and data access.
        inline size_t size() const noexcept
        {
            return mNumElements;
        }

        inline T* data() noexcept
        {
            return mElements;
        }

        inline T const* data() const noexcept
        {
            return mElements;
        }

        // Element access. The 'at' functions throw std::runtime_error
        // exceptions when the storage pointer is null, and they throw
        // std::out_of_range exceptions for an invalid index. The
        // 'operator[]' functions do not throw exceptions.
        inline T const& at(size_t i) const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            GTL_LENGTH_ASSERT(
                i < mNumElements,
                "Index exceeds maximum.");

            return mElements[i];
        }

        inline T& at(size_t i)
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            GTL_LENGTH_ASSERT(
                i < mNumElements,
                "Index exceeds maximum.");

            return mElements[i];
        }

        inline T const& operator[](size_t i) const noexcept
        {
            return mElements[i];
        }

        inline T& operator[](size_t i) noexcept
        {
            return mElements[i];
        }

        // Support for iteration.
        inline iterator begin()
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return iterator(mElements, 0);
        }

        inline const_iterator begin() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return const_iterator(mElements, 0);
        }

        inline iterator end()
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return iterator(mElements, mNumElements);
        }

        inline const_iterator end() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return const_iterator(mElements, mNumElements);
        }

        inline reverse_iterator rbegin()
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return reverse_iterator(end());
        }

        inline const_reverse_iterator rbegin() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return const_reverse_iterator(end());
        }

        inline reverse_iterator rend()
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return reverse_iterator(begin());
        }

        inline const_reverse_iterator rend() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return const_reverse_iterator(begin());
        }

        inline const_iterator cbegin() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return begin();
        }

        inline const_iterator cend() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return end();
        }

        inline const_reverse_iterator crbegin() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return rbegin();
        }

        inline const_reverse_iterator crend() const
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            return rend();
        }

        // Set all elements to the specified value.
        void fill(T const& value)
        {
            GTL_RUNTIME_ASSERT(
                mElements,
                "Elements pointer is null.");

            for (size_t i = 0; i < mNumElements; ++i)
            {
                mElements[i] = value;
            }
        }

    private:
        size_t mNumElements;
        T* mElements;
    };
}
