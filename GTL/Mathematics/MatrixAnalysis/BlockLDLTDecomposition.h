// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <GTL/Mathematics/MatrixAnalysis/GaussianElimination.h>
#include <array>
#include <vector>

// Factor a positive symmetric matrix A = L * D * L^T, where L is a lower
// triangular matrix with diagonal entries all 1 (L is lower unit triangular)
// and where D is a diagonal matrix with diagonal entries all positive.

namespace gtl
{
    template <typename T, size_t...> class BlockLDLTDecomposition;

    // Implementation for sizes known at compile time.
    template <typename T, size_t BlockSize, size_t NumBlocks>
    class BlockLDLTDecomposition<T, BlockSize, NumBlocks>
    {
    public:
        // Let B represent the block size and N represent the number of
        // blocks. The matrix A is (N*B)-by-(N*B) but partitioned into an
        // N-by-N matrix of blocks, each block of size B-by-B. The value
        // N*B is NumDimensions.
        enum
        {
            NumDimensions = NumBlocks * BlockSize
        };

        using BlockVector = std::array<Vector<T, BlockSize>, NumBlocks>;
        using BlockMatrix = std::array<std::array<Matrix<T, BlockSize, BlockSize>, NumBlocks>, NumBlocks>;

        BlockLDLTDecomposition()
        {
            static_assert(
                BlockSize > 0 && NumBlocks > 0,
                "Invalid size.");
        }

        // Treating the matrix as a 2D table of scalars with NumDimensions
        // rows and NumDimensions columns, look up the correct block that
        // stores the requested element and return a reference.
        void Get(BlockMatrix const& M, size_t row, size_t col, T& value)
        {
            size_t b0 = col / BlockSize;
            size_t b1 = row / BlockSize;
            size_t i0 = col - BlockSize * b0;
            size_t i1 = row - BlockSize * b1;
            auto const& MBlock = M[b1][b0];
            value = MBlock(i1, i0);
        }

        void Set(BlockMatrix& M, size_t row, size_t col, T const& value)
        {
            size_t b0 = col / BlockSize;
            size_t b1 = row / BlockSize;
            size_t i0 = col - BlockSize * b0;
            size_t i1 = row - BlockSize * b1;
            auto& MBlock = M[b1][b0];
            MBlock(i1, i0) = value;
        }

        // Convert from a matrix to a block matrix.
        void Convert(Matrix<T, NumDimensions, NumDimensions> const& M, BlockMatrix& MBlock) const
        {
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (size_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize)
                {
                    auto& current = MBlock[r][c];
                    for (size_t j = 0; j < BlockSize; ++j)
                    {
                        for (size_t i = 0; i < BlockSize; ++i)
                        {
                            current(j, i) = M(rb + j, cb + i);
                        }
                    }
                }
            }
        }

