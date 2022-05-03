// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Support for unsigned integer arithmetic in BSNumber and BSRational. The
// class UIntegerALU32 provides a namespace for the (A)rithmetic (L)ogic
// (U)nit associated with arbitrary-precision arithmetic. The derived
// classes UIntegerAP32 (unlimited storage using std::vector) and
// UIntegerFP32<N> (predetermined storage using std::array) use the
// services provided here. The template type UInteger minimally must have
// the following interface:
//
// class UInteger
// {
// public:
//     // The number of bits required to store a nonzero UInteger. If the
//     // UInteger is 0, the number of bits is 0. If the UInteger is positive,
//     // the number of bits is the index of the leading 1-bit plus 1.
//     size_t GetNumBits() const;
//     void SetNumBits(size_t);
//
//     // Get access to the sequence of bits, where Container is a type that
//     // stores contiguous bits in 32-bit blocks. If 'bits' is a Container
//     // type, then it must support dereferencing by 'bits[i]' to access the
//     // i-th block, where 'i' is of type size_t. UIntegerAP32 has Container
//     // = std::vector<uint32_t> and UIntegerFP32<N> has Container =
//     // std::array<uint32_t, N>.
//     Container const& GetBits() const;
//     Container& GetBits();
//
//     // The number of blocks used to store a UInteger. The block
//     // bits[GetNumBlocks()-1] must have at least one 1-bit. There is no
//     // need for 'void SetNumBlocks(size_t)' because the number of blocks
//     // is modified by 'void SetNumBits(size_t)'.
//     size_t GetNumBlocks() const;
//
//     // Access the high-order block. It is necessary that the bits[] array
//     // has at least one block.
//     void SetBack(uint32_t);
//     uint32_t GetBack() const;
// };
//
// IMPORTANT NOTE. The classes UIntegerALU32, UIntegerAP32 and UIntegerFP32
// are designed to work with BSNumber. The constructors and arithmetic
// operators all work to ensure that the UInteger objects are either 0 or
// an odd number.

#include <GTL/Mathematics/Arithmetic/BitHacks.h>
#include <GTL/Utility/Exceptions.h>
#include <algorithm>

namespace gtl
{
    template <typename UInteger>
    class UIntegerALU32
    {
    public:
        // Comparisons. These are not generic. They rely on being called when
        // two BSNumber arguments to BSNumber::operatorX() are of the form
        // 1.u*2^p and 1.v*2^p. The comparisons apply to 1.u and 1.v as
        // unsigned integers with their leading 1-bits aligned.

