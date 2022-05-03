// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <GTL/Mathematics/MatrixAnalysis/CholeskyDecomposition.h>
#include <array>
#include <vector>

namespace gtl
{
    template <typename T, size_t...> class BlockCholeskyDecomposition;

    // Implementation for sizes known at compile time. Let A be M-by-M
    // and C be M-by-1. Let A be partitioned into N-by-N blocks, each block
    // of size B-by-B. Let C and X be partitioned into N-by-1 blocks, each
    // block of size B-by-1. To solve A*X = C as Ablock*Xblock = Cblock,
    //   using BCD = BlockCholeskyDecomposition<T, B, N>;
    //   BCD::BlockMatrix Ablock = user_specified_symmetric_matrix;
    //   BCD::BlockVector Cblock = user_specified_vector;
    //   BCD::BlockVector Xblock = solution_to_be_determined;
    //   BCD decomposer;
    //   bool success = decomposer.Factor(Ablock);
    //   if (success)
    //   {
    //       Xblock = Cblock;
    //       decomposer.SolveLower(Ablock, Xblock);
    //       decomposer.SolveUpper(Ablock, Xblock);
    //   }
    //
    // or
    //
    //   using BCD = BlockCholeskyDecomposition<T, B, N>;
    //   BCD::BlockMatrix Ablock = user_specified_symmetric_matrix;
    //   BCD::BlockMatrix Lblock = lower_triangular_to_be_determined;
    //   BCD::BlockVector Cblock = user_specified_vector;
    //   BCD::BlockVector Xblock = solution_to_be_determined;
    //   BCD decomposer;
    //   bool success = decomposer.Factor(Ablock, Lblock);
    //   if (success)
    //   {
    //       Xblock = Cblock;
    //       decomposer.SolveLower(Lblock, Xblock);
    //       decomposer.SolveUpper(Lblock, Xblock);
    //   }
    //
    // You can convert a matrix to a block matrix and convert a vector to a
    // block vector by
    //   Matrix<T, N*B, N*B> A = user_specified_symmetric_matrix;
    //   Vector<T, N*B> C = user_specified_vector;
    //   BCD::BlockMatrix Ablock;
    //   BCD::BlockVector Cblock;
    //   decomposer.Convert(A, Ablock);
    //   decomposer.Convert(C, Cblock);
    //   BCD::BlockVector Xblock = solution_to_be_determined;
    //   solve_system_using_decomposer;  // solve Ablock * Xblock = Cblock
    //   Vector<T, N*B> X;
    //   decomposer.Convert(Xblock, X);
    template <typename T, size_t BlockSize, size_t NumBlocks>
    class BlockCholeskyDecomposition<T, BlockSize, NumBlocks>
    {
    public:
        // Let B represent the block size and N represent the number of
        // blocks. The matrix A is (N*B)-by-(N*B) but partitioned into an
        // N-by-N matrix of blocks, each block of size B-by-B and stored in
        // row-major order. The value N*B is NumDimensions.
        enum
        {
            NumDimensions = NumBlocks * BlockSize
        };

        using BlockVector = std::array<Vector<T, BlockSize>, NumBlocks>;
        using BlockMatrix = std::array<std::array<Matrix<T, BlockSize, BlockSize>, NumBlocks>, NumBlocks>;

        // Ensure that BlockSize > 0 and NumBlocks > 0 at compile time.
        BlockCholeskyDecomposition()
        {
            static_assert(
                BlockSize > 0 && NumBlocks > 0,
                "Invalid size in BlockCholeskyDecomposition constructor.");
        }

        // Treating the matrix as a 2D table of scalars with NumDimensions
        // rows and NumDimensions columns, look up the correct block that
        // stores the requested element and return a reference to the element.
        T const& Get(BlockMatrix const& M, size_t row, size_t col) const
        {
            size_t b0 = col / BlockSize;
            size_t b1 = row / BlockSize;
            size_t i0 = col - BlockSize * b0;
            size_t i1 = row - BlockSize * b1;
            auto const& block = M[b1][b0];
            return block(i1, i0);
        }

