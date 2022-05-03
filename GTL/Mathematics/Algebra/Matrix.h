// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <GTL/Mathematics/Algebra/Vector.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <utility>
#include <vector>

// Template metaprogramming to support Matrix classes and operations.
namespace gtl
{
    // The primary template for Matrix classes. The matrices are stored in
    // row-major order. Any GTL numerical method code that has large matrix
    // inputs specified via a pointer to 1-dimensional memory also provides
    // an input to specify whether the matrix has row-major or column-major
    // order. Internal adapters are provided for 2-dimensional access to
    // the matrix elements.
    template <typename T, size_t...> class Matrix;

    template <typename T, size_t RowDimension, size_t ColDimension>
    struct MatrixTraits
    {
        using value_type = T;
        static size_t constexpr NumRows = RowDimension;
        static size_t constexpr NumCols = ColDimension;
    };
}

// Implementation for matrices whose sizes are known at compile time.
namespace gtl
{
    template <typename T, size_t NumRows, size_t NumCols>
    class Matrix<T, NumRows, NumCols> : public MatrixTraits<T, NumRows, NumCols>
    {
    public:
        // Construction and destruction.

        // All elements of the matrix are initialized to 0.
        Matrix()
            :
            mContainer{}
        {
            static_assert(
                NumRows > 0 && NumCols > 0,
                "The number of rows and columns must be positive.");

            fill(C_<T>(0));
        }

        // Create a matrix from a list of rows.
        Matrix(std::initializer_list<std::initializer_list<T>> const& elements)
            :
            mContainer{}
        {
            static_assert(
                NumRows > 0 && NumCols > 0,
                "The number of rows and columns must be positive.");

            GTL_ARGUMENT_ASSERT(
                elements.size() == NumRows,
                "Invalid length for row initializer list.");

            size_t offset = 0;
            for (auto const& rowElements : elements)
            {
                GTL_ARGUMENT_ASSERT(
                    rowElements.size() == NumCols,
                    "Invalid length for col initializer list.");

                std::copy(rowElements.begin(), rowElements.end(), mContainer.begin() + offset);
                offset += NumCols;
            }
        }

        ~Matrix() = default;


        // Copy semantics.
        Matrix(Matrix const& other)
            :
            mContainer{}
        {
            *this = other;
        }

        Matrix& operator=(Matrix const& other)
        {
            mContainer = other.mContainer;
            return *this;
        }

        // Move semantics.
        Matrix(Matrix&& other) noexcept
            :
            mContainer{}
        {
            *this = std::move(other);
        }

        Matrix& operator=(Matrix&& other) noexcept
        {
            mContainer = std::move(other.mContainer);
            return *this;
        }

        // Size and data access.
        inline size_t constexpr size() const noexcept
        {
            return NumRows * NumCols;
        }

        inline size_t constexpr GetNumRows() const noexcept
        {
            return NumRows;
        }

        inline size_t constexpr GetNumCols() const noexcept
        {
            return NumCols;
        }

        size_t GetIndex(size_t row, size_t col) const noexcept
        {
            return col + NumCols * row;
        }

        inline T const* data() const noexcept
        {
            return mContainer.data();
        }

        inline T* data() noexcept
        {
            return mContainer.data();
        }

        // Container access.
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
            return this->mContainer[i];
        }

        inline T& operator[](size_t i)
        {
            return this->mContainer[i];
        }

        inline T const& at(size_t row, size_t col) const
        {
            return at(GetIndex(row, col));
        }

        inline T& at(size_t row, size_t col)
        {
            return at(GetIndex(row, col));
        }

        inline T const& operator()(size_t row, size_t col) const
        {
            return mContainer[GetIndex(row, col)];
        }

        inline T& operator()(size_t row, size_t col)
        {
            return mContainer[GetIndex(row, col)];
        }

        void SetRow(size_t row, Vector<T, NumCols> const& v)
        {
            GTL_OUTOFRANGE_ASSERT(
                row < NumRows,
                "Invalid row.");

            for (size_t col = 0; col < NumCols; ++col)
            {
                operator()(row, col) = v[col];
            }
        }

        void SetCol(size_t col, Vector<T, NumRows> const& v)
        {
            GTL_OUTOFRANGE_ASSERT(
                col < NumCols,
                "Invalid column.");

            for (size_t row = 0; row < NumRows; ++row)
            {
                operator()(row, col) = v[row];
            }
        }

        Vector<T, NumCols> GetRow(size_t row) const
        {
            GTL_OUTOFRANGE_ASSERT(
                row < NumRows,
                "Invalid row.");

            Vector<T, NumCols> v;
            for (size_t col = 0; col < NumCols; ++col)
            {
                v[col] = operator()(row, col);
            }
            return v;
        }

        Vector<T, NumRows> GetCol(size_t col) const
        {
            GTL_OUTOFRANGE_ASSERT(
                col < NumCols,
                "Invalid column.");

            Vector<T, NumRows> v;
            for (size_t row = 0; row < NumRows; ++row)
            {
                v[row] = operator()(row, col);
            }
            return v;
        }

        // Set all elements to the specified value.
        void fill(T const& value)
        {
            std::fill(mContainer.begin(), mContainer.end(), value);
        }

        static Matrix<T, NumRows, NumCols> Zero()
        {
            Matrix<T, NumRows, NumCols> zero{};
            return zero;
        }

        static Matrix<T, NumRows, NumCols> Identity()
        {
            Matrix<T, NumRows, NumCols> identity{};
            size_t constexpr numDiagonal = std::min(NumRows, NumCols);
            for (size_t d = 0; d < numDiagonal; ++d)
            {
                identity(d, d) = C_<T>(1);
            }
            return identity;
        }

    private:
        std::array<T, NumRows * NumCols> mContainer;

        friend class UnitTestMatrix;
    };
}

// Implementation for matrices whose sizes are known only at run time.
namespace gtl
{
    template <typename T>
    class Matrix<T> : public MatrixTraits<T, 0, 0>
    {
    public:
        // Construction and destruction.

        // Create an empty matrix or a non-empty matrix whose elements are
        // initialized to 0.
        Matrix(size_t numRows = 0, size_t numCols = 0)
            :
            mNumRows(numRows),
            mNumCols(numCols),
            mContainer(numRows * numCols, C_<T>(0))
        {
            GTL_ARGUMENT_ASSERT(
                (numRows > 0 && numCols > 0) || (numRows == 0 && numCols == 0),
                "Invalid number of rows or columns.");
        }

        // Create a matrix from a list of rows.
        Matrix(std::initializer_list<std::initializer_list<T>> const& elements)
            :
            mNumRows(0),
            mNumCols(0),
            mContainer{}
        {
            mNumRows = elements.size();
            GTL_ARGUMENT_ASSERT(
                mNumRows > 0,
                "Invalid row initializer_list size.");

            mNumCols = elements.begin()->size();
            GTL_ARGUMENT_ASSERT(
                mNumCols > 0,
                "Invalid col initializer_list size.");

            mContainer.resize(mNumRows * mNumCols);

            size_t offset = 0;
            for (auto const& rowElements : elements)
            {
                GTL_ARGUMENT_ASSERT(
                    rowElements.size() == mNumCols,
                    "Invalid length for col initializer list.");

                std::copy(rowElements.begin(), rowElements.end(), mContainer.begin() + offset);
                offset += mNumCols;
            }
        }

        // Resize the matrix to support deferred construction.
        void resize(size_t numRows, size_t numCols)
        {
            GTL_LENGTH_ASSERT(
                (numRows > 0 && numCols > 0) || (numRows == 0 && numCols == 0),
                "Invalid number of rows or columns.");

            mNumRows = numRows;
            mNumCols = numCols;
            mContainer.resize(mNumRows * mNumCols);
        }

