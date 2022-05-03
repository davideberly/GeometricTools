// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Class UIntegerAP32 is designed to support arbitrary precision arithmetic
// using BSNumber and BSRational. It is not a general-purpose class for
// arithmetic of unsigned integers.
//
// To collect statistics on how large the UIntegerAP32 storage becomes when
// using it for the UInteger of BSNumber, add the preprocessor symbol
// GTL_COLLECT_UINTEGERAP32_STATISTICS to the global defines passed to the
// compiler.
// 
// If you use this feature, you must define gsUIntegerAP32MaxBlocks somewhere
// in your code. After a sequence of BSNumber operations, look at
// gsUIntegerAP32MaxBlocks in the debugger watch window. If the number is not
// too large, you might be safe in replacing UIntegerAP32 by UIntegerFP32<N>,
// where N is the value of gsUIntegerAP32MaxBlocks. This leads to faster code
// because you no longer have dynamic memory allocations and deallocations
// that occur regularly with std::vector<uint32_t> during BSNumber operations.
// A safer choice is to argue mathematically that the maximum size is bounded
// by N. This requires an analysis of how many bits of precision you need for
// the types of computation you perform. See class BSPrecision for code that
// allows you to compute maximum N.

#if defined(GTL_COLLECT_UINTEGERAP32_STATISTICS)
#include <GTL/Utility/AtomicMinMax.h>
namespace gtl
{
    extern std::atomic<size_t> gsUIntegerAP32MaxBlocks;
}
#endif

#include <GTL/Mathematics/Arithmetic/UIntegerALU32.h>
#include <GTL/Utility/Exceptions.h>
#include <algorithm>
#include <istream>
#include <limits>
#include <ostream>
#include <utility>
#include <vector>

namespace gtl
{
    class UIntegerAP32
    {
    public:
        // Construction and destruction.
        UIntegerAP32()
            :
            mNumBits(0),
            mBits{}
        {
        }

        UIntegerAP32(uint32_t number)
            :
            mNumBits(0),
            mBits{}
        {
            if (number > 0)
            {
                uint32_t const first = BitHacks::GetLeadingBit(number);
                uint32_t const last = BitHacks::GetTrailingBit(number);
                mNumBits = static_cast<size_t>(first - last) + 1;
                mBits.resize(1);
                mBits[0] = (number >> last);
            }
            else
            {
                mNumBits = 0;
            }

#if defined(GTL_COLLECT_UINTEGERAP32_STATISTICS)
            AtomicMax(gsUIntegerAP32MaxBlocks, mBits.size());
#endif
        }

        UIntegerAP32(uint64_t number)
            :
            mNumBits(0),
            mBits{}
        {
            if (number > 0)
            {
                uint32_t const first = BitHacks::GetLeadingBit(number);
                uint32_t const last = BitHacks::GetTrailingBit(number);
                number >>= last;
                size_t const numBitsM1 = static_cast<size_t>(first - last);
                mNumBits = numBitsM1 + 1;
                size_t const numBlocks = static_cast<size_t>(1 + numBitsM1 / 32);
                mBits.resize(numBlocks);
                mBits[0] = static_cast<uint32_t>(number & 0x00000000FFFFFFFFull);
                if (mBits.size() > 1)
                {
                    mBits[1] = static_cast<uint32_t>((number >> 32) & 0x00000000FFFFFFFFull);
                }
            }
            else
            {
                mNumBits = 0;
            }

#if defined(GTL_COLLECT_UINTEGERAP32_STATISTICS)
            AtomicMax(gsUIntegerAP32MaxBlocks, mBits.size());
#endif
        }

        ~UIntegerAP32() = default;

        // Copy semantics.
        UIntegerAP32(UIntegerAP32 const& other)
            :
            mNumBits(0),
            mBits{}
        {
            *this = other;
        }

        UIntegerAP32& operator=(UIntegerAP32 const& other)
        {
            mNumBits = other.mNumBits;
            mBits = other.mBits;
            return *this;
        }

        // Move semantics.
        UIntegerAP32(UIntegerAP32&& other) noexcept
            :
            mNumBits(0),
            mBits{}
        {
            *this = std::move(other);
        }