        // Treating the matrix as a 2D table of scalars with NumDimensions
        // rows and NumDimensions columns, look up the correct block that
        // stores the requested element and assign value to the element.
        void Set(BlockMatrix& M, size_t row, size_t col, T const& value) const
        {
            size_t b0 = col / BlockSize;
            size_t b1 = row / BlockSize;
            size_t i0 = col - BlockSize * b0;
            size_t i1 = row - BlockSize * b1;
            auto& block = M[b1][b0];
            block(i1, i0) = value;
        }

        // Convert from a matrix to a block matrix.
        void Convert(Matrix<T, NumDimensions, NumDimensions> const& M, BlockMatrix& Mblock) const
        {
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (size_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize)
                {
                    auto& current = Mblock[r][c];
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
        void Convert(Vector<T, NumDimensions> const& V, BlockVector& Vblock) const
        {
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto& current = Vblock[r];
                for (size_t j = 0; j < BlockSize; ++j)
                {
                    current[j] = V[rb + j];
                }
            }
        }

        // Convert from a block matrix to a matrix.
        void Convert(BlockMatrix const& Mblock, Matrix<T, NumDimensions, NumDimensions>& M) const
        {
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (size_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize)
                {
                    auto const& current = Mblock[r][c];
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
        void Convert(BlockVector const& Vblock, Vector<T, NumDimensions>& V) const
        {
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto const& current = Vblock[r];
                for (size_t j = 0; j < BlockSize; ++j)
                {
                    V[rb + j] = current[j];
                }
            }
        }

        // The input matrix A must be symmetric. Only the lower-triangular
        // portion is modified, including the diagonal. On output, the
        // lower-triangular portion is L where A = L * L^T. You can pass A
        // to SolveLower and SolveUpper because those functions access the
        // original entries of A in the lower-triangular part of the matrix.
        bool Factor(BlockMatrix& A) const
        {
            CholeskyDecomposition<T, BlockSize> decomposer{};
            for (size_t c = 0; c < NumBlocks; ++c)
            {
                if (!decomposer.Factor(A[c][c]))
                {
                    return false;
                }

                for (size_t r = c + 1; r < NumBlocks; ++r)
                {
                    LowerTriangularSolver(r, c, A);
                }

                for (size_t k = c + 1; k < NumBlocks; ++k)
                {
                    for (size_t r = k; r < NumBlocks; ++r)
                    {
                        SubtractiveUpdate(r, k, c, A);
                    }
                }
            }

            return true;
        }

        // The input matrix A must be symmetric. The output matrix is L which
        // is lower triangular and A = L * L^T. You must pass L to SolveLower
        // and SolveUpper.
        bool Factor(BlockMatrix const& A, BlockMatrix& L) const
        {
            L = A;
            if (Factor(L))
            {
                Matrix<T, BlockSize, BlockSize> zeroBlock{};
                for (size_t r = 0; r < NumBlocks; ++r)
                {
                    // Set the upper-triangular parts of the diagonal blocks
                    // to zero.
                    for (size_t j = 0; j < BlockSize; ++j)
                    {
                        for (size_t i = j + 1; i < BlockSize; ++i)
                        {
                            L[r][r](j, i) = C_<T>(0);
                        }
                    }

                    // Set the upper-triangular blocks to zero.
                    for (size_t c = r + 1; c < NumBlocks; ++c)
                    {
                        L[r][c] = zeroBlock;
                    }
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        // Solve L*Y = B, where L is an invertible lower-triangular block
        // matrix whose diagonal blocks are lower-triangular matrices.
        // The input B is a block vector of commensurate size. The input
        // value of Y is B. On output, Y is the solution.
        void SolveLower(BlockMatrix const& L, BlockVector& Y) const
        {
            for (size_t r = 0; r < NumBlocks; ++r)
            {
                auto& Yr = Y[r];
                for (size_t c = 0; c < r; ++c)
                {
                    auto const& Lrc = L[r][c];
                    auto const& Yc = Y[c];
                    for (size_t i = 0; i < BlockSize; ++i)
                    {
                        for (size_t j = 0; j < BlockSize; ++j)
                        {
                            Yr[i] -= Lrc(i, j) * Yc[j];
                        }
                    }
                }
                CholeskyDecomposition<T, BlockSize> decomposer;
                decomposer.SolveLower(L[r][r], Yr);
            }
        }

        // Solve L^T*X = Y, where L is an invertible lower-triangular block
        // matrix (L^T is an upper-triangular block matrix) whose diagonal
        // blocks are lower-triangular matrices. The input value of X is Y.
        // On output, X is the solution.
        void SolveUpper(BlockMatrix const& L, BlockVector& X) const
        {
            for (size_t k = 0, r = NumBlocks - 1; k < NumBlocks; ++k, --r)
            {
                auto& Xr = X[r];
                for (size_t c = r + 1; c < NumBlocks; ++c)
                {
                    auto const& Lcr = L[c][r];
                    auto const& Xc = X[c];
                    for (size_t i = 0; i < BlockSize; ++i)
                    {
                        for (size_t j = 0; j < BlockSize; ++j)
                        {
                            Xr[i] -= Lcr(j, i) * Xc[j];
                        }
                    }
                }
                CholeskyDecomposition<T, BlockSize> decomposer;
                decomposer.SolveUpper(L[r][r], Xr);
            }
        }

    private:
        // Solve G(c,c)*G(r,c)^T = A(r,c)^T for G(r,c). The matrices
        // G(c,c) and A(r,c) are known quantities, and G(c,c) occupies
        // the lower triangular portion of A(c,c). The solver stores
        // its results in-place, so A(r,c) stores the G(r,c) result.
        void LowerTriangularSolver(size_t r, size_t c, BlockMatrix& A) const
        {
            auto const& Acc = A[c][c];
            auto& Arc = A[r][c];
            for (size_t j = 0; j < BlockSize; ++j)
            {
                for (size_t i = 0; i < j; ++i)
                {
                    T Lji = Acc(j, i);
                    for (size_t k = 0; k < BlockSize; ++k)
                    {
                        Arc(k, j) -= Lji * Arc(k, i);
                    }
                }

                T Ljj = Acc(j, j);
                for (size_t k = 0; k < BlockSize; ++k)
                {
                    Arc(k, j) /= Ljj;
                }
            }
        }

        void SubtractiveUpdate(size_t r, size_t k, size_t c, BlockMatrix& A) const
        {
            auto const& Arc = A[r][c];
            auto const& Akc = A[k][c];
            auto& Ark = A[r][k];
            for (size_t j = 0; j < BlockSize; ++j)
            {
                for (size_t i = 0; i < BlockSize; ++i)
                {
                    for (size_t m = 0; m < BlockSize; ++m)
                    {
                        Ark(j, i) -= Arc(j, m) * Akc(i, m);
                    }
                }
            }
        }
    };

    // Implementation for sizes known only at time. Let A be M-by-M
    // and C be M-by-1. Let A be partitioned into N-by-N blocks, each block
    // of size B-by-B. Let C and X be partitioned into N-by-1 blocks, each
    // block of size B-by-1. To solve A*X = C as Ablock*Xblock = Cblock,
    //   using BCD = BlockCholeskyDecomposition<T>;
    //   BCD::BlockMatrix Ablock = user_specified_symmetric_matrix;
    //   BCD::BlockVector Cblock = user_specified_vector;
    //   BCD::BlockVector Xblock = solution_to_be_determined;
    //   BCD decomposer(B, N);
    //   bool success = decomposer.Factor(Ablock);
    //   if (success)
    //   {
    //       Xblock = Cblock;
    //       decomposer.SolveLower(Ablock, Xblock);
    //       decomposer.SolveUpper(Ablock, Xblock);
    //   }
    //
    // or
    //
    //   using BCD = BlockCholeskyDecomposition<T>;
    //   BCD::BlockMatrix A = user_specified_symmetric_matrix;
    //   BCD::BlockMatrix L = lower_triangular_to_be_determined;
    //   BCD::BlockVector Cblock = user_specified_vector;
    //   BCD::BlockVector Xblock = solution_to_be_determined;
    //   BCD decomposer;
    //   bool success = decomposer.Factor(Ablock, Lblock);
    //   if (success)
    //   {
    //       Xblock = Cblock;
    //       decomposer.SolveLower(Lblock, Xblock);
    //       decomposer.SolveUpper(Lblock, Xblock);
    //   }
    //
    // You can convert a matrix to a block matrix and convert a vector to a
    // block vector by
    //   Matrix<T> A(N*B, N*B) = user_specified_symmetric_matrix;
    //   Vector<T> C(N*B) = user_specified_vector;
    //   BCD::BlockMatrix Ablock;
    //   BCD::BlockVector Cblock;
    //   decomposer.Convert(A, Ablock);
    //   decomposer.Convert(C, Cblock);
    //   BCD::BlockVector Xblock = solution_to_be_determined;
    //   solve_system_using_decomposer;  // solve Ablock * Xblock = Cblock
    //   Vector<T> X;
    //   decomposer.Convert(Xblock, X);
    template <typename T>
    class BlockCholeskyDecomposition<T>
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

        // Ensure that BlockSize > 0 and NumDimensions > 0 at run time.
        BlockCholeskyDecomposition(size_t blockSize, size_t numBlocks)
            :
            BlockSize(blockSize),
            NumBlocks(numBlocks),
            NumDimensions(numBlocks * blockSize)
        {
            GTL_ARGUMENT_ASSERT(
                blockSize > 0 && numBlocks > 0,
                "Invalid input.");
        }

        // Treating the matrix as a 2D table of scalars with NumDimensions
        // rows and NumDimensions columns, look up the correct block that
        // stores the requested element and return a reference.
        T const& Get(BlockMatrix const& M, size_t row, size_t col) const
        {
            GTL_ARGUMENT_ASSERT(
                M.size() == NumBlocks * NumBlocks,
                "Incorrect number of elements in block matrix.");

            size_t b0 = col / BlockSize;
            size_t b1 = row / BlockSize;
            size_t i0 = col - BlockSize * b0;
            size_t i1 = row - BlockSize * b1;
            auto const& block = M[GetIndex(b1, b0)];
            GTL_LENGTH_ASSERT(
                block.GetNumRows() == BlockSize && block.GetNumCols() == BlockSize,
                "Invalid dimensions for block matrix.");
            return block(i1, i0);
        }

        void Set(BlockMatrix& M, size_t row, size_t col, T const& value) const
        {
            GTL_ARGUMENT_ASSERT(
                M.size() == NumBlocks * NumBlocks,
                "Incorrect number of elements in block matrix.");

            size_t b0 = col / BlockSize;
            size_t b1 = row / BlockSize;
            size_t i0 = col - BlockSize * b0;
            size_t i1 = row - BlockSize * b1;
            auto& block = M[GetIndex(b1, b0)];
            GTL_LENGTH_ASSERT(
                block.GetNumRows() == BlockSize && block.GetNumCols() == BlockSize,
                "Invalid dimensions for block matrix.");
            block(i1, i0) = value;
        }

        // Convert from a matrix to a block matrix.
        void Convert(Matrix<T> const& M, BlockMatrix& Mblock) const
        {
            GTL_LENGTH_ASSERT(
                M.GetNumRows() == NumDimensions && M.GetNumCols() == NumDimensions,
                "M matrix has invalid dimensions.");

            Mblock.resize(NumBlocks * NumBlocks);
            for (size_t r = 0, rb = 0, index = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (size_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize, ++index)
                {
                    auto& current = Mblock[index];
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
        void Convert(Vector<T> const& V, BlockVector& Vblock) const
        {
            GTL_LENGTH_ASSERT(
                V.size() == NumDimensions,
                "V vector has invalid dimension.");

            Vblock.resize(NumBlocks);
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto& current = Vblock[r];
                current.resize(BlockSize);
                for (size_t j = 0; j < BlockSize; ++j)
                {
                    current[j] = V[rb + j];
                }
            }
        }

        // Convert from a block matrix to a matrix.
        void Convert(BlockMatrix const& Mblock, Matrix<T>& M) const
        {
            GTL_LENGTH_ASSERT(
                Mblock.size() == NumBlocks * NumBlocks,
                "Incorrect number of elements in block matrix.");
            for (auto const& current : Mblock)
            {
                GTL_LENGTH_ASSERT(
                    current.GetNumRows() == NumBlocks && current.GetNumCols() == NumBlocks,
                    "A matrix block has invalid dimensions.");
            }

            M.resize(NumDimensions, NumDimensions);
            for (size_t r = 0, rb = 0, index = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                for (size_t c = 0, cb = 0; c < NumBlocks; ++c, cb += BlockSize, ++index)
                {
                    auto const& current = Mblock[index];
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
        void Convert(BlockVector const& Vblock, Vector<T>& V) const
        {
            GTL_LENGTH_ASSERT(
                Vblock.size() == NumBlocks,
                "Incorrect number of elements in block vector.");
            for (auto const& current : Vblock)
            {
                GTL_LENGTH_ASSERT(
                    current.size() == NumBlocks,
                    "A vector block has invalid dimensions.");
            }

            V.resize(NumDimensions);
            for (size_t r = 0, rb = 0; r < NumBlocks; ++r, rb += BlockSize)
            {
                auto const& current = Vblock[r];
                for (size_t j = 0; j < BlockSize; ++j)
                {
                    V[rb + j] = current[j];
                }
            }
        }

        // The input matrix A must be symmetric. Only the lower-triangular
        // portion is modified, including the diagonal. On output, the
        // lower-triangular portion is L where A = L * L^T. You can pass A
        // to SolveLower and SolveUpper because those functions access the
        // original entries of A in the lower-triangular part of the matrix.
        bool Factor(BlockMatrix& A) const
        {
            CholeskyDecomposition<T> decomposer(BlockSize);
            for (size_t c = 0; c < NumBlocks; ++c)
            {
                if (!decomposer.Factor(A[GetIndex(c, c)]))
                {
                    return false;
                }

                for (size_t r = c + 1; r < NumBlocks; ++r)
                {
                    LowerTriangularSolver(r, c, A);
                }

                for (size_t k = c + 1; k < NumBlocks; ++k)
                {
                    for (size_t r = k; r < NumBlocks; ++r)
                    {
                        SubtractiveUpdate(r, k, c, A);
                    }
                }
            }
            return true;
        }

        // The input matrix A must be symmetric. The output matrix is L which
        // is lower triangular and A = L * L^T. You must pass L to SolveLower
        // and SolveUpper.
        bool Factor(BlockMatrix const& A, BlockMatrix& L) const
        {
            L = A;
            if (Factor(L))
            {
                Matrix<T> zeroBlock(BlockSize, BlockSize);
                for (size_t r = 0; r < NumBlocks; ++r)
                {
                    // Set the upper-triangular parts of the diagonal blocks
                    // to zero.
                    for (size_t j = 0; j < BlockSize; ++j)
                    {
                        for (size_t i = j + 1; i < BlockSize; ++i)
                        {
                            L[r + NumBlocks * r](j, i) = C_<T>(0);
                        }
                    }

                    // Set the upper-triangular blocks to zero.
                    for (size_t c = r + 1; c < NumBlocks; ++c)
                    {
                        L[c + NumBlocks * r] = zeroBlock;
                    }
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        // Solve L*Y = B, where L is an invertible lower-triangular block
        // matrix whose diagonal blocks are lower-triangular matrices.
        // The input B is a block vector of commensurate size. The input
        // value of Y is B. On output, Y is the solution.
        void SolveLower(BlockMatrix const& L, BlockVector& Y) const
        {
            for (size_t r = 0; r < NumBlocks; ++r)
            {
                auto& Yr = Y[r];
                for (size_t c = 0; c < r; ++c)
                {
                    auto const& Lrc = L[GetIndex(r, c)];
                    auto const& Yc = Y[c];
                    for (size_t i = 0; i < NumBlocks; ++i)
                    {
                        for (size_t j = 0; j < NumBlocks; ++j)
                        {
                            Yr[i] -= Lrc[GetIndex(i, j)] * Yc[j];
                        }
                    }
                }
                CholeskyDecomposition<T> decomposer(BlockSize);
                decomposer.SolveLower(L[GetIndex(r, r)], Yr);
            }
        }

        // Solve L^T*X = Y, where L is an invertible lower-triangular block
        // matrix (L^T is an upper-triangular block matrix) whose diagonal
        // blocks are lower-triangular matrices. The input value of X is Y.
        // On output, X is the solution.
        void SolveUpper(BlockMatrix const& L, BlockVector& X) const
        {
            for (size_t k = 0, r = NumBlocks - 1; k < NumBlocks; ++k, --r)
            {
                auto& Xr = X[r];
                for (size_t c = r + 1; c < NumBlocks; ++c)
                {
                    auto const& Lcr = L[GetIndex(c, r)];
                    auto const& Xc = X[c];
                    for (size_t i = 0; i < BlockSize; ++i)
                    {
                        for (size_t j = 0; j < BlockSize; ++j)
                        {
                            Xr[i] -= Lcr[GetIndex(j, i)] * Xc[j];
                        }
                    }
                }
                CholeskyDecomposition<T> decomposer(BlockSize);
                decomposer.SolveUpper(L[GetIndex(r, r)], Xr);
            }
        }

    private:
        // Compute the 1-dimensional index of the block matrix in a
        // 2-dimensional BlockMatrix object.
        inline size_t GetIndex(size_t row, size_t col) const
        {
            return col + row * NumBlocks;
        }

        // Solve G(c,c)*G(r,c)^T = A(r,c)^T for G(r,c). The matrices
        // G(c,c) and A(r,c) are known quantities, and G(c,c) occupies
        // the lower triangular portion of A(c,c). The solver stores
        // its results in-place, so A(r,c) stores the G(r,c) result.
        void LowerTriangularSolver(size_t r, size_t c, BlockMatrix& A) const
        {
            auto const& Acc = A[GetIndex(c, c)];
            auto& Arc = A[GetIndex(r, c)];
            for (size_t j = 0; j < BlockSize; ++j)
            {
                for (size_t i = 0; i < j; ++i)
                {
                    T Lji = Acc[GetIndex(j, i)];
                    for (size_t k = 0; k < BlockSize; ++k)
                    {
                        Arc[GetIndex(k, j)] -= Lji * Arc[GetIndex(k, i)];
                    }
                }

                T Ljj = Acc[GetIndex(j, j)];
                for (size_t k = 0; k < BlockSize; ++k)
                {
                    Arc[GetIndex(k, j)] /= Ljj;
                }
            }
        }

        void SubtractiveUpdate(size_t r, size_t k, size_t c, BlockMatrix& A) const
        {
            auto const& Arc = A[GetIndex(r, c)];
            auto const& Akc = A[GetIndex(k, c)];
            auto& Ark = A[GetIndex(r, k)];
            for (size_t j = 0; j < BlockSize; ++j)
            {
                for (size_t i = 0; i < BlockSize; ++i)
                {
                    for (size_t m = 0; m < BlockSize; ++m)
                    {
                        Ark[GetIndex(j, i)] -= Arc[GetIndex(j, m)] * Akc[GetIndex(i, m)];
                    }
                }
            }
        }
    };
}