        ~Matrix() = default;


        // Copy semantics.
        Matrix(Matrix const& other)
            :
            mNumRows(0),
            mNumCols(0),
            mContainer{}
        {
            *this = other;
        }

        Matrix& operator=(Matrix const& other)
        {
            mNumRows = other.mNumRows;
            mNumCols = other.mNumCols;
            mContainer = other.mContainer;
            return *this;
        }

        // Move semantics.
        Matrix(Matrix&& other) noexcept
            :
            mNumRows(0),
            mNumCols(0),
            mContainer{}
        {
            *this = std::move(other);
        }

        Matrix& operator=(Matrix&& other) noexcept
        {
            mNumRows = other.mNumRows;
            mNumCols = other.mNumCols;
            mContainer = std::move(other.mContainer);
            other.mNumRows = 0;
            other.mNumCols = 0;
            return *this;
        }

        // Size and data access.
        inline size_t const size() const noexcept
        {
            return mNumRows * mNumCols;
        }

        inline size_t const GetNumRows() const noexcept
        {
            return mNumRows;
        }

        inline size_t const GetNumCols() const noexcept
        {
            return mNumCols;
        }

        size_t GetIndex(size_t row, size_t col) const noexcept
        {
            return col + mNumCols * row;
        }

        inline T const* data() const noexcept
        {
            return mContainer.data();
        }

        inline T* data() noexcept
        {
            return mContainer.data();
        }

        // Container access.
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

        inline T const& at(size_t row, size_t col) const
        {
            return at(GetIndex(row, col));
        }

        inline T& at(size_t row, size_t col)
        {
            return at(GetIndex(row, col));
        }

        inline T const& operator() (size_t row, size_t col) const
        {
            return mContainer[GetIndex(row, col)];
        }

        inline T& operator() (size_t row, size_t col)
        {
            return mContainer[GetIndex(row, col)];
        }

        void SetRow(size_t row, Vector<T> const& v)
        {
            GTL_OUTOFRANGE_ASSERT(
                v.size() == mNumCols && row < mNumRows,
                "Invalid size or invalid row.");

            for (size_t col = 0; col < mNumCols; ++col)
            {
                operator()(row, col) = v[col];
            }
        }

        void SetCol(size_t col, Vector<T> const& v)
        {
            GTL_OUTOFRANGE_ASSERT(
                v.size() == mNumRows && col < mNumCols,
                "Invalid size or invalid column.");

            for (size_t row = 0; row < mNumRows; ++row)
            {
                operator()(row, col) = v[row];
            }
        }

        Vector<T> GetRow(size_t row) const
        {
            GTL_OUTOFRANGE_ASSERT(
                row < mNumRows,
                "Invalid row.");

            Vector<T> v(mNumCols);
            for (size_t col = 0; col < mNumCols; ++col)
            {
                v[col] = operator()(row, col);
            }
            return v;
        }

        Vector<T> GetCol(size_t col) const
        {
            GTL_OUTOFRANGE_ASSERT(
                col < mNumCols,
                "Invalid column.");

            Vector<T> v(mNumRows);
            for (size_t row = 0; row < mNumRows; ++row)
            {
                v[row] = operator()(row, col);
            }
            return v;
        }

        // Set all elements to the specified value.
        void fill(T const& value)
        {
            std::fill(mContainer.begin(), mContainer.end(), value);
        }

        static Matrix<T> Zero(size_t numRows, size_t numCols)
        {
            Matrix<T> zero(numRows, numCols);
            return zero;
        }

        static Matrix<T> Identity(size_t numRows, size_t numCols)
        {
            Matrix<T> identity(numRows, numCols);
            MakeIdentity(identity);
            return identity;
        }

    private:
        size_t mNumRows, mNumCols;
        std::vector<T> mContainer;

        friend class UnitTestMatrix;
    };
}

