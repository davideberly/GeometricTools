// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Support for determining the number of bits of precision required to compute
// an expression using BSNumber or BSRational.

#include <GTL/Mathematics/Arithmetic/IEEEBinary.h>
#include <algorithm>
#include <cstdint>
#include <limits>

namespace gtl
{
    class BSPrecision
    {
    public:
        enum class Type
        {
            IS_FLOAT,
            IS_DOUBLE,
            IS_INT32,
            IS_UINT32,
            IS_INT64,
            IS_UINT64
        };

        struct Parameters
        {
            Parameters()
                :
                minExponent(0),
                maxExponent(0),
                maxBits(0),
                maxWords(0)
            {
            }

            Parameters(int32_t inMinExponent, int32_t inMaxExponent, int32_t inMaxBits)
                :
                minExponent(inMinExponent),
                maxExponent(inMaxExponent),
                maxBits(inMaxBits),
                maxWords(GetMaxWords())
            {
            }

            inline int32_t GetMaxWords() const
            {
                return maxBits / 32 + ((maxBits % 32) > 0 ? 1 : 0);
            }

            inline bool operator==(Parameters const& other) const
            {
                return minExponent == other.minExponent
                    && maxExponent == other.maxExponent
                    && maxBits == other.maxBits
                    && maxWords == other.maxWords;
            }

            int32_t minExponent, maxExponent, maxBits, maxWords;
        };


        BSPrecision()
            :
            bsn{},
            bsr{}
        {
        }

        BSPrecision(Type type)
            :
            bsn{},
            bsr{}
        {
            switch (type)
            {
            case Type::IS_FLOAT:
                bsn = Parameters(
                    IEEEBinary32::MIN_EXPONENT,
                    IEEEBinary32::EXPONENT_BIAS,
                    IEEEBinary32::NUM_SIGNIFICAND_BITS);
                break;
            case Type::IS_DOUBLE:
                bsn = Parameters(
                    IEEEBinary64::MIN_EXPONENT,
                    IEEEBinary64::EXPONENT_BIAS,
                    IEEEBinary64::NUM_SIGNIFICAND_BITS);
                break;
            case Type::IS_INT32:
                bsn = Parameters(
                    0,
                    std::numeric_limits<int32_t>::digits - 1,
                    std::numeric_limits<int32_t>::digits);
                break;
            case Type::IS_UINT32:
                bsn = Parameters(
                    0,
                    std::numeric_limits<uint32_t>::digits - 1,
                    std::numeric_limits<uint32_t>::digits);
                break;
            case Type::IS_INT64:
                bsn = Parameters(
                    0,
                    std::numeric_limits<int64_t>::digits - 1,
                    std::numeric_limits<int64_t>::digits);
                    break;
            case Type::IS_UINT64:
                bsn = Parameters(
                    0,
                    std::numeric_limits<uint64_t>::digits - 1,
                    std::numeric_limits<uint64_t>::digits);
                break;
            }
            bsr = bsn;
        }

        BSPrecision(int32_t minExponent, int32_t maxExponent, int32_t maxBits)
            :
            bsn(minExponent, maxExponent, maxBits),
            bsr(minExponent, maxExponent, maxBits)
        {
        }

        Parameters bsn, bsr;

    private:
        friend class UnitTestBSPrecision;
    };

