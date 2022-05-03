// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Several functions are provided for bit manipulation. Some of these are
// required by the arbitrary precision arithmetic implementation in GTL. The
// functions GetLeadingBit, GetTrailingBit, Log2OfPowerOfTwo,
// RoundUpToPowerOfTwo and RoundDownToPowerOfTwo have preconditions for their
// inputs. If these are not satisfied, domain-error exceptions can be thrown.
// The default is NOT to throw exceptions but return reasonable values that
// indicate failure. If you want the exceptions thrown, add the preprocessor
// symbol GTL_THROW_ON_BIT_HACKS_ERROR to the global defines passed to the
// compiler.

#include <GTL/Utility/Exceptions.h>
#include <array>
#include <cstdint>
#include <limits>

namespace gtl
{
    class BitHacks
    {
    public:
        // The population-count operation counts the number of 1-bits in a
        // binary sequence.
        inline static uint32_t PopulationCount(uint32_t value)
        {
            value = (value & 0x55555555u) + ((value >> 1) & 0x55555555u);
            value = (value & 0x33333333u) + ((value >> 2) & 0x33333333u);
            value = (value & 0x0F0F0F0Fu) + ((value >> 4) & 0x0F0F0F0Fu);
            value = (value & 0x00FF00FFu) + ((value >> 8) & 0x00FF00FFu);
            value = (value & 0x0000FFFFu) + ((value >> 16) & 0x0000FFFFu);
            return value;
        }

        inline static uint32_t PopulationCount(uint64_t value)
        {
            value = (value & 0x5555555555555555ull) + ((value >> 1) & 0x5555555555555555ull);
            value = (value & 0x3333333333333333ull) + ((value >> 2) & 0x3333333333333333ull);
            value = (value & 0x0F0F0F0F0F0F0F0Full) + ((value >> 4) & 0x0F0F0F0F0F0F0F0Full);
            value = (value & 0x00FF00FF00FF00FFull) + ((value >> 8) & 0x00FF00FF00FF00FFull);
            value = (value & 0x0000FFFF0000FFFFull) + ((value >> 16) & 0x0000FFFF0000FFFFull);
            value = (value & 0x00000000FFFFFFFFull) + ((value >> 32) & 0x00000000FFFFFFFFull);
            return static_cast<uint32_t>(value & 0x00000000FFFFFFFFull);
        }

        // Count the number of leading 0-bits in a number.
        inline static uint32_t GetNumLeadingZeroBits(uint32_t value)
        {
            value = value | (value >> 1);
            value = value | (value >> 2);
            value = value | (value >> 4);
            value = value | (value >> 8);
            value = value | (value >> 16);
            return PopulationCount(~value);
        }

        inline static uint32_t GetNumLeadingZeroBits(uint64_t value)
        {
            value = value | (value >> 1);
            value = value | (value >> 2);
            value = value | (value >> 4);
            value = value | (value >> 8);
            value = value | (value >> 16);
            value = value | (value >> 32);
            return PopulationCount(~value);
        }

        // Count the number of trailing 0-bits in a number.
        inline static uint32_t GetNumTrailingZeroBits(uint32_t value)
        {
            return static_cast<uint32_t>(32 - GetNumLeadingZeroBits(~value & (value - 1)));
        }

        inline static uint32_t GetNumTrailingZeroBits(uint64_t value)
        {
            return static_cast<uint32_t>(64 - GetNumLeadingZeroBits(~value & (value - 1)));
        }

        // Find the leading 1-bit in a number. The input must be positive, in
        // which case the function returns a number between 0 and N-1 for
        // uintN_t for N in {32,62}. If the input is zero, then the function
        // returns 0xFFFFFFFFu. If the caller cannot guarantee value > 0 at
        // runtime, then a test of the return value is required to validate
        // the result.
        inline static uint32_t GetLeadingBit(uint32_t value)
        {
#if defined(GTL_THROW_ON_BIT_HACKS_ERROR)
            GTL_DOMAIN_ASSERT(
                value > 0,
                "The input must be positive.");
#endif
            if (value > 0)
            {
                return static_cast<uint32_t>(31 - GetNumLeadingZeroBits(value));
            }
            else
            {
                return std::numeric_limits<uint32_t>::max();
            }
        }