// Operations for Matrix classes whose sizes are known at compile time.
namespace gtl
{
    // Allow sorting and comparing of objects.
    template <typename T, size_t NumRows, size_t NumCols>
    bool operator==(Matrix<T, NumRows, NumCols> const& M0, Matrix<T, NumRows, NumCols> const& M1)
    {
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                if (M0(row, col) != M1(row, col))
                {
                    return false;
                }
            }
        }
        return true;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    bool operator!=(Matrix<T, NumRows, NumCols> const& M0, Matrix<T, NumRows, NumCols> const& M1)
    {
        return !operator==(M0, M1);
    }

    template <typename T, size_t NumRows, size_t NumCols>
    bool operator<(Matrix<T, NumRows, NumCols> const& M0, Matrix<T, NumRows, NumCols> const& M1)
    {
        // The comparison is based on traversing the matrix entries stored
        // in row-major order.
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                T const& e0 = M0(row, col);
                T const& e1 = M1(row, col);
                if (e0 < e1)
                {
                    return true;
                }
                if (e1 < e0)
                {
                    return false;
                }
            }
        }
        return false;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    bool operator<=(Matrix<T, NumRows, NumCols> const& M0, Matrix<T, NumRows, NumCols> const& M1)
    {
        return !operator<(M1, M0);
    }

    template <typename T, size_t NumRows, size_t NumCols>
    bool operator>(Matrix<T, NumRows, NumCols> const& M0, Matrix<T, NumRows, NumCols> const& M1)
    {
        return operator<(M1, M0);
    }

    template <typename T, size_t NumRows, size_t NumCols>
    bool operator>=(Matrix<T, NumRows, NumCols> const& M0, Matrix<T, NumRows, NumCols> const& M1)
    {
        return !operator<(M0, M1);
    }

    // Set all matrix elements to 0.
    template <typename T, size_t NumRows, size_t NumCols>
    void MakeZero(Matrix<T, NumRows, NumCols>& M)
    {
        M.fill(C_<T>(0));
    }

    // Test whether the matrix is the zero matrix.
    template <typename T, size_t NumRows, size_t NumCols>
    bool IsZero(Matrix<T, NumRows, NumCols> const& M)
    {
        for (size_t i = 0; i < M.size(); ++i)
        {
            if (M[i] != C_<T>(0))
            {
                return false;
            }
        }
        return true;
    }

    // For 0 <= row < numRows and 0 <= col < numCols, element (row,col) is 1
    // and all other elements are 0. If either of row or col is invalid, the
    // zero matrix is created. This function is a convenience for creating the
    // standard Euclidean basis for matrices.
    template <typename T, size_t NumRows, size_t NumCols>
    void MakeBasis(size_t row, size_t col, Matrix<T, NumRows, NumCols>& M)
    {
        GTL_LENGTH_ASSERT(
            row < NumRows && col < NumCols,
            "Invalid row or column.");

        M.fill(C_<T>(0));
        M(row, col) = C_<T>(1);
    }

    // Test whether the matrix is the basis matrix whose (rowQuery,colQuery)
    // element is 1 and all other elements are 0.
    template <typename T, size_t NumRows, size_t NumCols>
    bool IsBasis(size_t rowQuery, size_t colQuery, Matrix<T, NumRows, NumCols> const& M)
    {
        GTL_LENGTH_ASSERT(
            rowQuery < NumRows && colQuery < NumCols,
            "Invalid row or column.");

        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                if (row != rowQuery || col != colQuery)
                {
                    if (M(row, col) != C_<T>(0))
                    {
                        return false;
                    }
                }
                else
                {
                    if (M(row, col) != C_<T>(1))
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // Create the identity matrix.
    template <typename T, size_t N>
    void MakeIdentity(Matrix<T, N, N>& M)
    {
        M.fill(C_<T>(0));
        for (size_t i = 0; i < N; ++i)
        {
            M(i, i) = C_<T>(1);
        }
    }

    // Test whether the matrix is the identity matrix.
    template <typename T, size_t N>
    bool IsIdentity(Matrix<T, N, N> const& M)
    {
        for (size_t row = 0; row < N; ++row)
        {
            for (size_t col = 0; col < N; ++col)
            {
                if (row != col)
                {
                    if (M(row, col) != C_<T>(0))
                    {
                        return false;
                    }
                }
                else
                {
                    if (M(row, col) != C_<T>(1))
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // Create a diagonal matrix.
    template <typename T, size_t N>
    void MakeDiagonal(std::array<T, N> const& diagonal, Matrix<T, N, N>& M)
    {
        M.fill(C_<T>(0));
        for (size_t i = 0; i < N; ++i)
        {
            M(i, i) = diagonal[i];
        }
    }

    // Test whether the matrix is a diagonal matrix.
    template <typename T, size_t N>
    bool IsDiagonal(Matrix<T, N, N> const& M)
    {
        for (size_t row = 0; row < N; ++row)
        {
            for (size_t col = 0; col < N; ++col)
            {
                if (row != col)
                {
                    if (M(row, col) != C_<T>(0))
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // Create the outer product u0 * Transpose(u1).
    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols> OuterProduct(Vector<T, NumRows> const& v0, Vector<T, NumCols> const& v1)
    {
        Matrix<T, NumRows, NumCols> result{};
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                result(row, col) = v0[row] * v1[col];
            }
        }
        return result;
    }

    // Unary operations.
    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols> operator+(Matrix<T, NumRows, NumCols> const& M)
    {
        return M;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols> operator-(Matrix<T, NumRows, NumCols> const& M)
    {
        Matrix<T, NumRows, NumCols> result{};
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            result[i] = -M[i];
        }
        return result;
    }

    // Linear-algebraic operations.
    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols> operator+(Matrix<T, NumRows, NumCols> const& M0, Matrix<T, NumRows, NumCols> const& M1)
    {
        Matrix<T, NumRows, NumCols> result = M0;
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            result[i] += M1[i];
        }
        return result;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols>& operator+=(Matrix<T, NumRows, NumCols>& M0, Matrix<T, NumRows, NumCols> const& M1)
    {
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            M0[i] += M1[i];
        }
        return M0;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols> operator-(Matrix<T, NumRows, NumCols> const& M0, Matrix<T, NumRows, NumCols> const& M1)
    {
        Matrix<T, NumRows, NumCols> result = M0;
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            result[i] -= M1[i];
        }
        return result;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols>& operator-=(Matrix<T, NumRows, NumCols>& M0, Matrix<T, NumRows, NumCols> const& M1)
    {
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            M0[i] -= M1[i];
        }
        return M0;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols> operator*(Matrix<T, NumRows, NumCols> const& M, T const& scalar)
    {
        Matrix<T, NumRows, NumCols> result = M;
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            result[i] *= scalar;
        }
        return result;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols> operator*(T const& scalar, Matrix<T, NumRows, NumCols> const& M)
    {
        Matrix<T, NumRows, NumCols> result = M;
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            result[i] *= scalar;
        }
        return result;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols>& operator*=(Matrix<T, NumRows, NumCols>& M, T const& scalar)
    {
        M = std::move(M * scalar);
        return M;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols> operator/(Matrix<T, NumRows, NumCols> const& M, T const& scalar)
    {
        Matrix<T, NumRows, NumCols> result = M;
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            result[i] /= scalar;
        }
        return result;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols>& operator/=(Matrix<T, NumRows, NumCols>& M, T const& scalar)
    {
        M = std::move(M / scalar);
        return M;
    }

    // Geometric operations.
    template <typename T, size_t NumRows, size_t NumCols>
    T L1Norm(Matrix<T, NumRows, NumCols> const& M)
    {
        T sum = C_<T>(0);
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            sum += std::fabs(M[i]);
        }
        return sum;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    T L2Norm(Matrix<T, NumRows, NumCols> const& M)
    {
        T sum = C_<T>(0);
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            sum += M[i] * M[i];
        }
        return std::sqrt(sum);
    }

    template <typename T, size_t NumRows, size_t NumCols>
    T LInfinityNorm(Matrix<T, NumRows, NumCols> const& M)
    {
        T maxAbsElement = C_<T>(0);
        for (size_t i = 0; i < NumRows * NumCols; ++i)
        {
            T absElement = std::fabs(M[i]);
            if (absElement > maxAbsElement)
            {
                maxAbsElement = absElement;
            }
        }
        return maxAbsElement;
    }

    template <typename T, size_t N>
    T Trace(Matrix<T, N, N> const& M)
    {
        T trace = C_<T>(0);
        for (size_t i = 0; i < N; ++i)
        {
            trace += M(i, i);
        }
        return trace;
    }

    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumCols, NumRows> Transpose(Matrix<T, NumRows, NumCols> const& M)
    {
        Matrix<T, NumCols, NumRows> result{};
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                result(col, row) = M(row, col);
            }
        }
        return result;
    }

    // Inverse and determinant operations for square matrices are found in
    // GTL/Mathematics/NumericalMethods/LinearSystems/GaussianElimination.h.


    // Matrix-vector and matrix-matrix multiplications.

    // M * v
    template <typename T, size_t NumRows, size_t NumCols>
    Vector<T, NumRows> operator*(Matrix<T, NumRows, NumCols> const& M, Vector<T, NumCols> const& v)
    {
        Vector<T, NumRows> result{};
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                result[row] += M(row, col) * v[col];
            }
        }
        return result;
    }

    // v * M
    template <typename T, size_t NumRows, size_t NumCols>
    Vector<T, NumCols> operator*(Vector<T, NumRows> const& v, Matrix<T, NumRows, NumCols> const& M)
    {
        Vector<T, NumCols> result{};
        for (size_t col = 0; col < NumCols; ++col)
        {
            for (size_t row = 0; row < NumRows; ++row)
            {
                result[col] += v[row] * M(row, col);
            }
        }
        return result;
    }

    // M0 * M1
    template <typename T, size_t NumRows, size_t NumCols, size_t NumCommon>
    Matrix<T, NumRows, NumCols> operator*(
        Matrix<T, NumRows, NumCommon> const& M0,
        Matrix<T, NumCommon, NumCols> const& M1)
    {
        return MultiplyAB(M0, M1);
    }

    template <typename T, size_t NumRows, size_t NumCols, size_t NumCommon>
    Matrix<T, NumRows, NumCols> MultiplyAB(
        Matrix<T, NumRows, NumCommon> const& M0,
        Matrix<T, NumCommon, NumCols> const& M1)
    {
        Matrix<T, NumRows, NumCols> result{};
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                for (size_t i = 0; i < NumCommon; ++i)
                {
                    result(row, col) += M0(row, i) * M1(i, col);
                }
            }
        }
        return result;
    }

    // M0 * M1^T
    template <typename T, size_t NumRows, size_t NumCols, size_t NumCommon>
    Matrix<T, NumRows, NumCols> MultiplyABT(
        Matrix<T, NumRows, NumCommon> const& M0,
        Matrix<T, NumCols, NumCommon> const& M1)
    {
        Matrix<T, NumRows, NumCols> result{};
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                for (size_t i = 0; i < NumCommon; ++i)
                {
                    result(row, col) += M0(row, i) * M1(col, i);
                }
            }
        }
        return result;
    }

    // M0^T * M1
    template <typename T, size_t NumRows, size_t NumCols, size_t NumCommon>
    Matrix<T, NumRows, NumCols> MultiplyATB(
        Matrix<T, NumCommon, NumRows> const& M0,
        Matrix<T, NumCommon, NumCols> const& M1)
    {
        Matrix<T, NumRows, NumCols> result{};
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                for (size_t i = 0; i < NumCommon; ++i)
                {
                    result(row, col) += M0(i, row) * M1(i, col);
                }
            }
        }
        return result;
    }

    // M0^T * M1^T
    template <typename T, size_t NumRows, size_t NumCols, size_t NumCommon>
    Matrix<T, NumRows, NumCols> MultiplyATBT(
        Matrix<T, NumCommon, NumRows> const& M0,
        Matrix<T, NumCols, NumCommon> const& M1)
    {
        Matrix<T, NumRows, NumCols> result{};
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                for (size_t i = 0; i < NumCommon; ++i)
                {
                    result(row, col) += M0(i, row) * M1(col, i);
                }
            }
        }
        return result;
    }

    // M * D, D is diagonal NumCols-by-NumCols stored as a 1D array
    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols> MultiplyMD(
        Matrix<T, NumRows, NumCols> const& M,
        std::array<T, NumCols> const& D)
    {
        Matrix<T, NumRows, NumCols> result{};
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                result(row, col) = M(row, col) * D[col];
            }
        }
        return result;
    }

    // D * M, D is diagonal NumRows-by-NumRows stored as a 1D array
    template <typename T, size_t NumRows, size_t NumCols>
    Matrix<T, NumRows, NumCols> MultiplyDM(
        std::array<T, NumRows> const& D,
        Matrix<T, NumRows, NumCols> const& M)
    {
        Matrix<T, NumRows, NumCols> result{};
        for (size_t row = 0; row < NumRows; ++row)
        {
            for (size_t col = 0; col < NumCols; ++col)
            {
                result(row, col) = D[row] * M(row, col);
            }
        }
        return result;
    }

    // Create an (N+1)-by-(N+1) matrix H by setting the upper N-by-N block to
    // the input N-by-N matrix and all other entries to 0 except for the last
    // row and last column entry which is set to 1.
    template <typename T, size_t N>
    Matrix<T, N + 1, N + 1> HLift(Matrix<T, N, N> const& M)
    {
        Matrix<T, N + 1, N + 1> result{};
        for (size_t row = 0; row < N; ++row)
        {
            for (size_t col = 0; col < N; ++col)
            {
                result(row, col) = M(row, col);
            }
            result(row, N) = C_<T>(0);
        }
        for (size_t col = 0; col < N; ++col)
        {
            result(N, col) = C_<T>(0);
        }
        result(N, N) = C_<T>(1);
        return result;
    }

    // Extract the upper (N-1)-by-(N-1) block of the input N-by-N matrix.
    template <typename T, size_t N>
    Matrix<T, N - 1, N - 1> HProject(Matrix<T, N, N> const& M)
    {
        static_assert(
            N > 1,
            "Invalid dimension for a projection.");

        Matrix<T, N - 1, N - 1> result{};
        for (size_t row = 0; row < N - 1; ++row)
        {
            for (size_t col = 0; col < N - 1; ++col)
            {
                result(row, col) = M(row, col);
            }
        }
        return result;
    }
}