    inline BSPrecision operator+(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        BSPrecision result{};

        result.bsn.minExponent = std::min(bsp0.bsn.minExponent, bsp1.bsn.minExponent);
        if (bsp0.bsn.maxExponent >= bsp1.bsn.maxExponent)
        {
            result.bsn.maxExponent = bsp0.bsn.maxExponent;
            if (bsp0.bsn.maxExponent - bsp0.bsn.maxBits + 1 <= bsp1.bsn.maxExponent)
            {
                ++result.bsn.maxExponent;
            }

            result.bsn.maxBits = bsp0.bsn.maxExponent - bsp1.bsn.minExponent + 1;
            if (result.bsn.maxBits <= bsp0.bsn.maxBits + bsp1.bsn.maxBits - 1)
            {
                ++result.bsn.maxBits;
            }
        }
        else
        {
            result.bsn.maxExponent = bsp1.bsn.maxExponent;
            if (bsp1.bsn.maxExponent - bsp1.bsn.maxBits + 1 <= bsp0.bsn.maxExponent)
            {
                ++result.bsn.maxExponent;
            }

            result.bsn.maxBits = bsp1.bsn.maxExponent - bsp0.bsn.minExponent + 1;
            if (result.bsn.maxBits <= bsp0.bsn.maxBits + bsp1.bsn.maxBits - 1)
            {
                ++result.bsn.maxBits;
            }
        }
        result.bsn.maxWords = result.bsn.GetMaxWords();

        // Addition is n0/d0 + n1/d1 = (n0*d1 + n1*d0)/(d0*d1). The numerator
        // and denominator of a number are assumed to have the same
        // parameters, so for the addition, the numerator is used for the
        // parameter computations.

        // Compute the parameters for the multiplication.
        int32_t mulMinExponent = bsp0.bsr.minExponent + bsp1.bsr.minExponent;
        int32_t mulMaxExponent = bsp0.bsr.maxExponent + bsp1.bsr.maxExponent + 1;
        int32_t mulMaxBits = bsp0.bsr.maxBits + bsp1.bsr.maxBits;

        // Compute the parameters for the addition. The number n0*d1 and n1*d0
        // are in the same arbitrary-precision set.
        result.bsr.minExponent = mulMinExponent;
        result.bsr.maxExponent = mulMaxExponent + 1; // Always a carry-out.
        result.bsr.maxBits = mulMaxExponent - mulMinExponent + 1;
        if (result.bsr.maxBits <= 2 * mulMaxBits - 1)
        {
            ++result.bsr.maxBits;
        }
        result.bsr.maxWords = result.bsr.GetMaxWords();

        return result;
    }

    inline BSPrecision operator-(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return bsp0 + bsp1;
    }

    inline BSPrecision operator*(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        BSPrecision result{};

        result.bsn.minExponent = bsp0.bsn.minExponent + bsp1.bsn.minExponent;
        result.bsn.maxExponent = bsp0.bsn.maxExponent + bsp1.bsn.maxExponent + 1;
        result.bsn.maxBits = bsp0.bsn.maxBits + bsp1.bsn.maxBits;
        result.bsn.maxWords = result.bsn.GetMaxWords();

        // Multiplication is (n0/d0) * (n1/d1) = (n0 * n1) / (d0 * d1). The
        // parameters are the same as for numerator/denominator.
        result.bsr.minExponent = bsp0.bsr.minExponent + bsp1.bsr.minExponent;
        result.bsr.maxExponent = bsp0.bsr.maxExponent + bsp1.bsr.maxExponent + 1;
        result.bsr.maxBits = bsp0.bsr.maxBits + bsp1.bsr.maxBits;
        result.bsr.maxWords = result.bsr.GetMaxWords();

        return result;
    }

    inline BSPrecision operator/(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        BSPrecision result{};

        // BSNumber does not support division, so result.bsr has all members
        // set to zero.

        // Division is (n0/d0) / (n1/d1) = (n0 * d1) / (n1 * d0). The
        // parameters are the same as for multiplication.
        result.bsr.minExponent = bsp0.bsr.minExponent + bsp1.bsr.minExponent;
        result.bsr.maxExponent = bsp0.bsr.maxExponent + bsp1.bsr.maxExponent + 1;
        result.bsr.maxBits = bsp0.bsr.maxBits + bsp1.bsr.maxBits;
        result.bsr.maxWords = result.bsr.GetMaxWords();

        return result;
    }

    // Comparisons for BSNumber do not involve dynamic allocations, so
    // the results are the extremes of the inputs. Comparisons for BSRational
    // involve multiplications of numerators and denominators.
    inline BSPrecision operator==(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        BSPrecision result{};

        result.bsn.minExponent = std::min(bsp0.bsn.minExponent, bsp1.bsn.minExponent);
        result.bsn.maxExponent = std::max(bsp0.bsn.maxExponent, bsp1.bsn.maxExponent);
        result.bsn.maxBits = std::max(bsp0.bsn.maxBits, bsp1.bsn.maxBits);
        result.bsn.maxWords = result.bsn.GetMaxWords();

        result.bsr.minExponent = bsp0.bsr.minExponent + bsp1.bsr.minExponent;
        result.bsr.maxExponent = bsp0.bsr.maxExponent + bsp1.bsr.maxExponent + 1;
        result.bsr.maxBits = bsp0.bsr.maxBits + bsp1.bsr.maxBits;
        result.bsr.maxWords = result.bsr.GetMaxWords();

        return result;
    }

    inline BSPrecision operator!=(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return operator==(bsp0, bsp1);
    }

    inline BSPrecision operator<(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return operator==(bsp0, bsp1);
    }

    inline BSPrecision operator<=(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return operator==(bsp0, bsp1);
    }

    inline BSPrecision operator>(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return operator==(bsp0, bsp1);
    }

    inline BSPrecision operator>=(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return operator==(bsp0, bsp1);
    }
}