        inline static uint32_t GetLeadingBit(uint64_t value)
        {
#if defined(GTL_THROW_ON_BIT_HACKS_ERROR)
            GTL_DOMAIN_ASSERT(
                value > 0,
                "The input must be positive.");
#endif
            if (value > 0)
            {
                return static_cast<uint32_t>(63 - GetNumLeadingZeroBits(value));
            }
            else
            {
                return std::numeric_limits<uint32_t>::max();
            }
        }

        // Find the leading 1-bit in a number. The input must be positive, in
        // which case the function returns a number between 0 and N-1 for
        // uintN_t for N in {32,64}. If the input is zero, then the function
        // returns 0xFFFFFFFFu. If the caller cannot guarantee value > 0 at
        // runtime, then a test of the return value is required to validate
        // the result.
        inline static uint32_t GetTrailingBit(uint32_t value)
        {
#if defined(GTL_THROW_ON_BIT_HACKS_ERROR)
            GTL_DOMAIN_ASSERT(
                value > 0,
                "The input must be positive.");
#endif
            if (value > 0)
            {
                return GetNumTrailingZeroBits(value);
            }
            else
            {
                return std::numeric_limits<uint32_t>::max();
            }
        }

        inline static uint32_t GetTrailingBit(uint64_t value)
        {
#if defined(GTL_THROW_ON_BIT_HACKS_ERROR)
            GTL_DOMAIN_ASSERT(
                value > 0,
                "The input must be positive.");
#endif
            if (value > 0)
            {
                return GetNumTrailingZeroBits(value);
            }
            else
            {
                return std::numeric_limits<uint32_t>::max();
            }
        }

        // Test whether the number is a power of two. If value is zero, the
        // function returns false.
        inline static bool IsPowerOfTwo(uint32_t value)
        {
            return (value > 0) && ((value & (value - 1)) == 0);
        }

        inline static bool IsPowerOfTwo(uint64_t value)
        {
            return (value > 0) && ((value & (value - 1)) == 0);
        }

        // For power-of-two numbers, compute the power. The input must be
        // positive and a power of two. If is does not satisfy these
        // conditions, the function returns 0xFFFFFFFFu even though the
        // logarithm of zero is undefined.
        inline static uint32_t Log2OfPowerOfTwo(uint32_t value)
        {
            bool isPowerOfTwo = IsPowerOfTwo(value);
#if defined(GTL_THROW_ON_BIT_HACKS_ERROR)
            GTL_DOMAIN_ASSERT(
                isPowerOfTwo == true,
                "The input must be a power of 2.");
#endif
            if (isPowerOfTwo)
            {
                uint32_t log2 = (value & 0xAAAAAAAAu) != 0;
                log2 |= ((value & 0xFFFF0000u) != 0) << 4;
                log2 |= ((value & 0xFF00FF00u) != 0) << 3;
                log2 |= ((value & 0xF0F0F0F0u) != 0) << 2;
                log2 |= ((value & 0xCCCCCCCCu) != 0) << 1;
                return log2;
            }
            else
            {
                return std::numeric_limits<uint32_t>::max();
            }
        }

        inline static uint32_t Log2OfPowerOfTwo(uint64_t value)
        {
            bool isPowerOfTwo = IsPowerOfTwo(value);
#if defined(GTL_THROW_ON_BIT_HACKS_ERROR)
            GTL_DOMAIN_ASSERT(
                isPowerOfTwo == true,
                "The input must be a power of 2.");
#endif
            if (isPowerOfTwo)
            {
                uint64_t log2 = (value & 0xAAAAAAAAAAAAAAAAull) != 0;
                log2 |= static_cast<uint64_t>((value & 0xFFFFFFFF00000000ull) != 0) << 5;
                log2 |= static_cast<uint64_t>((value & 0xFFFF0000FFFF0000ull) != 0) << 4;
                log2 |= static_cast<uint64_t>((value & 0xFF00FF00FF00FF00ull) != 0) << 3;
                log2 |= static_cast<uint64_t>((value & 0xF0F0F0F0F0F0F0F0ull) != 0) << 2;
                log2 |= static_cast<uint64_t>((value & 0xCCCCCCCCCCCCCCCCull) != 0) << 1;
                return static_cast<uint32_t>(log2);
            }
            else
            {
                return std::numeric_limits<uint32_t>::max();
            }
        }