// Operations for Matrix classes whose sizes are known only at run time.
namespace gtl
{
    // Allow sorting and comparing of objects.
    template <typename T>
    bool operator==(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        GTL_LENGTH_ASSERT(
            M0.GetNumRows() == M1.GetNumRows() &&
            M0.GetNumCols() == M1.GetNumCols(),
            "Mismatched sizes.");

        for (size_t row = 0; row < M0.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < M0.GetNumCols(); ++col)
            {
                if (M0(row, col) != M1(row, col))
                {
                    return false;
                }
            }
        }
        return true;
    }

    template <typename T>
    bool operator!=(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        return !operator==(M0, M1);
    }

    template <typename T>
    bool operator<(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        GTL_LENGTH_ASSERT(
            M0.GetNumRows() == M1.GetNumRows() &&
            M0.GetNumCols() == M1.GetNumCols(),
            "Mismatched sizes.");

        // The comparison is based on traversing the matrix entries stored
        // in row-major order.
        for (size_t row = 0; row < M0.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < M0.GetNumCols(); ++col)
            {
                T const& e0 = M0(row, col);
                T const& e1 = M1(row, col);
                if (e0 < e1)
                {
                    return true;
                }
                if (e1 < e0)
                {
                    return false;
                }
            }
        }
        return false;
    }

    template <typename T>
    bool operator<=(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        return !operator<(M1, M0);
    }

    template <typename T>
    bool operator>(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        return operator<(M1, M0);
    }

    template <typename T>
    bool operator>=(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        return !operator<(M0, M1);
    }

    // Set all matrix elements to 0.
    template <typename T>
    void MakeZero(Matrix<T>& M)
    {
        M.fill(C_<T>(0));
    }

    // Test whether the matrix is the zero matrix.
    template <typename T>
    bool IsZero(Matrix<T> const& M)
    {
        if (M.size() == 0)
        {
            return false;
        }

        for (size_t i = 0; i < M.size(); ++i)
        {
            if (M[i] != C_<T>(0))
            {
                return false;
            }
        }
        return true;
    }

    // For 0 <= row < numRows and 0 <= col < numCols, element (row,col) is 1
    // and all other elements are 0. If either of row or col is invalid, the
    // zero matrix is created. This function is a convenience for creating the
    // standard Euclidean basis for matrices.
    template <typename T>
    void MakeBasis(size_t row, size_t col, Matrix<T>& M)
    {
        GTL_LENGTH_ASSERT(
            row < M.GetNumRows() && col < M.GetNumCols(),
            "Invalid row or column.");

        M.fill(C_<T>(0));
        M(row, col) = C_<T>(1);
    }

    // Test whether the matrix is the unit matrix whose (rowQuery,colQuery)
    // element is 1 and all other elements are 0.
    template <typename T>
    bool IsBasis(size_t rowQuery, size_t colQuery, Matrix<T> const& M)
    {
        GTL_LENGTH_ASSERT(
            rowQuery < M.GetNumRows() && colQuery < M.GetNumCols(),
            "Invalid row or column.");

        for (size_t row = 0; row < M.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < M.GetNumCols(); ++col)
            {
                if (row != rowQuery || col != colQuery)
                {
                    if (M(row, col) != C_<T>(0))
                    {
                        return false;
                    }
                }
                else
                {
                    if (M(row, col) != C_<T>(1))
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // Create the identity matrix.
    template <typename T>
    void MakeIdentity(Matrix<T>& M)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumRows() > 0 && M.GetNumRows() == M.GetNumCols(),
            "Matrix must be square.");

        M.fill(C_<T>(0));
        for (size_t i = 0; i < M.GetNumRows(); ++i)
        {
            M(i, i) = C_<T>(1);
        }
    }

    // Test whether the matrix is the identity matrix.
    template <typename T>
    bool IsIdentity(Matrix<T> const& M)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumRows() > 0 && M.GetNumRows() == M.GetNumCols(),
            "Matrix must be square.");

        for (size_t row = 0; row < M.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < M.GetNumCols(); ++col)
            {
                if (row != col)
                {
                    if (M(row, col) != C_<T>(0))
                    {
                        return false;
                    }
                }
                else
                {
                    if (M(row, col) != C_<T>(1))
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // Create a diagonal matrix.
    template <typename T>
    void MakeDiagonal(std::vector<T> const& diagonal, Matrix<T>& M)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumRows() > 0 && M.GetNumRows() == M.GetNumCols(),
            "Matrix must be square.");

        GTL_LENGTH_ASSERT(
            diagonal.size() == M.GetNumRows(),
            "Mismatch in diagonal and matrix size.");

        M.fill(C_<T>(0));
        for (size_t i = 0; i < M.GetNumRows(); ++i)
        {
            M(i, i) = diagonal[i];
        }
    }

    // Test whether the matrix is a diagonal matrix.
    template <typename T>
    bool IsDiagonal(Matrix<T> const& M)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumRows() > 0 && M.GetNumRows() == M.GetNumCols(),
            "Matrix must be square.");

        for (size_t row = 0; row < M.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < M.GetNumCols(); ++col)
            {
                if (row != col)
                {
                    if (M(row, col) != C_<T>(0))
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // Create the outer product u0 * Transpose(u1).
    template <typename T>
    Matrix<T> OuterProduct(Vector<T> const& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() > 0 && v1.size() > 0,
            "Dimensions must be positive.");

        Matrix<T> result(v0.size(), v1.size());
        for (size_t row = 0; row < v0.size(); ++row)
        {
            for (size_t col = 0; col < v1.size(); ++col)
            {
                result(row, col) = v0[row] * v1[col];
            }
        }
        return result;
    }

    // Unary operations.
    template <typename T>
    Matrix<T> operator+(Matrix<T> const& M)
    {
        return M;
    }

    template <typename T>
    Matrix<T> operator-(Matrix<T> const& M)
    {
        Matrix<T> result(M.GetNumRows(), M.GetNumCols());
        for (size_t i = 0; i < result.size(); ++i)
        {
            result[i] = -M[i];
        }
        return result;
    }

    // Linear-algebraic operations.
    template <typename T>
    Matrix<T> operator+(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        GTL_LENGTH_ASSERT(
            M0.GetNumRows() == M1.GetNumRows() &&
            M0.GetNumCols() == M1.GetNumCols(),
            "Mismatched sizes.");

        Matrix<T> result = M0;
        for (size_t i = 0; i < result.size(); ++i)
        {
            result[i] += M1[i];
        }
        return result;
    }

    template <typename T>
    Matrix<T>& operator+=(Matrix<T>& M0, Matrix<T> const& M1)
    {
        GTL_LENGTH_ASSERT(
            M0.GetNumRows() == M1.GetNumRows() &&
            M0.GetNumCols() == M1.GetNumCols(),
            "Mismatched sizes.");

        for (size_t i = 0; i < M0.size(); ++i)
        {
            M0[i] += M1[i];
        }
        return M0;
    }

    template <typename T>
    Matrix<T> operator-(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        GTL_LENGTH_ASSERT(
            M0.GetNumRows() == M1.GetNumRows() &&
            M0.GetNumCols() == M1.GetNumCols(),
            "Mismatched sizes.");

        Matrix<T> result = M0;
        for (size_t i = 0; i < result.size(); ++i)
        {
            result[i] -= M1[i];
        }
        return result;
    }

    template <typename T>
    Matrix<T>& operator-=(Matrix<T>& M0, Matrix<T> const& M1)
    {
        GTL_LENGTH_ASSERT(
            M0.GetNumRows() == M1.GetNumRows() &&
            M0.GetNumCols() == M1.GetNumCols(),
            "Mismatched sizes.");

        for (size_t i = 0; i < M0.size(); ++i)
        {
            M0[i] -= M1[i];
        }
        return M0;
    }

    template <typename T>
    Matrix<T> operator*(Matrix<T> const& M, T const& scalar)
    {
        Matrix<T> result = M;
        for (size_t i = 0; i < M.size(); ++i)
        {
            result[i] *= scalar;
        }
        return result;
    }

    template <typename T>
    Matrix<T> operator*(T const& scalar, Matrix<T> const& M)
    {
        Matrix<T> result = M;
        for (size_t i = 0; i < M.size(); ++i)
        {
            result[i] *= scalar;
        }
        return result;
    }

    template <typename T>
    Matrix<T>& operator*=(Matrix<T>& M, T const& scalar)
    {
        M = std::move(M * scalar);
        return M;
    }

    template <typename T>
    Matrix<T> operator/(Matrix<T> const& M, T const& scalar)
    {
        Matrix<T> result = M;
        for (size_t i = 0; i < M.size(); ++i)
        {
            result[i] /= scalar;
        }
        return result;
    }

    template <typename T>
    Matrix<T>& operator/=(Matrix<T>& M, T const& scalar)
    {
        M = std::move(M / scalar);
        return M;
    }

    // Geometric operations.
    template <typename T>
    T L1Norm(Matrix<T> const& M)
    {
        T sum = C_<T>(0);
        for (size_t i = 0; i < M.size(); ++i)
        {
            sum += std::fabs(M[i]);
        }
        return sum;
    }

    template <typename T>
    T L2Norm(Matrix<T> const& M)
    {
        T sum = C_<T>(0);
        for (size_t i = 0; i < M.size(); ++i)
        {
            sum += M[i] * M[i];
        }
        return std::sqrt(sum);
    }

    template <typename T>
    T LInfinityNorm(Matrix<T> const& M)
    {
        T maxAbsElement = C_<T>(0);
        for (size_t i = 0; i < M.size(); ++i)
        {
            T absElement = std::fabs(M[i]);
            if (absElement > maxAbsElement)
            {
                maxAbsElement = absElement;
            }
        }
        return maxAbsElement;
    }

    template <typename T>
    T Trace(Matrix<T> const& M)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumRows() == M.GetNumCols(),
            "Matrix must be square.");

        T trace = C_<T>(0);
        for (size_t i = 0; i < M.GetNumRows(); ++i)
        {
            trace += M(i, i);
        }
        return trace;
    }

    template <typename T>
    Matrix<T> Transpose(Matrix<T> const& M)
    {
        Matrix<T> result(M.GetNumCols(), M.GetNumRows());
        for (size_t row = 0; row < M.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < M.GetNumCols(); ++col)
            {
                result(col, row) = M(row, col);
            }
        }
        return result;
    }

    // Inverse and determinant operations for square matrices are found in
    // GTL/Mathematics/NumericalMethods/LinearSystems/GaussianElimination.h.


    // Matrix-vector and matrix-matrix multiplications.

    // M * v
    template <typename T>
    Vector<T> operator*(Matrix<T> const& M, Vector<T> const& v)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumCols() == v.size(),
            "Mismatched sizes.");

        Vector<T> result(M.GetNumRows());
        for (size_t row = 0; row < M.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < M.GetNumCols(); ++col)
            {
                result[row] += M(row, col) * v[col];
            }
        }
        return result;
    }

    // v * M
    template <typename T>
    Vector<T> operator*(Vector<T> const& v, Matrix<T> const& M)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumRows() == v.size(),
            "Mismatched sizes.");

        Vector<T> result(M.GetNumCols());
        for (size_t col = 0; col < M.GetNumCols(); ++col)
        {
            for (size_t row = 0; row < M.GetNumRows(); ++row)
            {
                result[col] += v[row] * M(row, col);
            }
        }
        return result;
    }

    // M0 * M1
    template <typename T>
    Matrix<T> operator*(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        return MultiplyAB(M0, M1);
    }

    template <typename T>
    Matrix<T> MultiplyAB(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        GTL_LENGTH_ASSERT(
            M0.GetNumCols() > 0 && M0.GetNumCols() == M1.GetNumRows(),
            "Mismatched sizes.");

        Matrix<T> result(M0.GetNumRows(), M1.GetNumCols());
        size_t const numCommon = M0.GetNumCols();
        for (size_t row = 0; row < result.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < result.GetNumCols(); ++col)
            {
                for (size_t i = 0; i < numCommon; ++i)
                {
                    result(row, col) += M0(row, i) * M1(i, col);
                }
            }
        }
        return result;
    }

    // M0 * M1^T
    template <typename T>
    Matrix<T> MultiplyABT(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        GTL_LENGTH_ASSERT(
            M0.GetNumCols() > 0 && M0.GetNumCols() == M1.GetNumCols(),
            "Mismatched sizes.");

        Matrix<T> result(M0.GetNumRows(), M1.GetNumRows());
        size_t const numCommon = M0.GetNumCols();
        for (size_t row = 0; row < result.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < result.GetNumCols(); ++col)
            {
                for (size_t i = 0; i < numCommon; ++i)
                {
                    result(row, col) += M0(row, i) * M1(col, i);
                }
            }
        }
        return result;
    }

    // M0^T * M1
    template <typename T>
    Matrix<T> MultiplyATB(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        GTL_LENGTH_ASSERT(
            M0.GetNumRows() > 0 && M0.GetNumRows() == M1.GetNumRows(),
            "Mismatched sizes.");

        Matrix<T> result(M0.GetNumCols(), M1.GetNumCols());
        size_t const numCommon = M0.GetNumRows();
        for (size_t row = 0; row < result.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < result.GetNumCols(); ++col)
            {
                for (size_t i = 0; i < numCommon; ++i)
                {
                    result(row, col) += M0(i, row) * M1(i, col);
                }
            }
        }
        return result;
    }

    // M0^T * M1^T
    template <typename T>
    Matrix<T> MultiplyATBT(Matrix<T> const& M0, Matrix<T> const& M1)
    {
        GTL_LENGTH_ASSERT(
            M0.GetNumRows() > 0 && M0.GetNumRows() == M1.GetNumCols(),
            "Mismatched sizes.");

        Matrix<T> result(M0.GetNumCols(), M1.GetNumRows());
        size_t const numCommon = M0.GetNumRows();
        for (size_t row = 0; row < result.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < result.GetNumCols(); ++col)
            {
                for (size_t i = 0; i < numCommon; ++i)
                {
                    result(row, col) += M0(i, row) * M1(col, i);
                }
            }
        }
        return result;
    }

    // M * D, D is diagonal NumCols-by-NumCols stored as a 1D array
    template <typename T>
    Matrix<T> MultiplyMD(Matrix<T> const& M, std::vector<T> const& D)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumCols() == D.size(),
            "Mismatched sizes.");

        Matrix<T> result(M.GetNumRows(), M.GetNumCols());
        for (size_t row = 0; row < M.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < M.GetNumCols(); ++col)
            {
                result(row, col) = M(row, col) * D[col];
            }
        }
        return result;
    }

    // D * M, D is diagonal NumRows-by-NumRows stored as a 1D array
    template <typename T>
    Matrix<T> MultiplyDM(std::vector<T> const& D, Matrix<T> const& M)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumRows() == D.size(),
            "Mismatched sizes.");

        Matrix<T> result(M.GetNumRows(), M.GetNumCols());
        for (size_t row = 0; row < M.GetNumRows(); ++row)
        {
            for (size_t col = 0; col < M.GetNumCols(); ++col)
            {
                result(row, col) = D[row] * M(row, col);
            }
        }
        return result;
    }

    // Create an (N+1)-by-(N+1) matrix H by setting the upper N-by-N block to
    // the input N-by-N matrix and all other entries to 0 except for the last
    // row and last column entry which is set to 1.
    template <typename T>
    Matrix<T> HLift(Matrix<T> const& M)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumRows() > 0 && M.GetNumRows() == M.GetNumCols(),
            "Mismatched sizes.");

        size_t const numRows = M.GetNumRows();
        Matrix<T> result(numRows + 1, numRows + 1);
        for (size_t row = 0; row < numRows; ++row)
        {
            for (size_t col = 0; col < numRows; ++col)
            {
                result(row, col) = M(row, col);
            }
            result(row, numRows) = C_<T>(0);
        }
        for (size_t col = 0; col < numRows; ++col)
        {
            result(numRows, col) = C_<T>(0);
        }
        result(numRows, numRows) = C_<T>(1);
        return result;
    }

    // Extract the upper (N-1)-by-(N-1) block of the input N-by-N matrix.
    template <typename T>
    Matrix<T> HProject(Matrix<T> const& M)
    {
        GTL_LENGTH_ASSERT(
            M.GetNumRows() > 1 && M.GetNumRows() == M.GetNumCols(),
            "Mismatched sizes.");

        size_t const numRowsM1 = M.GetNumRows() - 1;
        Matrix<T> result(numRowsM1, numRowsM1);
        for (size_t row = 0; row < numRowsM1; ++row)
        {
            for (size_t col = 0; col < numRowsM1; ++col)
            {
                result(row, col) = M(row, col);
            }
        }
        return result;
    }
}