        UIntegerAP32& operator=(UIntegerAP32&& other) noexcept
        {
            mNumBits = other.mNumBits;
            mBits = std::move(other.mBits);
            other.mNumBits = 0;
            return *this;
        }

        // Member access.
        void SetNumBits(size_t numBits)
        {
            if (numBits > 0)
            {
                mNumBits = numBits;
                size_t const numBitsM1 = static_cast<size_t>(numBits - 1);
                size_t const numBlocks = static_cast<size_t>(1 + numBitsM1 / 32);
                mBits.resize(numBlocks);
            }
            else
            {
                mNumBits = 0;
                mBits.clear();
            }

#if defined(GTL_COLLECT_UINTEGERAP32_STATISTICS)
            AtomicMax(gsUIntegerAP32MaxBlocks, mBits.size());
#endif
        }

        inline size_t GetNumBits() const
        {
            return mNumBits;
        }

        inline std::vector<uint32_t> const& GetBits() const
        {
            return mBits;
        }

        inline std::vector<uint32_t>& GetBits()
        {
            return mBits;
        }

        inline size_t GetNumBlocks() const
        {
            return mBits.size();
        }

        inline static size_t GetMaxNumBlocks()
        {
            return std::numeric_limits<size_t>::max();
        }

        inline void SetBack(uint32_t value)
        {
            GTL_RUNTIME_ASSERT(
                mBits.size() > 0,
                "Cannot call SetBack on an empty mBits array.");

            mBits.back() = value;
        }

        inline uint32_t GetBack() const
        {
            GTL_RUNTIME_ASSERT(
                mBits.size() > 0,
                "Cannot call SetBack on an empty mBits array.");

            return mBits.back();
        }

        inline void SetAllBitsToZero()
        {
            std::fill(mBits.begin(), mBits.end(), 0u);
        }

        // Disk input/output. The fstream objects should be created using
        // std::ios::binary. The return value is 'true' iff the operation
        // is successful.
        bool Write(std::ostream& output) const
        {
            if (output.write((char const*)&mNumBits, sizeof(mNumBits)).bad())
            {
                return false;
            }

            size_t numBlocks = mBits.size();
            if (output.write((char const*)&numBlocks, sizeof(numBlocks)).bad())
            {
                return false;
            }

            return output.write((char const*)&mBits[0], numBlocks * sizeof(mBits[0])).good();
        }

        bool Read(std::istream& input)
        {
            if (input.read((char*)&mNumBits, sizeof(mNumBits)).bad())
            {
                return false;
            }

            size_t numBlocks = 0;
            if (input.read((char*)&numBlocks, sizeof(numBlocks)).bad())
            {
                return false;
            }

            mBits.resize(numBlocks);
            return input.read((char*)&mBits[0], numBlocks * sizeof(mBits[0])).good();
        }

    private:
        size_t mNumBits;
        std::vector<uint32_t> mBits;

        friend class UnitTestUIntegerAP32;
    };

    // Comparisons.
    inline bool operator==(UIntegerAP32 const& n0, UIntegerAP32 const& n1)
    {
        return UIntegerALU32<UIntegerAP32>::Equal(n0, n1);
    }

    inline bool operator!=(UIntegerAP32 const& n0, UIntegerAP32 const& n1)
    {
        return UIntegerALU32<UIntegerAP32>::NotEqual(n0, n1);
    }

    inline bool operator<(UIntegerAP32 const& n0, UIntegerAP32 const& n1)
    {
        return UIntegerALU32<UIntegerAP32>::LessThan(n0, n1);
    }

    inline bool operator<=(UIntegerAP32 const& n0, UIntegerAP32 const& n1)
    {
        return UIntegerALU32<UIntegerAP32>::LessThanOrEqual(n0, n1);
    }

    inline bool operator>(UIntegerAP32 const& n0, UIntegerAP32 const& n1)
    {
        return UIntegerALU32<UIntegerAP32>::GreaterThan(n0, n1);
    }

    inline bool operator>=(UIntegerAP32 const& n0, UIntegerAP32 const& n1)
    {
        return UIntegerALU32<UIntegerAP32>::GreaterThanOrEqual(n0, n1);
    }
}