        // Convert from a vector to a block vector.
        void Convert(Vector<T, NumDimensions> const& V, BlockVector& VBlock) const
        {
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto& current = VBlock[r];
                for (size_t j = 0; j < BlockSize; ++j)
                {
                    current[j] = V[rb + j];
                }
            }
        }

        // Convert from a block matrix to a matrix.
        void Convert(BlockMatrix const& MBlock, Matrix<T, NumDimensions, NumDimensions>& M) const
        {
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (size_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize)
                {
                    auto const& current = MBlock[r][c];
                    for (size_t j = 0; j < BlockSize; ++j)
                    {
                        for (size_t i = 0; i < BlockSize; ++i)
                        {
                            M(rb + j, cb + i) = current(j, i);
                        }
                    }
                }
            }
        }

        // Convert from a block vector to a vector.
        void Convert(BlockVector const& VBlock, Vector<T, NumDimensions>& V) const
        {
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto const& current = VBlock[r];
                for (size_t j = 0; j < BlockSize; ++j)
                {
                    V[rb + j] = current[j];
                }
            }
        }

        // The block matrix A must be positive definite. The implementation
        // uses only the lower-triangular blocks of A. On output, the block
        // matrix L is lower unit triangular (diagonal blocks are BxB identity
        // matrices) and the block matrix D is diagonal (diagonal blocks are
        // BxB diagonal matrices).
        bool Factor(BlockMatrix const& A, BlockMatrix& L, BlockMatrix& D)
        {
            for (size_t row = 0; row < NumBlocks; ++row)
            {
                for (size_t col = 0; col < NumBlocks; ++col)
                {
                    MakeZero(L[row][col]);
                    MakeZero(D[row][col]);
                }
            }

            for (size_t j = 0; j < NumBlocks; ++j)
            {
                Matrix<T, BlockSize, BlockSize> Djj = A[j][j];
                for (size_t k = 0; k < j; ++k)
                {
                    auto const& Ljk = L[j][k];
                    auto const& Dkk = D[k][k];
                    Djj -= MultiplyABT(Ljk * Dkk, Ljk);
                }
                D[j][j] = Djj;
                T determinant = C_<T>(0);
                Matrix<T, BlockSize, BlockSize> invDjj = Inverse(Djj, &determinant);
                if (determinant == C_<T>(0))
                {
                    return false;
                }

                MakeIdentity(L[j][j]);
                for (size_t i = j + 1; i < NumBlocks; ++i)
                {
                    Matrix<T, BlockSize, BlockSize> Lij = A[i][j];
                    for (size_t k = 0; k < j; ++k)
                    {
                        auto const& Lik = L[i][k];
                        auto const& Ljk = L[j][k];
                        auto const& Dkk = D[k][k];
                        Lij -= MultiplyABT(Lik * Dkk, Ljk);
                    }
                    Lij = Lij * invDjj;
                    L[i][j] = Lij;
                }
            }
            return true;
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring before the call.
        void Solve(BlockMatrix const& L, BlockMatrix const& D,
            BlockVector const& B, BlockVector& X)
        {
            // Solve L * Z = L * (D * L^T * X) = B for Z.
            for (size_t r = 0; r < NumBlocks; ++r)
            {
                X[r] = B[r];
                for (size_t c = 0; c < r; ++c)
                {
                    X[r] -= L[r][c] * X[c];
                }
            }

            // Solve D * Y = D * (L^T * X) = Z for Y.
            for (size_t r = 0; r < NumBlocks; ++r)
            {
                X[r] = Inverse(D[r][r]) * X[r];
            }

            // Solve L^T * Y = Z for X.
            for (size_t k = 0, r = NumBlocks - 1; k < NumBlocks; ++k, --r)
            {
                for (size_t c = r + 1; c < NumBlocks; ++c)
                {
                    X[r] -= X[c] * L[c][r];
                }
            }
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring during the call.
        void Solve(BlockMatrix const& A, BlockVector const& B, BlockVector& X)
        {
            BlockMatrix L{}, D{};
            Factor(A, L, D);
            Solve(L, D, B, X);
        }
    };

    // Implementation for sizes known only at run time.
    template <typename T>
    class BlockLDLTDecomposition<T>
    {
    public:
        // Let B represent the block size and N represent the number of
        // blocks. The matrix A is (N*B)-by-(N*B) but partitioned into an
        // N-by-N matrix of blocks, each block of size B-by-B and stored in
        // row-major order. The value N*B is NumDimensions.
        size_t const BlockSize;
        size_t const NumBlocks;
        size_t const NumDimensions;

        // The number of elements in a BlockVector object must be NumBlocks
        // and each Vector element has BlockSize components.
        using BlockVector = std::vector<Vector<T>>;

        // The BlockMatrix is an array of NumBlocks-by-NumBlocks matrices.
        // Each block matrix is stored in row-major order. The BlockMatrix
        // elements themselves are stored in row-major order. The block
        // matrix element M = BlockMatrix[col + NumBlocks * row] is of size
        // BlockSize-by-BlockSize (in row-major order) and is in the (row,col)
        // location of the full matrix of blocks.
        using BlockMatrix = std::vector<Matrix<T>>;

        BlockLDLTDecomposition(size_t blockSize, size_t numBlocks)
            :
            BlockSize(blockSize),
            NumBlocks(numBlocks),
            NumDimensions(blockSize * numBlocks)
        {
            GTL_ARGUMENT_ASSERT(
                blockSize > 0 && numBlocks > 0,
                "Invalid size.");
        }

        // Treating the matrix as a 2D table of scalars with NumDimensions
        // rows and NumDimensions columns, look up the correct block that
        // stores the requested element and return a reference. NOTE: You
        // are responsible for ensuring that M has NumBlocks-by-NumBlocks
        // elements, each M[] having BlockSize-by-BlockSize elements.
        void Get(BlockMatrix const& M, size_t row, size_t col, T& value, bool verifySize = true)
        {
            if (verifySize)
            {
                GTL_ARGUMENT_ASSERT(
                    M.size() == NumBlocks * NumBlocks,
                    "Invalid size.");
            }

            size_t b0 = col / BlockSize;
            size_t b1 = row / BlockSize;
            size_t i0 = col - BlockSize * b0;
            size_t i1 = row - BlockSize * b1;
            auto const& MBlock = M[GetIndex(b1, b0)];

            if (verifySize)
            {
                GTL_ARGUMENT_ASSERT(
                    MBlock.GetNumRows() == BlockSize &&
                    MBlock.GetNumCols() == BlockSize,
                    "Invalid size.");
            }

            value = MBlock(i1, i0);
        }

        void Set(BlockMatrix& M, size_t row, size_t col, T const& value, bool verifySize = true)
        {
            if (verifySize)
            {
                GTL_ARGUMENT_ASSERT(
                    M.size() == NumBlocks * NumBlocks,
                    "Invalid size.");
            }

            size_t b0 = col / BlockSize;
            size_t b1 = row / BlockSize;
            size_t i0 = col - BlockSize * b0;
            size_t i1 = row - BlockSize * b1;
            auto& MBlock = M[GetIndex(b1, b0)];

            if (verifySize)
            {
                GTL_ARGUMENT_ASSERT(
                    MBlock.GetNumRows() == BlockSize &&
                    MBlock.GetNumCols() == BlockSize,
                    "Invalid size.");
            }

            MBlock(i1, i0) = value;
        }

        // Convert from a matrix to a block matrix.
        void Convert(Matrix<T> const& M, BlockMatrix& MBlock, bool verifySize = true) const
        {
            if (verifySize)
            {
                GTL_ARGUMENT_ASSERT(
                    M.GetNumRows() == NumDimensions &&
                    M.GetNumCols() == NumDimensions,
                    "Invalid size.");
            }

            MBlock.resize(NumBlocks * NumBlocks);
            for (size_t r = 0, rb = 0, index = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (size_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize, ++index)
                {
                    auto& current = MBlock[index];
                    current.resize(BlockSize, BlockSize);
                    for (size_t j = 0; j < BlockSize; ++j)
                    {
                        for (size_t i = 0; i < BlockSize; ++i)
                        {
                            current(j, i) = M(rb + j, cb + i);
                        }
                    }
                }
            }
        }

        // Convert from a vector to a block vector.
        void Convert(Vector<T> const& V, BlockVector& VBlock, bool verifySize = true) const
        {
            if (verifySize)
            {
                GTL_ARGUMENT_ASSERT(
                    V.size() == NumDimensions,
                    "Invalid size.");
            }

            VBlock.resize(NumBlocks);
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto& current = VBlock[r];
                current.resize(BlockSize);
                for (size_t j = 0; j < BlockSize; ++j)
                {
                    current[j] = V[rb + j];
                }
            }
        }

        // Convert from a block matrix to a matrix.
        void Convert(BlockMatrix const& MBlock, Matrix<T>& M, bool verifySize = true) const
        {
            if (verifySize)
            {
                GTL_ARGUMENT_ASSERT(
                    MBlock.size() == NumBlocks * NumBlocks,
                    "Invalid size.");

                for (auto const& current : MBlock)
                {
                    GTL_ARGUMENT_ASSERT(
                        current.GetNumRows() == NumBlocks &&
                        current.GetNumCols() == NumBlocks,
                        "Invalid size.");
                }
            }

            M.resize(NumDimensions, NumDimensions);
            for (size_t r = 0, rb = 0, index = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (size_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize, ++index)
                {
                    auto const& current = MBlock[index];
                    for (size_t j = 0; j < BlockSize; ++j)
                    {
                        for (size_t i = 0; i < BlockSize; ++i)
                        {
                            M(rb + j, cb + i) = current(j, i);
                        }
                    }
                }
            }
        }

        // Convert from a block vector to a vector.
        void Convert(BlockVector const& VBlock, Vector<T>& V, bool verifySize = true) const
        {
            if (verifySize)
            {
                GTL_ARGUMENT_ASSERT(
                    VBlock.size() == NumBlocks,
                    "Invalid size.");

                for (auto const& current : VBlock)
                {
                    GTL_ARGUMENT_ASSERT(
                        current.size() == NumBlocks,
                        "Invalid size.");
                }
            }

            V.resize(NumDimensions);
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto const& current = VBlock[r];
                for (size_t j = 0; j < BlockSize; ++j)
                {
                    V[rb + j] = current[j];
                }
            }
        }

        // The block matrix A must be positive definite. The implementation
        // uses only the lower-triangular blocks of A. On output, the block
        // matrix L is lower unit triangular (diagonal blocks are BxB identity
        // matrices) and the block matrix D is diagonal (diagonal blocks are
        // BxB diagonal matrices).
        bool Factor(BlockMatrix const& A, BlockMatrix& L, BlockMatrix& D,
            bool verifySize = true)
        {
            if (verifySize)
            {
                GTL_ARGUMENT_ASSERT(
                    A.size() == NumBlocks * NumBlocks,
                    "Invalid size.");

                for (size_t i = 0; i < A.size(); ++i)
                {
                    GTL_ARGUMENT_ASSERT(
                        A[i].GetNumRows() == BlockSize &&
                        A[i].GetNumCols() == BlockSize,
                        "Invalid size.");
                }
            }

            L.resize(A.size());
            D.resize(A.size());
            for (size_t i = 0; i < L.size(); ++i)
            {
                L[i].resize(BlockSize, BlockSize);
                MakeZero(L[i]);
                D[i].resize(BlockSize, BlockSize);
                MakeZero(D[i]);
            }

            for (size_t j = 0; j < NumBlocks; ++j)
            {
                Matrix<T> Djj = A[GetIndex(j, j)];
                for (size_t k = 0; k < j; ++k)
                {
                    auto const& Ljk = L[GetIndex(j, k)];
                    auto const& Dkk = D[GetIndex(k, k)];
                    Djj -= MultiplyABT(Ljk * Dkk, Ljk);
                }
                D[GetIndex(j, j)] = Djj;
                T determinant = C_<T>(0);
                Matrix<T> invDjj = Inverse(Djj, &determinant);
                if (determinant == C_<T>(0))
                {
                    return false;
                }

                MakeIdentity(L[GetIndex(j, j)]);
                for (size_t i = j + 1; i < NumBlocks; ++i)
                {
                    Matrix<T> Lij = A[GetIndex(i, j)];
                    for (size_t k = 0; k < j; ++k)
                    {
                        auto const& Lik = L[GetIndex(i, k)];
                        auto const& Ljk = L[GetIndex(j, k)];
                        auto const& Dkk = D[GetIndex(k, k)];
                        Lij -= MultiplyABT(Lik * Dkk, Ljk);
                    }
                    Lij = Lij * invDjj;
                    L[GetIndex(i, j)] = Lij;
                }
            }
            return true;
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring before the call.
        void Solve(BlockMatrix const& L, BlockMatrix const& D,
            BlockVector const& B, BlockVector& X, bool verifySize = true)
        {
            if (verifySize)
            {
                size_t const LDsize = NumBlocks * NumBlocks;
                GTL_ARGUMENT_ASSERT(
                    L.size() == LDsize &&
                    D.size() == LDsize &&
                    B.size() == NumBlocks,
                    "Invalid size.");

                for (size_t i = 0; i < L.size(); ++i)
                {
                    GTL_ARGUMENT_ASSERT(
                        L[i].GetNumRows() == BlockSize &&
                        L[i].GetNumCols() == BlockSize &&
                        D[i].GetNumRows() == BlockSize &&
                        D[i].GetNumCols() == BlockSize,
                        "Invalid size.");
                }

                for (size_t i = 0; i < B.size(); ++i)
                {
                    GTL_ARGUMENT_ASSERT(
                        B[i].size() == BlockSize,
                        "Invalid size.");
                }
            }

            // Solve L * Z = L * (D * L^T * X) = B for Z.
            X.resize(NumBlocks);
            for (size_t r = 0; r < NumBlocks; ++r)
            {
                X[r] = B[r];
                for (size_t c = 0; c < r; ++c)
                {
                    X[r] -= L[GetIndex(r, c)] * X[c];
                }
            }

            // Solve D * Y = D * (L^T * X) = Z for Y.
            for (size_t r = 0; r < NumBlocks; ++r)
            {
                X[r] = Inverse(D[GetIndex(r, r)]) * X[r];
            }

            // Solve L^T * Y = Z for X.
            for (size_t k = 0, r = NumBlocks - 1; k < NumBlocks; ++k, --r)
            {
                for (size_t c = r + 1; c < NumBlocks; ++c)
                {
                    X[r] -= X[c] * L[GetIndex(c, r)];
                }
            }
        }

        // Solve A*X = B for positive definite A = L * D * L^T with
        // factoring during the call.
        void Solve(BlockMatrix const& A, BlockVector const& B, BlockVector& X,
            bool verifySize = true)
        {
            if (verifySize)
            {
                GTL_ARGUMENT_ASSERT(
                    A.size() == NumBlocks * NumBlocks &&
                    B.size() == NumBlocks,
                    "Invalid size.");

                for (size_t i = 0; i < A.size(); ++i)
                {
                    GTL_ARGUMENT_ASSERT(
                        A[i].GetNumRows() == BlockSize &&
                        A[i].GetNumCols() == BlockSize,
                        "Invalid size.");
                }

                for (size_t i = 0; i < B.size(); ++i)
                {
                    GTL_ARGUMENT_ASSERT(
                        B[i].size() == BlockSize,
                        "Invalid size.");
                }
            }

            BlockMatrix L{}, D{};
            Factor(A, L, D, false);
            Solve(L, D, B, X, false);
        }

    private:
        // Compute the 1-dimensional index of the block matrix in a
        // 2-dimensional BlockMatrix object.
        inline size_t GetIndex(size_t row, size_t col) const
        {
            return col + row * NumBlocks;
        }
    };
}