// Additional support for 2x2 matrices.
namespace gtl
{
    // Template alias for convenience.
    template <typename T> using Matrix2x2 = Matrix<T, 2, 2>;

    // Geometric operations.
    template <typename T>
    Matrix2x2<T> GetInverse(Matrix2x2<T> const& M, T* determinant = nullptr)
    {
        Matrix2x2<T> inverse{};  // the zero matrix
        T locDeterminant = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        if (locDeterminant != C_<T>(0))
        {
            inverse(0, 0) = M(1, 1) / locDeterminant;
            inverse(0, 1) = -M(0, 1) / locDeterminant;
            inverse(1, 0) = -M(1, 0) / locDeterminant;
            inverse(1, 1) = M(0, 0) / locDeterminant;
        }

        if (determinant)
        {
            *determinant = locDeterminant;
        }

        return inverse;
    }

    template <typename T>
    Matrix2x2<T> GetAdjoint(Matrix2x2<T> const& M)
    {
        Matrix2x2<T> adjoint{};
        adjoint(0, 0) = M(1, 1);
        adjoint(0, 1) = -M(0, 1);
        adjoint(1, 0) = -M(1, 0);
        adjoint(1, 1) = M(0, 0);
        return adjoint;
    }

    template <typename T>
    T GetDeterminant(Matrix2x2<T> const& M)
    {
        T determinant = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        return determinant;
    }

