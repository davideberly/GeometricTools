// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.04.06

#pragma once

// Documentation for this class is
// https://www.geometrictools.com/Documentation/GTLUtility.pdf#RawIterators

#include <cstddef>
#include <cstdint>
#include <iterator>

namespace gtl
{
    template <typename T>
    class RawConstIterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T const*;
        using reference = T const&;

        // Construction.
        inline RawConstIterator()
            :
            mPointer(nullptr)
        {
        }

        inline explicit RawConstIterator(pointer inPointer, size_t offset = 0)
            :
            mPointer(inPointer + offset)
        {
        }

        // Pointer access.
        inline reference operator*() const
        {
            return *mPointer;
        }

        inline pointer operator->() const
        {
            return mPointer;
        }

        inline reference operator[](ptrdiff_t const offset) const
        {
            return mPointer[offset];
        }

        // Pointer arithmetic.
        inline RawConstIterator& operator++()
        {
            ++mPointer;
            return *this;
        }

        inline RawConstIterator operator++(int32_t)
        {
            RawConstIterator current = *this;
            ++mPointer;
            return current;
        }

        inline RawConstIterator& operator--()
        {
            --mPointer;
            return *this;
        }

        inline RawConstIterator operator--(int32_t)
        {
            RawConstIterator current = *this;
            --mPointer;
            return current;
        }

        inline RawConstIterator& operator+=(ptrdiff_t const offset)
        {
            mPointer += offset;
            return *this;
        }

        inline RawConstIterator operator+(ptrdiff_t const offset) const
        {
            RawConstIterator current = *this;
            return current += offset;
        }

        inline RawConstIterator& operator-=(ptrdiff_t const offset)
        {
            mPointer -= offset;
            return *this;
        }

        inline RawConstIterator operator-(ptrdiff_t const offset) const
        {
            RawConstIterator current = *this;
            return current -= offset;
        }

        inline ptrdiff_t operator-(RawConstIterator const& other) const
        {
            return mPointer - other.mPointer;
        }

        // Pointer comparisons.
        inline bool operator==(RawConstIterator const& other) const
        {
            return mPointer == other.mPointer;
        }

        inline bool operator!=(RawConstIterator const& other) const
        {
            return mPointer != other.mPointer;
        }

        inline bool operator<(RawConstIterator const& other) const
        {
            return mPointer < other.mPointer;
        }

        inline bool operator<=(RawConstIterator const& other) const
        {
            return mPointer <= other.mPointer;
        }

        inline bool operator>(RawConstIterator const& other) const
        {
            return mPointer > other.mPointer;
        }

        inline bool operator>=(RawConstIterator const& other) const
        {
            return mPointer >= other.mPointer;
        }

    private:
        pointer mPointer;
    };


    template <typename T>
    class RawIterator : public RawConstIterator<T>
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        // Construction.
        inline RawIterator() noexcept
        {
        }

        inline explicit RawIterator(pointer inPointer, size_t offset = 0)
            :
            RawConstIterator<T>(inPointer, offset)
        {
        }

        // Pointer access.
        inline reference operator*() const
        {
            return const_cast<reference>(RawConstIterator<T>::operator*());
        }

        inline pointer operator->() const
        {
            return const_cast<pointer>(RawConstIterator<T>::operator->());
        }

        // Pointer arithmetic.
        inline RawIterator operator++()
        {
            RawConstIterator<T>::operator++();
            return *this;
        }

        inline RawIterator operator++(int32_t)
        {
            RawIterator current = *this;
            RawConstIterator<T>::operator++();
            return current;
        }

        inline RawIterator operator--()
        {
            RawConstIterator<T>::operator--();
            return *this;
        }

        inline RawIterator operator--(int32_t)
        {
            RawIterator current = *this;
            RawConstIterator<T>::operator--();
            return current;
        }

        inline RawIterator& operator+=(ptrdiff_t const offset)
        {
            RawConstIterator<T>::operator+=(offset);
            return *this;
        }

        inline RawIterator operator+(ptrdiff_t const offset) const
        {
            RawIterator current = *this;
            return current += offset;
        }

        inline RawIterator& operator-=(ptrdiff_t const offset)
        {
            RawConstIterator<T>::operator-=(offset);
            return *this;
        }

        inline RawIterator operator-(ptrdiff_t const offset) const
        {
            RawIterator current = *this;
            return current -= offset;
        }
    };
}
