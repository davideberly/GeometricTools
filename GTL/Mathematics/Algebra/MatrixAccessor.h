// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <cstddef>

namespace gtl
{
    template <typename T>
    class MatrixAccessorBase
    {
    public:
        using value_type = T;

        inline size_t const size() const
        {
            return mNumRows * mNumCols;
        }

        inline T const* data() const
        {
            return mElements();
        }

        inline T* data()
        {
            return mElements();
        }

        inline T const& operator[](size_t i) const
        {
            return mElements[i];
        }

        inline T& operator[](size_t i)
        {
            return mElements[i];
        }

        void reset(size_t numRows, size_t numCols, T* elements)
        {
            mNumRows = numRows;
            mNumCols = numCols;
            mElements = elements;
        }

    protected:
        MatrixAccessorBase(size_t numRows = 0, size_t numCols = 0, T* elements = nullptr)
            :
            mNumRows(numRows),
            mNumCols(numCols),
            mElements(elements)
        {
        }

        size_t mNumRows, mNumCols;
        T* mElements;
    };


    template <typename T, bool IsRowMajor> class MatrixAccessor;

    // Access to row-major storage.
    template <typename T>
    class MatrixAccessor<T, true> : public MatrixAccessorBase<T>
    {
    public:
        MatrixAccessor(size_t numRows = 0, size_t numCols = 0, T* elements = nullptr)
            :
            MatrixAccessorBase<T>(numRows, numCols, elements)
        {
        }

        inline T const& operator()(size_t row, size_t col) const
        {
            return this->mElements[col + this->mNumCols * row];
        }

        inline T& operator()(size_t row, size_t col)
        {
            return this->mElements[col + this->mNumCols * row];
        }
    };

    // Access to column-major storage.
    template <typename T>
    class MatrixAccessor<T, false> : public MatrixAccessorBase<T>
    {
    public:
        MatrixAccessor(size_t numRows = 0, size_t numCols = 0, T* elements = nullptr)
            :
            MatrixAccessorBase<T>(numRows, numCols, elements)
        {
        }

        inline T const& operator()(size_t row, size_t col) const
        {
            return this->mElements[row + this->mNumRows * col];
        }

        inline T& operator()(size_t row, size_t col)
        {
            return this->mElements[row + this->mNumRows * col];
        }
    };
}