        // Round up to a power of two. Let v be the input value, which must be
        // positive. Let the returned array be {r[0],r[1]}. Let n = 32 for
        // uint32_t input or n = 64 for uint64_t input.  The possible outcomes
        // for r are
        //   v = 0, r = {1, 0} (if exceptions are disabled)
        //   v = 2^p, r = {2^p, 0}
        //   1 < 2^{p-1} < v < 2^p <= 2^{n-1}, r = {2^p, 0}
        //   2^{n-1} < v < 2^n, r = {0, 1}
        inline static std::array<uint32_t, 2> RoundUpToPowerOfTwo(uint32_t value)
        {
#if defined(GTL_THROW_ON_BIT_HACKS_ERROR)
            GTL_DOMAIN_ASSERT(
                value > 0,
                "The input must be positive.");
#endif
            std::array<uint32_t, 2> result{};
            if (value > 0)
            {
                uint32_t leading = GetLeadingBit(value);
                uint32_t mask = (1 << leading);
                if ((value & ~mask) == 0)
                {
                    // The value is a power of two.
                    result[0] = value;
                    result[1] = 0;
                }
                else
                {
                    // Round up to a power of two.
                    if ((mask & 0x80000000u) == 0)
                    {
                        result[0] = (mask << 1);
                        result[1] = 0;
                    }
                    else
                    {
                        result[0] = 0;
                        result[1] = 1;
                    }
                }
            }
            else
            {
                result[0] = 1;
                result[1] = 0;
            }
            return result;
        }

        inline static std::array<uint64_t, 2> RoundUpToPowerOfTwo(uint64_t value)
        {
#if defined(GTL_THROW_ON_BIT_HACKS_ERROR)
            GTL_DOMAIN_ASSERT(
                value > 0,
                "The input must be positive.");
#endif
            std::array<uint64_t, 2> result{};
            if (value > 0)
            {
                uint32_t leading = GetLeadingBit(value);
                uint64_t mask = (1ull << leading);
                if ((value & ~mask) == 0)
                {
                    // The value is a power of two.
                    result[0] = value;
                    result[1] = 0;
                }
                else
                {
                    // Round up to a power of two.
                    if ((mask & 0x8000000000000000ull) == 0)
                    {
                        result[0] = (mask << 1);
                        result[1] = 0;
                    }
                    else
                    {
                        result[0] = 0;
                        result[1] = 1;
                    }
                }
            }
            else
            {
                result[0] = 1;
                result[1] = 0;
        }
            return result;
        }

        // Round down to a power of two. Let v be the input value, which must
        // be positive. Let the returned value be r = 2^p, where p is the
        // power for which 2^p <= v < 2^{p+1}. If v is zero and exceptions
        // are disabled, the return value is 0.
        inline static uint32_t RoundDownToPowerOfTwo(uint32_t value)
        {
#if defined(GTL_THROW_ON_BIT_HACKS_ERROR)
            GTL_DOMAIN_ASSERT(
                value > 0,
                "The input must be positive.");
#endif
            if (value > 0)
            {
                uint32_t leading = GetLeadingBit(value);
                uint32_t mask = (1 << leading);
                return mask;
            }
            else
            {
                return 0;
            }
        }

        inline static uint64_t RoundDownToPowerOfTwo(uint64_t value)
        {
#if defined(GTL_THROW_ON_BIT_HACKS_ERROR)
            GTL_DOMAIN_ASSERT(
                value > 0,
                "The input must be positive.");
#endif
            if (value > 0)
            {
                uint32_t leading = GetLeadingBit(value);
                uint64_t mask = (1ull << leading);
                return mask;
            }
            else
            {
                return 0;
            }
        }

    private:
        friend class UnitTestBitHacks;
    };
}