    template <typename T>
    T GetTrace(Matrix2x2<T> const& M)
    {
        T trace = M(0, 0) + M(1, 1);
        return trace;
    }
}

// Additional support for 3x3 matrices.
namespace gtl
{
    // Template alias for convenience.
    template <typename T> using Matrix3x3 = Matrix<T, 3, 3>;

    // Geometric operations.
    template <typename T>
    Matrix3x3<T> GetInverse(Matrix3x3<T> const& M, T* determinant = nullptr)
    {
        Matrix3x3<T> inverse{}; // the zero matrix
        T c00 = M(1, 1) * M(2, 2) - M(1, 2) * M(2, 1);
        T c10 = M(1, 2) * M(2, 0) - M(1, 0) * M(2, 2);
        T c20 = M(1, 0) * M(2, 1) - M(1, 1) * M(2, 0);
        T locDeterminant = M(0, 0) * c00 + M(0, 1) * c10 + M(0, 2) * c20;
        if (locDeterminant != C_<T>(0))
        {
            inverse(0, 0) = c00 / locDeterminant;
            inverse(0, 1) = (M(0, 2) * M(2, 1) - M(0, 1) * M(2, 2)) / locDeterminant;
            inverse(0, 2) = (M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1)) / locDeterminant;
            inverse(1, 0) = c10 / locDeterminant;
            inverse(1, 1) = (M(0, 0) * M(2, 2) - M(0, 2) * M(2, 0)) / locDeterminant;
            inverse(1, 2) = (M(0, 2) * M(1, 0) - M(0, 0) * M(1, 2)) / locDeterminant;
            inverse(2, 0) = c20 / locDeterminant;
            inverse(2, 1) = (M(0, 1) * M(2, 0) - M(0, 0) * M(2, 1)) / locDeterminant;
            inverse(2, 2) = (M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0)) / locDeterminant;
        }

        if (determinant)
        {
            *determinant = locDeterminant;
        }

        return inverse;
    }

    template <typename T>
    Matrix3x3<T> GetAdjoint(Matrix3x3<T> const& M)
    {
        Matrix3x3<T> adjoint{};
        adjoint(0, 0) = M(1, 1) * M(2, 2) - M(1, 2) * M(2, 1);
        adjoint(0, 1) = M(0, 2) * M(2, 1) - M(0, 1) * M(2, 2);
        adjoint(0, 2) = M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1);
        adjoint(1, 0) = M(1, 2) * M(2, 0) - M(1, 0) * M(2, 2);
        adjoint(1, 1) = M(0, 0) * M(2, 2) - M(0, 2) * M(2, 0);
        adjoint(1, 2) = M(0, 2) * M(1, 0) - M(0, 0) * M(1, 2);
        adjoint(2, 0) = M(1, 0) * M(2, 1) - M(1, 1) * M(2, 0);
        adjoint(2, 1) = M(0, 1) * M(2, 0) - M(0, 0) * M(2, 1);
        adjoint(2, 2) = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        return adjoint;
    }

    template <typename T>
    T GetDeterminant(Matrix3x3<T> const& M)
    {
        T c00 = M(1, 1) * M(2, 2) - M(1, 2) * M(2, 1);
        T c10 = M(1, 2) * M(2, 0) - M(1, 0) * M(2, 2);
        T c20 = M(1, 0) * M(2, 1) - M(1, 1) * M(2, 0);
        T determinant = M(0, 0) * c00 + M(0, 1) * c10 + M(0, 2) * c20;
        return determinant;
    }

    template <typename T>
    T GetTrace(Matrix3x3<T> const& M)
    {
        T trace = M(0, 0) + M(1, 1) + M(2, 2);
        return trace;
    }
}