        static bool Equal(UInteger const& n0, UInteger const& n1)
        {
            size_t const numBits0 = n0.GetNumBits();
            size_t const numBits1 = n1.GetNumBits();
            if (numBits0 != numBits1)
            {
                return false;
            }

            if (numBits0 > 0)
            {
                size_t const numBlocks0 = n0.GetNumBlocks();
                auto const& bits0 = n0.GetBits();
                auto const& bits1 = n1.GetBits();
                size_t numBlocks0M1 = static_cast<size_t>(numBlocks0 - 1);
                for (size_t j = 0, i = numBlocks0M1; j < numBlocks0; ++j, --i)
                {
                    if (bits0[i] != bits1[i])
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        static bool NotEqual(UInteger const& n0, UInteger const& n1)
        {
            return !Equal(n0, n1);
        }

        static bool LessThan(UInteger const& n0, UInteger const& n1)
        {
            size_t const numBits0 = n0.GetNumBits();
            size_t const numBits1 = n1.GetNumBits();
            if (numBits0 > 0 && numBits1 > 0)
            {
                // The numbers must be compared as if they are left-aligned
                // with each other. We got here because n0 = 1.u * 2^p and
                // n1 = 1.v * 2^p. Although they have the same exponent, it
                // is possible that n0 < n1 but numBits0(1u) > numBits0(1v).
                // Compare the bits one 32-bit block at a time.
                auto const& bits0 = n0.GetBits();
                auto const& bits1 = n1.GetBits();
                size_t const bitIndex0 = static_cast<size_t>(numBits0 - 1);
                size_t const bitIndex1 = static_cast<size_t>(numBits1 - 1);
                uint32_t const numBlockBits0 = static_cast<uint32_t>(1 + (bitIndex0 % 32));
                uint32_t const numBlockBits1 = static_cast<uint32_t>(1 + (bitIndex1 % 32));
                int64_t blockIndex0 = static_cast<int64_t>(bitIndex0 / 32);
                int64_t blockIndex1 = static_cast<int64_t>(bitIndex1 / 32);

                // The type of block0 and block1 is uint64_t because it is
                // possible that a right-shift of 32 can occur. In this case
                // right-shift 32 of a 32-bit quantity is undefined and the
                // output is dependent on the compiler. For example, when
                // compiling with MSVS 2019 (16.5.1), if u is a uint32_t, the
                // result of (u >> 32) is u, not 0.
                uint64_t block0 = static_cast<uint64_t>(bits0[blockIndex0]);
                uint64_t block1 = static_cast<uint64_t>(bits1[blockIndex1]);
                while (blockIndex0 >= 0 && blockIndex1 >= 0)
                {
                    uint32_t const lshift0 = static_cast<uint32_t>(32 - numBlockBits0);
                    uint32_t const lshift1 = static_cast<uint32_t>(32 - numBlockBits1);
                    uint32_t value0 = static_cast<uint32_t>((block0 << lshift0) & 0x00000000FFFFFFFFull);
                    uint32_t value1 = static_cast<uint32_t>((block1 << lshift1) & 0x00000000FFFFFFFFull);

                    // Shift bits in the next block (if any) to fill the
                    // current block.
                    if (--blockIndex0 >= 0)
                    {
                        block0 = static_cast<uint64_t>(bits0[blockIndex0]);
                        value0 |= static_cast<uint32_t>((block0 >> numBlockBits0) & 0x00000000FFFFFFFFull);
                    }
                    if (--blockIndex1 >= 0)
                    {
                        block1 = static_cast<uint64_t>(bits1[blockIndex1]);
                        value1 |= static_cast<uint32_t>((block1 >> numBlockBits1) & 0x00000000FFFFFFFFull);
                    }
                    if (value0 < value1)
                    {
                        return true;
                    }
                    if (value0 > value1)
                    {
                        return false;
                    }
                }
                return blockIndex0 < blockIndex1;
            }
            else
            {
                // One or both numbers are zero. The only time 'less than'
                // is 'true' is when 'n1' is positive.
                return numBits1 > 0;
            }
        }

        static bool LessThanOrEqual(UInteger const& n0, UInteger const& n1)
        {
            return !LessThan(n1, n0);
        }

        static bool GreaterThan(UInteger const& n0, UInteger const& n1)
        {
            return LessThan(n1, n0);
        }

        static bool GreaterThanOrEqual(UInteger const& n0, UInteger const& n1)
        {
            return !LessThan(n0, n1);
        }

        // Arithmetic operations. These are static to allow passing in the
        // inputs, computing the output, and avoid the (potential) object
        // copy when the signature instead is
        //   static UInteger Operation(Arguments);
        // These operations support BSNumber arithmetic. The preconditions are
        // that the inputs n0 and n1 are odd integers.

        static void Add(UInteger const& n0, UInteger const& n1, UInteger& result)
        {
            size_t const n0NumBits = n0.GetNumBits();
            size_t const n1NumBits = n1.GetNumBits();

            // Add the numbers considered as positive integers. Set the last
            // block to zero in case no carry-out occurs.
            size_t const numBits = std::max(n0NumBits, n1NumBits) + 1;
            result.SetNumBits(numBits);
            result.SetBack(0);

            // Get the input array sizes.
            size_t const numBlocks0 = n0.GetNumBlocks();
            size_t const numBlocks1 = n1.GetNumBlocks();

            // Order the inputs so that the first has the most blocks.
            auto const& u0 = (numBlocks0 >= numBlocks1 ? n0.GetBits() : n1.GetBits());
            auto const& u1 = (numBlocks0 >= numBlocks1 ? n1.GetBits() : n0.GetBits());
            auto numBlocks = std::minmax(numBlocks0, numBlocks1);

            // Add the u1-blocks to u0-blocks.
            auto& bits = result.GetBits();
            uint64_t carry = 0;
            size_t i{};
            for (i = 0; i < numBlocks.first; ++i)
            {
                uint64_t sum = u0[i] + (u1[i] + carry);
                bits[i] = static_cast<uint32_t>(sum & 0x00000000FFFFFFFFull);
                carry = (sum >> 32);
            }

            // We have no more u1-blocks. Propagate the carry-out, if there is
            // one, or copy the remaining blocks if there is not.
            if (carry > 0)
            {
                for ( ; i < numBlocks.second; ++i)
                {
                    uint64_t sum = u0[i] + carry;
                    bits[i] = static_cast<uint32_t>(sum & 0x00000000FFFFFFFFull);
                    carry = (sum >> 32);
                }
                if (carry > 0)
                {
                    bits[i] = static_cast<uint32_t>(carry & 0x00000000FFFFFFFFull);
                }
            }
            else
            {
                for ( ; i < numBlocks.second; ++i)
                {
                    bits[i] = u0[i];
                }
            }

            // Reduce the number of bits if there was not a carry-out.
            size_t const numBitsM1 = static_cast<size_t>(numBits - 1);
            uint32_t const firstBitIndex = static_cast<uint32_t>(numBitsM1 % 32);
            uint32_t const mask = (1 << firstBitIndex);
            if ((mask & result.GetBack()) == 0)
            {
                result.SetNumBits(numBitsM1);
            }
        }

        // Subtraction has the precondition n0 > n1, which is guaranteed by
        // the BSNumber class.
        static void Sub(UInteger const& n0, UInteger const& n1, UInteger& result)
        {
            size_t const numBits0 = n0.GetNumBits();
            auto const& bits0 = n0.GetBits();
            auto const& bits1 = n1.GetBits();

            // Subtract the numbers considered as positive integers. We know
            // that n0 > n1, so create a number n2 that has the same number of
            // bits as n0 and use two's-complement to generate -n2, and then
            // add n0 and -n2. The result is positive, so we do not need to
            // apply two's complement to a negative result to extract the sign
            // and absolute value.

            // Get the input array sizes. We know that
            // numBlocks0 >= numBlocks1.
            size_t const numBlocks0 = n0.GetNumBlocks();
            size_t const numBlocks1 = n1.GetNumBlocks();

            // Create the two's-complement number n2. We know that
            // n2.GetNumBlocks() is the same as numBlocks0.
            UInteger n2{};
            n2.SetNumBits(numBits0);
            auto& bits2 = n2.GetBits();
            size_t i{};
            for (i = 0; i < numBlocks1; ++i)
            {
                bits2[i] = ~bits1[i];
            }
            for (; i < numBlocks0; ++i)
            {
                bits2[i] = ~0u;
            }

            // Now add 1 to the bit-negated result to obtain -n1.
            uint64_t carry = 1;
            for (i = 0; i < numBlocks0; ++i)
            {
                uint64_t sum = bits2[i] + carry;
                bits2[i] = static_cast<uint32_t>(sum & 0x00000000FFFFFFFFull);
                carry = (sum >> 32);
            }

            // Add the numbers as positive integers. Set the last block to
            // zero in case no carry-out occurs.
            result.SetNumBits(numBits0 + 1);
            result.SetBack(0);

            // Add the n0-blocks to n2-blocks.
            auto& bits = result.GetBits();
            for (i = 0, carry = 0; i < numBlocks0; ++i)
            {
                uint64_t sum = bits2[i] + (bits0[i] + carry);
                bits[i] = static_cast<uint32_t>(sum & 0x00000000FFFFFFFFull);
                carry = (sum >> 32);
            }

            // Strip off the bits introduced by two's-complement.
            bool foundNonzeroBlock = false;
            size_t block = static_cast<size_t>(numBlocks0 - 1);
            for (size_t j = 0; j < numBlocks0; ++j, --block)
            {
                if (bits[block] > 0)
                {
                    foundNonzeroBlock = true;
                    break;
                }
            }

            if (foundNonzeroBlock)
            {
                size_t constexpr bitsPerBlock = 32;
                size_t const leading = static_cast<size_t>(BitHacks::GetLeadingBit(bits[block]));
                result.SetNumBits(bitsPerBlock * block + leading + 1);
            }
            else
            {
                // TODO: This block originally did not exist in GTE, only the
                // if-block did. During some testing for the RAEGC book, a
                // crash occurred where it appeared the block was needed. I
                // added this block to fix the problem, but had forgotten the
                // precondition that n0 > n1. Consequently, I did not look
                // carefully at the inputs and call stack to determine how
                // this could have happened. Trap this problem and analyze the
                // call stack and inputs that lead to this case if it happens
                // again. The call stack is started by
                // BSNumber::SubIgnoreSign(...).
                result.SetNumBits(0);
                GTL_RUNTIME_ERROR(
                    "The difference of the numbers is zero, which violates the precondition n0 > n1.");
            }
        }

        static void Mul(UInteger const& n0, UInteger const& n1, UInteger& result)
        {
            size_t const numBits0 = n0.GetNumBits();
            size_t const numBits1 = n1.GetNumBits();
            auto const& bits0 = n0.GetBits();
            auto const& bits1 = n1.GetBits();

            // The number of bits is at most this, possibly one bit smaller.
            size_t const numBits = numBits0 + numBits1;
            result.SetNumBits(numBits);
            auto& bits = result.GetBits();

            // Product of a single-block number with a multiple-block number.
            UInteger product{};
            product.SetNumBits(numBits);
            auto& pBits = product.GetBits();

            // Get the array sizes.
            size_t const numBlocks0 = n0.GetNumBlocks();
            size_t const numBlocks1 = n1.GetNumBlocks();
            size_t const numBlocks = result.GetNumBlocks();

            // Compute the product v = u0*u1.
            size_t i0, i1, i2;

            // The case i0 == 0 is handled separately to initialize the
            // accumulator with u0[0]*v. This avoids having to fill the bytes
            // of 'bits' with zeros outside the double loop, something that
            // can be a performance issue when 'numBits' is large.
            uint64_t block0 = bits0[0];
            uint64_t carry = 0;
            for (i1 = 0; i1 < numBlocks1; ++i1)
            {
                uint64_t term = block0 * bits1[i1] + carry;
                bits[i1] = static_cast<uint32_t>(term & 0x00000000FFFFFFFFull);
                carry = (term >> 32);
            }
            if (i1 < numBlocks)
            {
                bits[i1] = static_cast<uint32_t>(carry & 0x00000000FFFFFFFFull);
            }

            for (i0 = 1; i0 < numBlocks0; ++i0)
            {
                // Compute the product p = u0[i0]*u1.
                block0 = bits0[i0];
                carry = 0;
                for (i1 = 0, i2 = i0; i1 < numBlocks1; ++i1, ++i2)
                {
                    uint64_t term = block0 * bits1[i1] + carry;
                    pBits[i2] = static_cast<uint32_t>(term & 0x00000000FFFFFFFFull);
                    carry = (term >> 32);
                }
                if (i2 < numBlocks)
                {
                    pBits[i2] = static_cast<uint32_t>(carry & 0x00000000FFFFFFFFull);
                }

                // Add p to the accumulator v.
                carry = 0;
                for (i1 = 0, i2 = i0; i1 < numBlocks1; ++i1, ++i2)
                {
                    uint64_t sum = pBits[i2] + (bits[i2] + carry);
                    bits[i2] = static_cast<uint32_t>(sum & 0x00000000FFFFFFFFull);
                    carry = (sum >> 32);
                }
                if (i2 < numBlocks)
                {
                    uint64_t sum = pBits[i2] + carry;
                    bits[i2] = static_cast<uint32_t>(sum & 0x00000000FFFFFFFFull);
                }
            }

            // Reduce the number of bits if there was not a carry-out.
            size_t const numBitsM1 = static_cast<size_t>(numBits - 1);
            uint32_t const firstBitIndex = static_cast<uint32_t>(numBitsM1 % 32);
            uint32_t const mask = (1 << firstBitIndex);
            if ((mask & result.GetBack()) == 0)
            {
                result.SetNumBits(numBitsM1);
            }
        }

        // Shift the bits of n0 to the left by the specified amount.
        static void ShiftLeft(UInteger const& n0, int32_t inShift, UInteger& result)
        {
            GTL_ARGUMENT_ASSERT(
                inShift > 0,
                "Shift must be positive.");

            size_t shift = static_cast<size_t>(inShift);

            size_t numBits0 = n0.GetNumBits();
            auto const& bits0 = n0.GetBits();

            // Shift n0 considered as an odd positive integer.
            result.SetNumBits(numBits0 + shift);

            // Set the low-order bits to zero.
            auto& bits = result.GetBits();
            size_t const shiftBlock = static_cast<size_t>(shift / 32);
            size_t i{};
            for (i = 0; i < shiftBlock; ++i)
            {
                bits[i] = 0;
            }

            // Get the location of the low-order 1-bit within the result.
            size_t const numBlocks0 = n0.GetNumBlocks();
            uint32_t const lshift = shift % 32;
            size_t j{};
            if (lshift > 0)
            {
                // The trailing 1-bits for source and target are at different
                // relative indices. Each shifted source block straddles a
                // boundary between two target blocks, so we must extract the
                // subblocks and copy accordingly.
                uint32_t const rshift = static_cast<uint32_t>(32 - lshift);
                uint32_t prev = 0;
                for (i = shiftBlock, j = 0; j < numBlocks0; ++i, ++j)
                {
                    uint32_t const curr = bits0[j];
                    bits[i] = (curr << lshift) | (prev >> rshift);
                    prev = curr;
                }
                if (i < result.GetNumBlocks())
                {
                    // The leading 1-bit of the source is at a relative index
                    // such that when you add the shift amount, that bit
                    // occurs in a new block.
                    bits[i] = (prev >> rshift);
                }
            }
            else
            {
                // The trailing 1-bits for source and target are at the same
                // relative index. The shift reduces to a block copy.
                for (i = shiftBlock, j = 0; j < numBlocks0; ++i, ++j)
                {
                    bits[i] = bits0[j];
                }
            }
        }

        // The preconditions are that n0 is even and positive. It is shifted
        // right to become an odd number and the return value is the amount
        // shifted.
        static int32_t ShiftRightToOdd(UInteger const& n0, UInteger& result)
        {
            auto const& bits0 = n0.GetBits();

            // Get the leading 1-bit.
            size_t const numBlocks0 = n0.GetNumBlocks();
            size_t const numBlocks0M1 = static_cast<size_t>(numBlocks0 - 1);
            size_t const leading = static_cast<size_t>(BitHacks::GetLeadingBit(bits0[numBlocks0M1]));
            size_t constexpr bitsPerBlock = 32;
            size_t const firstBitIndex = static_cast<size_t>(bitsPerBlock * numBlocks0M1 + leading);

            // Get the trailing 1-bit.
            size_t lastBitIndex = std::numeric_limits<size_t>::max();
            for (size_t block = 0; block < numBlocks0; ++block)
            {
                uint32_t value = bits0[block];
                if (value > 0)
                {
                    size_t trailing = static_cast<size_t>(BitHacks::GetTrailingBit(value));
                    lastBitIndex = bitsPerBlock * block + trailing;
                    break;
                }
            }
            // As long as the preconditions n0 even and positive are satisfied,
            // lastBitIndex will not be std::numeric_limits<size_t>::max().

            // The right-shifted result.
            size_t const numBits = static_cast<size_t>(firstBitIndex - lastBitIndex) + 1;
            result.SetNumBits(numBits);
            auto& bits = result.GetBits();
            size_t const numBlocks = result.GetNumBlocks();

            // Get the location of the low-order 1-bit within the result.
            size_t const shiftBlock = lastBitIndex / 32;
            uint32_t const rshift = static_cast<uint32_t>(lastBitIndex % 32);
            if (rshift > 0)
            {
                uint32_t const lshift = static_cast<uint32_t>(32 - rshift);
                size_t i{}, j = shiftBlock;
                uint32_t curr = bits0[j++];
                for (i = 0; j < numBlocks0; ++i, ++j)
                {
                    uint32_t const next = bits0[j];
                    bits[i] = (curr >> rshift) | (next << lshift);
                    curr = next;
                }
                if (i < numBlocks)
                {
                    bits[i] = (curr >> rshift);
                }
            }
            else
            {
                for (size_t i = 0, j = shiftBlock; i < numBlocks; ++i, ++j)
                {
                    bits[i] = bits0[j];
                }
            }

            size_t fullRShift = bitsPerBlock * shiftBlock + static_cast<size_t>(rshift);
            return static_cast<int32_t>(fullRShift);
        }

        // Add 1 to the input, useful for rounding modes in conversions of
        // BSNumber and BSRational. The return value is the amount shifted
        // after the addition in order to obtain an odd integer.
        static int32_t RoundUp(UInteger& n0)
        {
            UInteger rounded{};
            Add(n0, UInteger(1u), rounded);
            return ShiftRightToOdd(rounded, n0);
        }

    private:
        friend class UnitTestUIntegerALU32;
    };
}
