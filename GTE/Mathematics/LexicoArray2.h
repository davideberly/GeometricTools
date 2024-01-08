// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <cstdint>

namespace gte
{
    // A template class to provide 2D array access that conforms to row-major
    // order (RowMajor = true) or column-major order (RowMajor = false).  The
    template <bool RowMajor, typename Real, int32_t... Dimensions>
    class LexicoArray2 {};

    // The array dimensions are known only at run time.
    template <typename Real>
    class LexicoArray2<true, Real>
    {
    public:
        LexicoArray2(int32_t numRows, int32_t numCols, Real* matrix)
            :
            mNumRows(numRows),
            mNumCols(numCols),
            mMatrix(matrix)
        {
        }

        inline int32_t GetNumRows() const
        {
            return mNumRows;
        }

        inline int32_t GetNumCols() const
        {
            return mNumCols;
        }

        inline Real& operator()(int32_t r, int32_t c)
        {
            return mMatrix[c + mNumCols * r];
        }

        inline Real const& operator()(int32_t r, int32_t c) const
        {
            return mMatrix[c + mNumCols * r];
        }

    private:
        int32_t mNumRows, mNumCols;
        Real* mMatrix;
    };

    template <typename Real>
    class LexicoArray2<false, Real>
    {
    public:
        LexicoArray2(int32_t numRows, int32_t numCols, Real* matrix)
            :
            mNumRows(numRows),
            mNumCols(numCols),
            mMatrix(matrix)
        {
        }

        inline int32_t GetNumRows() const
        {
            return mNumRows;
        }

        inline int32_t GetNumCols() const
        {
            return mNumCols;
        }

        inline Real& operator()(int32_t r, int32_t c)
        {
            return mMatrix[r + mNumRows * c];
        }

        inline Real const& operator()(int32_t r, int32_t c) const
        {
            return mMatrix[r + mNumRows * c];
        }

    private:
        int32_t mNumRows, mNumCols;
        Real* mMatrix;
    };

    // The array dimensions are known at compile time.
    template <typename Real, int32_t NumRows, int32_t NumCols>
    class LexicoArray2<true, Real, NumRows, NumCols>
    {
    public:
        LexicoArray2(Real* matrix)
            :
            mMatrix(matrix)
        {
        }

        inline int32_t GetNumRows() const
        {
            return NumRows;
        }

        inline int32_t GetNumCols() const
        {
            return NumCols;
        }

        inline Real& operator()(int32_t r, int32_t c)
        {
            return mMatrix[c + NumCols * r];
        }

        inline Real const& operator()(int32_t r, int32_t c) const
        {
            return mMatrix[c + NumCols * r];
        }

    private:
        Real* mMatrix;
    };

    template <typename Real, int32_t NumRows, int32_t NumCols>
    class LexicoArray2<false, Real, NumRows, NumCols>
    {
    public:
        LexicoArray2(Real* matrix)
            :
            mMatrix(matrix)
        {
        }

        inline int32_t GetNumRows() const
        {
            return NumRows;
        }

        inline int32_t GetNumCols() const
        {
            return NumCols;
        }

        inline Real& operator()(int32_t r, int32_t c)
        {
            return mMatrix[r + NumRows * c];
        }

        inline Real const& operator()(int32_t r, int32_t c) const
        {
            return mMatrix[r + NumRows * c];
        }

    private:
        Real* mMatrix;
    };
}