// Additional support for 4x4 matrices.
namespace gtl
{
    // Template alias for convenience.
    template <typename T> using Matrix4x4 = Matrix<T, 4, 4>;

    // Geometric operations.
    template <typename T>
    Matrix4x4<T> GetInverse(Matrix4x4<T> const& M, T* determinant = nullptr)
    {
        Matrix4x4<T> inverse{}; // the zero matrix
        T a0 = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        T a1 = M(0, 0) * M(1, 2) - M(0, 2) * M(1, 0);
        T a2 = M(0, 0) * M(1, 3) - M(0, 3) * M(1, 0);
        T a3 = M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1);
        T a4 = M(0, 1) * M(1, 3) - M(0, 3) * M(1, 1);
        T a5 = M(0, 2) * M(1, 3) - M(0, 3) * M(1, 2);
        T b0 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0);
        T b1 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0);
        T b2 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0);
        T b3 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1);
        T b4 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1);
        T b5 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2);
        T locDeterminant = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
        if (locDeterminant != C_<T>(0))
        {
            inverse(0, 0) = (+M(1, 1) * b5 - M(1, 2) * b4 + M(1, 3) * b3) / locDeterminant;
            inverse(0, 1) = (-M(0, 1) * b5 + M(0, 2) * b4 - M(0, 3) * b3) / locDeterminant;
            inverse(0, 2) = (+M(3, 1) * a5 - M(3, 2) * a4 + M(3, 3) * a3) / locDeterminant;
            inverse(0, 3) = (-M(2, 1) * a5 + M(2, 2) * a4 - M(2, 3) * a3) / locDeterminant;
            inverse(1, 0) = (-M(1, 0) * b5 + M(1, 2) * b2 - M(1, 3) * b1) / locDeterminant;
            inverse(1, 1) = (+M(0, 0) * b5 - M(0, 2) * b2 + M(0, 3) * b1) / locDeterminant;
            inverse(1, 2) = (-M(3, 0) * a5 + M(3, 2) * a2 - M(3, 3) * a1) / locDeterminant;
            inverse(1, 3) = (+M(2, 0) * a5 - M(2, 2) * a2 + M(2, 3) * a1) / locDeterminant;
            inverse(2, 0) = (+M(1, 0) * b4 - M(1, 1) * b2 + M(1, 3) * b0) / locDeterminant;
            inverse(2, 1) = (-M(0, 0) * b4 + M(0, 1) * b2 - M(0, 3) * b0) / locDeterminant;
            inverse(2, 2) = (+M(3, 0) * a4 - M(3, 1) * a2 + M(3, 3) * a0) / locDeterminant;
            inverse(2, 3) = (-M(2, 0) * a4 + M(2, 1) * a2 - M(2, 3) * a0) / locDeterminant;
            inverse(3, 0) = (-M(1, 0) * b3 + M(1, 1) * b1 - M(1, 2) * b0) / locDeterminant;
            inverse(3, 1) = (+M(0, 0) * b3 - M(0, 1) * b1 + M(0, 2) * b0) / locDeterminant;
            inverse(3, 2) = (-M(3, 0) * a3 + M(3, 1) * a1 - M(3, 2) * a0) / locDeterminant;
            inverse(3, 3) = (+M(2, 0) * a3 - M(2, 1) * a1 + M(2, 2) * a0) / locDeterminant;
        }

        if (determinant)
        {
            *determinant = locDeterminant;
        }

        return inverse;
    }

    template <typename T>
    Matrix4x4<T> GetAdjoint(Matrix4x4<T> const& M)
    {
        T a0 = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        T a1 = M(0, 0) * M(1, 2) - M(0, 2) * M(1, 0);
        T a2 = M(0, 0) * M(1, 3) - M(0, 3) * M(1, 0);
        T a3 = M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1);
        T a4 = M(0, 1) * M(1, 3) - M(0, 3) * M(1, 1);
        T a5 = M(0, 2) * M(1, 3) - M(0, 3) * M(1, 2);
        T b0 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0);
        T b1 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0);
        T b2 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0);
        T b3 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1);
        T b4 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1);
        T b5 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2);

        Matrix4x4<T> adjoint{};
        adjoint(0, 0) = +M(1, 1) * b5 - M(1, 2) * b4 + M(1, 3) * b3;
        adjoint(0, 1) = -M(0, 1) * b5 + M(0, 2) * b4 - M(0, 3) * b3;
        adjoint(0, 2) = +M(3, 1) * a5 - M(3, 2) * a4 + M(3, 3) * a3;
        adjoint(0, 3) = -M(2, 1) * a5 + M(2, 2) * a4 - M(2, 3) * a3;
        adjoint(1, 0) = -M(1, 0) * b5 + M(1, 2) * b2 - M(1, 3) * b1;
        adjoint(1, 1) = +M(0, 0) * b5 - M(0, 2) * b2 + M(0, 3) * b1;
        adjoint(1, 2) = -M(3, 0) * a5 + M(3, 2) * a2 - M(3, 3) * a1;
        adjoint(1, 3) = +M(2, 0) * a5 - M(2, 2) * a2 + M(2, 3) * a1;
        adjoint(2, 0) = +M(1, 0) * b4 - M(1, 1) * b2 + M(1, 3) * b0;
        adjoint(2, 1) = -M(0, 0) * b4 + M(0, 1) * b2 - M(0, 3) * b0;
        adjoint(2, 2) = +M(3, 0) * a4 - M(3, 1) * a2 + M(3, 3) * a0;
        adjoint(2, 3) = -M(2, 0) * a4 + M(2, 1) * a2 - M(2, 3) * a0;
        adjoint(3, 0) = -M(1, 0) * b3 + M(1, 1) * b1 - M(1, 2) * b0;
        adjoint(3, 1) = +M(0, 0) * b3 - M(0, 1) * b1 + M(0, 2) * b0;
        adjoint(3, 2) = -M(3, 0) * a3 + M(3, 1) * a1 - M(3, 2) * a0;
        adjoint(3, 3) = +M(2, 0) * a3 - M(2, 1) * a1 + M(2, 2) * a0;
        return adjoint;
    }

    template <typename T>
    T GetDeterminant(Matrix4x4<T> const& M)
    {
        T a0 = M(0, 0) * M(1, 1) - M(0, 1) * M(1, 0);
        T a1 = M(0, 0) * M(1, 2) - M(0, 2) * M(1, 0);
        T a2 = M(0, 0) * M(1, 3) - M(0, 3) * M(1, 0);
        T a3 = M(0, 1) * M(1, 2) - M(0, 2) * M(1, 1);
        T a4 = M(0, 1) * M(1, 3) - M(0, 3) * M(1, 1);
        T a5 = M(0, 2) * M(1, 3) - M(0, 3) * M(1, 2);
        T b0 = M(2, 0) * M(3, 1) - M(2, 1) * M(3, 0);
        T b1 = M(2, 0) * M(3, 2) - M(2, 2) * M(3, 0);
        T b2 = M(2, 0) * M(3, 3) - M(2, 3) * M(3, 0);
        T b3 = M(2, 1) * M(3, 2) - M(2, 2) * M(3, 1);
        T b4 = M(2, 1) * M(3, 3) - M(2, 3) * M(3, 1);
        T b5 = M(2, 2) * M(3, 3) - M(2, 3) * M(3, 2);
        T determinant = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
        return determinant;
    }

    template <typename T>
    T GetTrace(Matrix4x4<T> const& M)
    {
        T trace = M(0, 0) + M(1, 1) + M(2, 2) + M(3, 3);
        return trace;
    }

    // Special matrices.

    // The projection plane is Dot(N,X-P) = 0 where N = (n0,n1,n2,0) is a
    // unit-length normal vector and P = (p0,p1,p2,1) is a point on the plane.
    // The projection is oblique to the plane, in the direction of the vector
    // D = (d0,d1,d2,0). Necessarily Dot(N,D) is not 0 for this projection to
    // make sense. Given a point Q = (q0,q1,q2,1), compute the intersection of
    // the line Q+t*D with the plane to obtain t = -Dot(N,Q-P)/Dot(N,D); then
    //
    //   projection(Q) = P + [I - D*N^T/Dot(N,D)]*(Q-P)
    //
    // A 4-by-4 homogeneous transformation representing the projection is
    //
    //       +-                               -+
    //   M = | D*N^T - Dot(N,D)*I   -Dot(N,P)D |
    //       |          0^T          -Dot(N,D) |
    //       +-                               -+
    //
    // where M applies to [Q^T 1]^T as M*[Q^T 1]^T. The matrix is chosen so
    // that M(3, 3) > 0 whenever Dot(N,D) < 0; the projection is onto the
    // "positive side" of the plane.
    template <typename T>
    Matrix4x4<T> MakeObliqueProjection(Vector4<T> const& origin,
        Vector4<T> const& normal, Vector4<T> const& direction)
    {
        Matrix4x4<T> M{};

        T dotND = Dot(normal, direction);
        T dotNO = Dot(origin, normal);

        M(0, 0) = direction[0] * normal[0] - dotND;
        M(0, 1) = direction[0] * normal[1];
        M(0, 2) = direction[0] * normal[2];
        M(0, 3) = -dotNO * direction[0];
        M(1, 0) = direction[1] * normal[0];
        M(1, 1) = direction[1] * normal[1] - dotND;
        M(1, 2) = direction[1] * normal[2];
        M(1, 3) = -dotNO * direction[1];
        M(2, 0) = direction[2] * normal[0];
        M(2, 1) = direction[2] * normal[1];
        M(2, 2) = direction[2] * normal[2] - dotND;
        M(2, 3) = -dotNO * direction[2];
        M(3, 0) = C_<T>(0);
        M(3, 1) = C_<T>(0);
        M(3, 2) = C_<T>(0);
        M(3, 3) = -dotND;

        return M;
    }

    // The perspective projection of a point onto a plane is
    //
    //     +-                                                 -+
    // M = | Dot(N,E-P)*I - E*N^T    -(Dot(N,E-P)*I - E*N^T)*E |
    //     |        -N^t                      Dot(N,E)         |
    //     +-                                                 -+
    //
    // where E = (e0,e1,e2,1) is the eyepoint, P = (p0,p1,p2,1) is a point
    // on the plane, and N = (n0,n1,n2,0) is a unit-length plane normal.
    template <typename T>
    Matrix4x4<T> MakePerspectiveProjection(Vector4<T> const& origin,
        Vector4<T> const& normal, Vector4<T> const& eye)
    {
        Matrix4x4<T> M{};

        T dotND = Dot(normal, eye - origin);

        M(0, 0) = dotND - eye[0] * normal[0];
        M(0, 1) = -eye[0] * normal[1];
        M(0, 2) = -eye[0] * normal[2];
        M(0, 3) = -(M(0, 0) * eye[0] + M(0, 1) * eye[1] + M(0, 2) * eye[2]);
        M(1, 0) = -eye[1] * normal[0];
        M(1, 1) = dotND - eye[1] * normal[1];
        M(1, 2) = -eye[1] * normal[2];
        M(1, 3) = -(M(1, 0) * eye[0] + M(1, 1) * eye[1] + M(1, 2) * eye[2]);
        M(2, 0) = -eye[2] * normal[0];
        M(2, 1) = -eye[2] * normal[1];
        M(2, 2) = dotND - eye[2] * normal[2];
        M(2, 3) = -(M(2, 0) * eye[0] + M(2, 1) * eye[1] + M(2, 2) * eye[2]);
        M(3, 0) = -normal[0];
        M(3, 1) = -normal[1];
        M(3, 2) = -normal[2];
        M(3, 3) = Dot(eye, normal);

        return M;
    }

    // The reflection of a point through a plane is
    //     +-                         -+
    // M = | I-2*N*N^T    2*Dot(N,P)*N |
    //     |     0^T            1      |
    //     +-                         -+
    //
    // where P = (p0,p1,p2,1) is a point on the plane and N = (n0,n1,n2,0) is
    // a unit-length plane normal.
    template <typename T>
    Matrix4x4<T> MakeReflection(Vector4<T> const& origin,
        Vector4<T> const& normal)
    {
        Matrix4x4<T> M{};

        T twoDotNO = C_<T>(2) * Dot(origin, normal);

        M(0, 0) = C_<T>(1) - C_<T>(2) * normal[0] * normal[0];
        M(0, 1) = -C_<T>(2) * normal[0] * normal[1];
        M(0, 2) = -C_<T>(2) * normal[0] * normal[2];
        M(0, 3) = twoDotNO * normal[0];
        M(1, 0) = M(0, 1);
        M(1, 1) = C_<T>(1) - C_<T>(2) * normal[1] * normal[1];
        M(1, 2) = -C_<T>(2) * normal[1] * normal[2];
        M(1, 3) = twoDotNO * normal[1];
        M(2, 0) = M(0, 2);
        M(2, 1) = M(1, 2);
        M(2, 2) = C_<T>(1) - C_<T>(2) * normal[2] * normal[2];
        M(2, 3) = twoDotNO * normal[2];
        M(3, 0) = C_<T>(0);
        M(3, 1) = C_<T>(0);
        M(3, 2) = C_<T>(0);
        M(3, 3) = C_<T>(1);

        return M;
    }
}
