// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// The class BSNumber (binary scientific number) is designed to provide exact
// arithmetic for robust algorithms, typically those for which we need to know
// the exact sign of determinants. The template parameter UInteger must
// have support for the following public interface.
//
//      class UInteger
//      {
//      public:
//          // Construction and destruction.
//          UInteger();
//          UInteger(uint32_t number);
//          UInteger(uint64_t number);
//          ~UInteger();
//
//          // Copy semantics.
//          UInteger(UInteger const& other);
//          UInteger& operator=(UInteger const& nuothermber);
//
//          // Move semantics.
//          UInteger(UInteger&& other) noexcept;
//          UInteger& operator=(UInteger&& other) noexcept;
//
//          // The number of bits required to store a nonzero UInteger. If the
//          // UInteger is 0, the number of bits is 0. If the UInteger is
//          // positive, the number of bits is the index of the leading 1-bit
//          // plus 1.
//          void SetNumBits(size_t);
//          size_t GetNumBits() const;
//
//          // Get access to the sequence of bits, where Container is a type
//          // that stores contiguous bits in 32-bit blocks. If 'bits' is a
//          // Container type, then it must support dereferencing by 'bits[i]'
//          // to access the i-th block, where 'i' is of type size_t.
//          // UIntegerAP32 has Container = std::vector<uint32_t> and
//          // UIntegerFP32<N> has Container = std::array<uint32_t, N>.
//          Container const& GetBits() const;
//          Container& GetBits();
//
//          // The number of blocks used to store a UInteger. The block
//          // bits[GetNumBlocks()-1] must have at least one 1-bit. There is
//          // no need for 'void SetNumBlocks(size_t)' because the number of
//          // blocks is modified by 'void SetNumBits(size_t)'. The maximum
//          // number of blocks for UIntegerAP32 is effectively infinite, but
//          // set to std::numeric_limits<size_t>::max(). The maximum number
//          // of blocks for UIntegerFP32<N> is N.
//          size_t GetNumBlocks() const;
//          size_t GetMaxNumBlocks() const;
//
//          // Access the high-order block. It is necessary that the bits[]
//          // array has at least one block.
//          void SetBack(uint32_t);
//          uint32_t GetBack() const;
//
//          // Set all the bits to zero but neither compresses the storage
//          // nor sets mNumBits and mNumBlocks to 0.
//          void SetAllBitsToZero();
//
//          // Disk input/output. The fstream objects should be created using
//          // std::ios::binary. The return value is 'true' iff the operation
//          // is successful.
//          bool Write(std::ostream& output) const;
//          bool Read(std::istream& input);
//      };
//
//      bool operator==(UInteger const& n0, UInteger const& n1) const;
//      bool operator!=(UInteger const& n0, UInteger const& n1) const;
//      bool operator< (UInteger const& n0, UInteger const& n1) const;
//      bool operator<=(UInteger const& n0, UInteger const& n1) const;
//      bool operator> (UInteger const& n0, UInteger const& n1) const;
//      bool operator>=(UInteger const& n0, UInteger const& n1) const;
//
// The GTL currently has 32-bits-per-word storage for UInteger. See the
// classes UIntegerAP32 (arbitrary precision), UIntegerFP32<N> (fixed
// precision), and UIntegerALU32 (arithmetic logic unit shared by the
// previous two classes).

// The class can throw exceptions on various conditions. If you exceptions
// thrown for any of these conditions, add the preprocessor symbols to the
// global defines passed to the compiler.
//
// GTL_THROW_ON_INVALID_BSNUMBER:
// Support for unit testing algorithm correctness. The invariant for a
// nonzero BSNumber is that the UInteger part is a positive odd number.
// Expose this define to allow validation of the invariant.
//
// GTL_THROW_ON_CONVERT_FROM_INFINITY_OR_NAN:
// Enable this to throw exceptions in ConvertFrom when infinities or NaNs
// are the floating-point inputs. BSNumber does not have representations
// for these numbers.
//
// GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE:
// Support for debugging algorithms that use exact rational arithmetic. Each
// BSNumber and BSRational has a double-precision member that is exposed when
// the conditional define is enabled. Be aware that this can be very slow
// because of the conversion to double-precision whenever new objects are
// created by arithmetic operations. Also, the exact number might not be
// representable as a double-precision number. As a faster alternative, you
// can add temporary code in your algorithms that explicitly convert specific
// rational numbers to double precision.

#include <GTL/Mathematics/Arithmetic/BitHacks.h>
#include <GTL/Mathematics/Arithmetic/IEEEFunctions.h>
#include <GTL/Mathematics/Arithmetic/IEEEBinary.h>
#include <GTL/Mathematics/Arithmetic/UIntegerALU32.h>
#include <GTL/Utility/TypeTraits.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

namespace gtl
{
    // BSRational depends on the design of BSNumber. Allow it to have full
    // access to the implementation. To specify 'friend' status, the class
    // must be forward declared.
    template <typename UInteger> class BSRational;

    template <typename UInteger>
    class BSNumber
    {
    public:
        // Construction and destruction. The default constructor generates the
        // zero BSNumber.
        BSNumber()
            :
            mSign(0),
            mBiasedExponent(0),
            mUInteger{}
        {
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                IsValid(),
                "Invalid BSNumber.");
#endif
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = 0.0;
#endif
        }

        BSNumber(float number)
            :
            mSign(0),
            mBiasedExponent(0),
            mUInteger{}
        {
            ConvertFrom<IEEEBinary32>(number);
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                IsValid(),
                "Invalid BSNumber.");
#endif
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(number);
#endif
        }

        BSNumber(double number)
            :
            mSign(0),
            mBiasedExponent(0),
            mUInteger{}
        {
            ConvertFrom<IEEEBinary64>(number);
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                IsValid(),
                "Invalid BSNumber.");
#endif
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = number;
#endif
        }

        BSNumber(int32_t number)
            :
            mSign(0),
            mBiasedExponent(0),
            mUInteger{}
        {
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(number);
#endif
            if (number == 0)
            {
                mSign = 0;
                mBiasedExponent = 0;
            }
            else
            {
                if (number < 0)
                {
                    mSign = -1;
                    number = -number;
                }
                else
                {
                    mSign = 1;
                }

                uint32_t const unumber = static_cast<uint32_t>(number);
                mBiasedExponent = BitHacks::GetTrailingBit(unumber);
                mUInteger = unumber;
            }
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                IsValid(),
                "Invalid BSNumber.");
#endif
        }

        BSNumber(uint32_t number)
            :
            mSign(0),
            mBiasedExponent(0),
            mUInteger{}
        {
            if (number == 0)
            {
                mSign = 0;
                mBiasedExponent = 0;
            }
            else
            {
                mSign = 1;
                mBiasedExponent = BitHacks::GetTrailingBit(number);
                mUInteger = number;
            }
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                IsValid(),
                "Invalid BSNumber.");
#endif
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(number);
#endif
        }

        BSNumber(int64_t number)
            :
            mSign(0),
            mBiasedExponent(0),
            mUInteger{}
        {
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(number);
#endif
            if (number == 0)
            {
                mSign = 0;
                mBiasedExponent = 0;
            }
            else
            {
                if (number < 0)
                {
                    mSign = -1;
                    number = -number;
                }
                else
                {
                    mSign = 1;
                }

                uint64_t const unumber = static_cast<uint64_t>(number);
                mBiasedExponent = BitHacks::GetTrailingBit(unumber);
                mUInteger = unumber;
            }
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                IsValid(),
                "Invalid BSNumber.");
#endif
        }

        BSNumber(uint64_t number)
            :
            mSign(0),
            mBiasedExponent(0),
            mUInteger{}
        {
            if (number == 0)
            {
                mSign = 0;
                mBiasedExponent = 0;
            }
            else
            {
                mSign = 1;
                mBiasedExponent = BitHacks::GetTrailingBit(number);
                mUInteger = number;
            }
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                IsValid(),
                "Invalid BSNumber.");
#endif
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(number);
#endif
        }

        // The number must be of the form "x" or "+x" or "-x", where x is a
        // nonnegative integer with nonzero leading digit.
        BSNumber(std::string const& number)
            :
            mSign(0),
            mBiasedExponent(0),
            mUInteger{}
        {
            GTL_ARGUMENT_ASSERT(
                number.size() > 0,
                "Number must be specified.");

            // Get the leading '+' or '-' if it exists.
            std::string intNumber{};
            int32_t sign{};
            if (number[0] == '+')
            {
                intNumber = number.substr(1);
                sign = +1;
                GTL_ARGUMENT_ASSERT(
                    intNumber.size() > 1,
                    "Size must be larger than 1.");
            }
            else if (number[0] == '-')
            {
                intNumber = number.substr(1);
                sign = -1;
                GTL_ARGUMENT_ASSERT(
                    intNumber.size() > 1,
                    "Size must be larger than 1.");
            }
            else
            {
                intNumber = number;
                sign = +1;
            }

            *this = ConvertToInteger(intNumber);
            mSign = sign;
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                IsValid(),
                "Invalid BSNumber.");
#endif
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(*this);
#endif
        }

        BSNumber(char const* number)
            :
            BSNumber(std::string(number))
        {
        }

        ~BSNumber() = default;

        // Copy semantics.
        BSNumber(BSNumber const& other)
            :
            mSign(0),
            mBiasedExponent(0),
            mUInteger{}
        {
            *this = other;
        }

        BSNumber& operator=(BSNumber const& other)
        {
            mSign = other.mSign;
            mBiasedExponent = other.mBiasedExponent;
            mUInteger = other.mUInteger;
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = other.mValue;
#endif
            return *this;
        }

        // Move semantics.
        BSNumber(BSNumber&& other) noexcept
            :
            mSign(0),
            mBiasedExponent(0),
            mUInteger{}
        {
            *this = std::move(other);
        }

        BSNumber& operator=(BSNumber&& other) noexcept
        {
            mSign = other.mSign;
            mBiasedExponent = other.mBiasedExponent;
            mUInteger = std::move(other.mUInteger);
            other.mSign = 0;
            other.mBiasedExponent = 0;
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = other.mValue;
            other.mValue = 0.0;
#endif
            return *this;
        }

        // Implicit conversions. These always use the default rounding mode,
        // round-to-nearest-ties-to-even.
        inline operator float() const
        {
            return ConvertTo<IEEEBinary32>();
        }

        inline operator double() const
        {
            return ConvertTo<IEEEBinary64>();
        }

        // Member access.
        inline void SetSign(int32_t sign)
        {
            mSign = sign;
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(*this);
#endif
        }

        inline int32_t GetSign() const
        {
            return mSign;
        }

        inline void SetBiasedExponent(int32_t biasedExponent)
        {
            mBiasedExponent = biasedExponent;
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(*this);
#endif
        }

        inline int32_t GetBiasedExponent() const
        {
            return mBiasedExponent;
        }

        inline void SetExponent(int32_t exponent)
        {
            mBiasedExponent = exponent - static_cast<int32_t>(mUInteger.GetNumBits()) + 1;
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(*this);
#endif
        }

        inline int32_t GetExponent() const
        {
            return mBiasedExponent + static_cast<int32_t>(mUInteger.GetNumBits()) - 1;
        }

        inline UInteger const& GetUInteger() const
        {
            return mUInteger;
        }

        inline UInteger& GetUInteger()
        {
            return mUInteger;
        }

        // Comparisons.
        bool operator==(BSNumber const& other) const
        {
            return (mSign == other.mSign ? EqualIgnoreSign(*this, other) : false);
        }

        bool operator!=(BSNumber const& other) const
        {
            return !operator==(other);
        }

        bool operator< (BSNumber const& other) const
        {
            if (mSign > 0)
            {
                if (other.mSign <= 0)
                {
                    return false;
                }

                // Both numbers are positive.
                return LessThanIgnoreSign(*this, other);
            }
            else if (mSign < 0)
            {
                if (other.mSign >= 0)
                {
                    return true;
                }

                // Both numbers are negative.
                return LessThanIgnoreSign(other, *this);
            }
            else
            {
                return other.mSign > 0;
            }
        }

        bool operator<=(BSNumber const& other) const
        {
            return !(other < *this);
        }

        bool operator> (BSNumber const& other) const
        {
            return other < *this;
        }

        bool operator>=(BSNumber const& other) const
        {
            return !operator<(other);
        }

        // Unary operations.
        BSNumber operator+() const
        {
            return *this;
        }

        BSNumber operator-() const
        {
            BSNumber result = *this;
            result.mSign = -result.mSign;
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            result.mValue = -result.mValue;
#endif
            return result;
        }

        // Arithmetic.
        BSNumber operator+(BSNumber const& number) const
        {
            BSNumber result{};
            Add(*this, number, result);
            return result;
        }

        BSNumber operator-(BSNumber const& number) const
        {
            BSNumber result{};
            Sub(*this, number, result);
            return result;
        }

        BSNumber operator*(BSNumber const& number) const
        {
            BSNumber result{};
            Mul(*this, number, result);
            return result;
        }

        BSNumber& operator+=(BSNumber const& number)
        {
            *this = operator+(number);
            return *this;
        }

        BSNumber& operator-=(BSNumber const& number)
        {
            *this = operator-(number);
            return *this;
        }

        BSNumber& operator*=(BSNumber const& number)
        {
            *this = operator*(number);
            return *this;
        }

        // In-place computations for applications that wish to minimize the
        // use of the program stack.
        static void Add(BSNumber const& n0, BSNumber const& n1, BSNumber& result)
        {
            if (n0.mSign == 0)
            {
                result = n1;
                return;
            }

            if (n1.mSign == 0)
            {
                result = n0;
                return;
            }

            if (n0.mSign > 0)
            {
                if (n1.mSign > 0)
                {
                    // n0 + n1 = |n0| + |n1|
                    AddIgnoreSign(n0, n1, +1, result);
                    return;
                }
                else // n1.mSign < 0
                {
                    if (!EqualIgnoreSign(n0, n1))
                    {
                        if (LessThanIgnoreSign(n1, n0))
                        {
                            // n0 + n1 = |n0| - |n1| > 0
                            SubIgnoreSign(n0, n1, +1, result);
                            return;
                        }
                        else
                        {
                            // n0 + n1 = -(|n1| - |n0|) < 0
                            SubIgnoreSign(n1, n0, -1, result);
                            return;
                        }
                    }
                    // else n0 + n1 = 0
                }
            }
            else // n0.mSign < 0
            {
                if (n1.mSign < 0)
                {
                    // n0 + n1 = -(|n0| + |n1|)
                    AddIgnoreSign(n0, n1, -1, result);
                    return;
                }
                else // n1.mSign > 0
                {
                    if (!EqualIgnoreSign(n0, n1))
                    {
                        if (LessThanIgnoreSign(n1, n0))
                        {
                            // n0 + n1 = -(|n0| - |n1|) < 0
                            SubIgnoreSign(n0, n1, -1, result);
                            return;
                        }
                        else
                        {
                            // n0 + n1 = |n1| - |n0| > 0
                            SubIgnoreSign(n1, n0, +1, result);
                            return;
                        }
                    }
                    // else n0 + n1 = 0
                }
            }

            result = BSNumber();  // = 0
        }

        static void Sub(BSNumber const& n0, BSNumber const& n1, BSNumber& result)
        {
            if (n0.mSign == 0)
            {
                // result = -n1, avoid creation by operator-()
                result = n1;
                result.SetSign(-n1.GetSign());
                return;
            }

            if (n1.mSign == 0)
            {
                result = n0;
                return;
            }

            if (n0.mSign > 0)
            {
                if (n1.mSign < 0)
                {
                    // n0 - n1 = |n0| + |n1|
                    AddIgnoreSign(n0, n1, +1, result);
                    return;
                }
                else // n1.mSign > 0
                {
                    if (!EqualIgnoreSign(n0, n1))
                    {
                        if (LessThanIgnoreSign(n1, n0))
                        {
                            // n0 - n1 = |n0| - |n1| > 0
                            SubIgnoreSign(n0, n1, +1, result);
                            return;
                        }
                        else
                        {
                            // n0 - n1 = -(|n1| - |n0|) < 0
                            SubIgnoreSign(n1, n0, -1, result);
                            return;
                        }
                    }
                    // else n0 - n1 = 0
                }
            }
            else // n0.mSign < 0
            {
                if (n1.mSign > 0)
                {
                    // n0 - n1 = -(|n0| + |n1|)
                    AddIgnoreSign(n0, n1, -1, result);
                    return;
                }
                else // n1.mSign < 0
                {
                    if (!EqualIgnoreSign(n0, n1))
                    {
                        if (LessThanIgnoreSign(n1, n0))
                        {
                            // n0 - n1 = -(|n0| - |n1|) < 0
                            SubIgnoreSign(n0, n1, -1, result);
                            return;
                        }
                        else
                        {
                            // n0 - n1 = |n1| - |n0| > 0
                            SubIgnoreSign(n1, n0, +1, result);
                            return;
                        }
                    }
                    // else n0 - n1 = 0
                }
            }

            result = BSNumber();  // = 0
        }

        static void Mul(BSNumber const& n0, BSNumber const& n1, BSNumber& result)
        {
            int32_t sign = n0.mSign * n1.mSign;
            if (sign != 0)
            {
                result.mSign = sign;
                result.mBiasedExponent = n0.mBiasedExponent + n1.mBiasedExponent;

                if (n0.mUInteger.GetNumBits() != 1)
                {
                    if (n1.mUInteger.GetNumBits() != 1)
                    {
                        ALU::Mul(n0.mUInteger, n1.mUInteger, result.mUInteger);
                    }
                    else
                    {
                        result.mUInteger = n0.mUInteger;
                    }
                }
                else
                {
                    if (n1.mUInteger.GetNumBits() != 1)
                    {
                        result.mUInteger = n1.mUInteger;
                    }
                    else
                    {
                        result.mUInteger.SetNumBits(1);
                        result.mUInteger.GetBits()[0] = 1;
                    }
                }

#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
                result.mValue = static_cast<double>(result);
#endif
            }
            else
            {
                result = BSNumber();  // = 0
            }
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                result.IsValid(),
                "Invalid BSNumber.");
#endif
        }

        // Streaming support. The stream objects should be created using
        // std::ios::binary. The return value is 'true' iff the operation
        // is successful.
        bool Write(std::ostream& output) const
        {
            if (output.write((char const*)&mSign, sizeof(mSign)).bad())
            {
                return false;
            }

            if (output.write((char const*)&mBiasedExponent, sizeof(mBiasedExponent)).bad())
            {
                return false;
            }

            return mUInteger.Write(output);
        }

        bool Read(std::istream& input)
        {
            if (input.read((char*)&mSign, sizeof(mSign)).bad())
            {
                return false;
            }

            if (input.read((char*)&mBiasedExponent, sizeof(mBiasedExponent)).bad())
            {
                return false;
            }

            return mUInteger.Read(input);
        }

#if defined(GTL_THROW_ON_INVALID_BSNUMBER)
        bool IsValid() const
        {
            size_t const numBits = mUInteger.GetNumBits();
            size_t const numBlocks = mUInteger.GetNumBlocks();
            if (mSign != 0)
            {
                auto const& bits = mUInteger.GetBits();
                return numBits > 0 && numBlocks > 0 && bits[numBlocks - 1] != 0u
                    && (bits[0] & 0x00000001u) == 1u;
            }
            else
            {
                return numBits == 0 && numBlocks == 0 && mBiasedExponent == 0;
            }
        }
#endif

#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
        // List this first so that it shows up first in the debugger watch
        // window.
        double mValue;
#endif

    private:
        // BSRational depends on the design of BSNumber. Allow it to have
        // full access to the implementation.
        friend class BSRational<UInteger>;

        // Convenient type definitions for the arithmetic logic unit.
        using ALU = UIntegerALU32<UInteger>;

        // Helpers for operator==, operator<, operator+, operator-.
        static bool EqualIgnoreSign(BSNumber const& n0, BSNumber const& n1)
        {
            return n0.mBiasedExponent == n1.mBiasedExponent && n0.mUInteger == n1.mUInteger;
        }

        static bool LessThanIgnoreSign(BSNumber const& n0, BSNumber const& n1)
        {
            int32_t e0 = n0.GetExponent(), e1 = n1.GetExponent();
            if (e0 < e1)
            {
                return true;
            }
            if (e0 > e1)
            {
                return false;
            }
            return n0.mUInteger < n1.mUInteger;
        }

        // Add two positive numbers.
        static void AddIgnoreSign(BSNumber const& n0, BSNumber const& n1,
            int32_t resultSign, BSNumber& result)
        {
            UInteger temp{};

            int32_t shift = n0.mBiasedExponent - n1.mBiasedExponent;
            if (shift > 0)
            {
                ALU::ShiftLeft(n0.mUInteger, shift, temp);
                ALU::Add(temp, n1.mUInteger, result.mUInteger);
                result.mBiasedExponent = n1.mBiasedExponent;
            }
            else if (shift < 0)
            {
                ALU::ShiftLeft(n1.mUInteger, -shift, temp);
                ALU::Add(n0.mUInteger, temp, result.mUInteger);
                result.mBiasedExponent = n0.mBiasedExponent;
            }
            else
            {
                ALU::Add(n0.mUInteger, n1.mUInteger, temp);
                shift = ALU::ShiftRightToOdd(temp, result.mUInteger);
                result.mBiasedExponent = n0.mBiasedExponent + shift;
            }

            result.mSign = resultSign;
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                result.IsValid(),
                "Invalid BSNumber.");
#endif
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            result.mValue = static_cast<double>(result);
#endif
        }

        // Subtract two positive numbers where n0 > n1.
        static void SubIgnoreSign(BSNumber const& n0, BSNumber const& n1,
            int32_t resultSign, BSNumber& result)
        {
            UInteger temp{};

            int32_t shift = n0.mBiasedExponent - n1.mBiasedExponent;
            if (shift > 0)
            {
                ALU::ShiftLeft(n0.mUInteger, shift, temp);
                ALU::Sub(temp, n1.mUInteger, result.mUInteger);
                result.mBiasedExponent = n1.mBiasedExponent;
            }
            else if (shift < 0)
            {
                ALU::ShiftLeft(n1.mUInteger, -shift, temp);
                ALU::Sub(n0.mUInteger, temp, result.mUInteger);
                result.mBiasedExponent = n0.mBiasedExponent;
            }
            else
            {
                ALU::Sub(n0.mUInteger, n1.mUInteger, temp);
                shift = ALU::ShiftRightToOdd(temp, result.mUInteger);
                result.mBiasedExponent = n0.mBiasedExponent + shift;
            }

            result.mSign = resultSign;
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                result.IsValid(),
                "Invalid BSNumber.");
#endif
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            result.mValue = static_cast<double>(result);
#endif
        }

        // Helper for converting a string to a BSNumber. The string must be
        // valid for a nonnegative integer without a leading '+' sign.
        static BSNumber ConvertToInteger(std::string const& number)
        {
            int32_t digit = static_cast<int32_t>(number.back()) - static_cast<int32_t>('0');
            BSNumber x(digit);
            if (number.size() > 1)
            {
                GTL_ARGUMENT_ASSERT(
                    number.find_first_of("123456789") == 0,
                    "Incorrect number format.");

                GTL_ARGUMENT_ASSERT(
                    number.find_first_not_of("0123456789") == std::string::npos,
                    "Incorrect number format.");

                BSNumber ten(10), pow10(10);
                for (size_t i = 1, j = number.size() - 2; i < number.size(); ++i, --j)
                {
                    digit = static_cast<int32_t>(number[j]) - static_cast<int32_t>('0');
                    if (digit > 0)
                    {
                        x += BSNumber(digit) * pow10;
                    }
                    pow10 *= ten;
                }
            }
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
            GTL_RUNTIME_ASSERT(
                x.IsValid(),
                "Invalid BSNumber.");
#endif
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            x.mValue = static_cast<double>(x);
#endif
            return x;
        }

        // Support for conversions from floating-point numbers to BSNumber.
        template <typename IEEE>
        void ConvertFrom(typename IEEE::FloatType number)
        {
            IEEE x(number);

            // Extract sign s, biased exponent e, and trailing significand t.
            typename IEEE::UIntType s = x.GetSign();
            typename IEEE::UIntType e = x.GetBiased();
            typename IEEE::UIntType t = x.GetTrailing();

            if (e == 0)
            {
                if (t == 0)  // zeros
                {
                    // x = (-1)^s * 0
                    mSign = 0;
                    mBiasedExponent = 0;
                }
                else  // subnormal numbers
                {
                    // x = (-1)^s * 0.t * 2^{1-EXPONENT_BIAS}
                    int32_t last = static_cast<int32_t>(BitHacks::GetTrailingBit(t));
                    int32_t diff = IEEE::NUM_TRAILING_BITS - last;
                    mSign = (s > 0 ? -1 : 1);
                    mBiasedExponent = IEEE::MIN_SUB_EXPONENT - diff;
                    mUInteger = (t >> last);
                }
            }
            else if (e < IEEE::MAX_BIASED_EXPONENT)  // normal numbers
            {
                // x = (-1)^s * 1.t * 2^{e-EXPONENT_BIAS}
                if (t > 0)
                {
                    int32_t last = static_cast<int32_t>(BitHacks::GetTrailingBit(t));
                    int32_t diff = IEEE::NUM_TRAILING_BITS - last;
                    mSign = (s > 0 ? -1 : 1);
                    mBiasedExponent = static_cast<int32_t>(e) - IEEE::EXPONENT_BIAS - diff;
                    mUInteger = ((t | IEEE::SUP_TRAILING) >> last);
                }
                else
                {
                    mSign = (s > 0 ? -1 : 1);
                    mBiasedExponent = static_cast<int32_t>(e) - IEEE::EXPONENT_BIAS;
                    mUInteger = static_cast<typename IEEE::UIntType>(1);
                }
            }
            else  // e == MAX_BIASED_EXPONENT, special numbers
            {
                if (t == 0)  // infinities
                {
                    // x = (-1)^s * infinity
#if defined(GTL_THROW_ON_CONVERT_FROM_INFINITY_OR_NAN)
                    GTL_RUNTIME_ERROR(
                        "BSNumber does not have a representation for infinities.");
#else
                    // Return (-1)^s*2^{1+EXPONENT_BIAS} for a graceful exit.
                    mSign = (s > 0 ? -1 : 1);
                    mBiasedExponent = 1 + IEEE::EXPONENT_BIAS;
                    mUInteger = static_cast<typename IEEE::UIntType>(1);
#endif
                }
                else  // not-a-number (NaN)
                {
#if defined(GTL_THROW_ON_CONVERT_FROM_INFINITY_OR_NAN)
                    GTL_RUNTIME_ERROR(
                        "BSNumber does not have a representation for NaNs.");
#else
                    // Return 0 for a graceful exit.
                    mSign = 0;
                    mBiasedExponent = 0;
#endif
                }
            }
        }

        // Support for conversions from BSNumber to floating-point numbers.
        template <typename IEEE>
        typename IEEE::FloatType ConvertTo() const
        {
            if (mSign == 0)
            {
                return static_cast<typename IEEE::FloatType>(0);
            }

            // The conversions use round-to-nearest-ties-to-even semantics.
            typename IEEE::UIntType t{};
            int32_t e{};
            int32_t exponent = GetExponent();
            if (exponent < IEEE::MIN_EXPONENT)
            {
                // When exponent = MIN_EXPONENT - 1, the second term of the OR
                // is equivalent to x = 1.0*2^{MIN_EXPONENT-1} and therefore
                // the uinteger part is 1.
                if (exponent < IEEE::MIN_EXPONENT - 1 || mUInteger.GetNumBits() == 1)
                {
                    // Round to zero.
                    e = 0;
                    t = 0;
                }
                else
                {
                    // Round to min subnormal.
                    e = 0;
                    t = 1;
                }
            }
            else if (exponent < IEEE::MIN_SUB_EXPONENT)
            {
                t = GetTrailing<IEEE>(0, IEEE::MIN_SUB_EXPONENT - exponent - 1);
                if (t & IEEE::SUP_TRAILING)
                {
                    // Leading NUM_SIGNIFICAND_BITS bits were all 1, so round
                    // to min normal.
                    e = 1;
                    t = 0;
                }
                else
                {
                    e = 0;
                }
            }
            else if (exponent <= IEEE::EXPONENT_BIAS)
            {
                e = exponent + IEEE::EXPONENT_BIAS;
                t = GetTrailing<IEEE>(1, 0);
                if (t & (IEEE::SUP_TRAILING << 1))
                {
                    // Carry-out occurred, so increase exponent by 1 and shift
                    // right to compensate.
                    ++e;
                    t >>= 1;
                }
                // Eliminate the leading 1 (implied for normals).
                t &= ~IEEE::SUP_TRAILING;
            }
            else
            {
                // Set to infinity.
                e = IEEE::MAX_BIASED_EXPONENT;
                t = 0;
            }

            IEEE x((mSign < 0 ? 1 : 0), static_cast<typename IEEE::UIntType>(e), t);
            return x.number;
        }

        template <typename IEEE>
        typename IEEE::UIntType GetTrailing(int32_t normal, int32_t sigma) const
        {
            int32_t const numRequested = IEEE::NUM_SIGNIFICAND_BITS + normal;

            // We need numRequested bits to determine rounding direction.
            // These are stored in the high-order bits of 'prefix'.
            uint64_t const prefix = GetPrefix(numRequested);

            // The first bit index after the implied binary point for
            // rounding.
            int32_t const diff = numRequested - sigma;
            int32_t const roundBitIndex = 64 - diff;

            // Determine rounding value using round-to-nearest-ties-to-even.
            uint64_t const mask = (1ull << roundBitIndex);
            uint64_t round{};
            if (prefix & mask)
            {
                // The first bit of the remainder is 1.
                if (static_cast<int32_t>(mUInteger.GetNumBits()) == diff)
                {
                    // The first bit of the remainder is the lowest-order bit
                    // of mBits[0]. Apply the ties-to-even rule.
                    if (prefix & (mask << 1))
                    {
                        // The last bit of the trailing significand is odd,
                        // so round up.
                        round = 1;
                    }
                    else
                    {
                        // The last bit of the trailing significand is even,
                        // so round down.
                        round = 0;
                    }
                }
                else
                {
                    // The first bit of the remainder is not the lowest-order
                    // bit of mBits[0]. The remainder as a fraction is larger
                    // than 1/2, so round up.
                    round = 1;
                }
            }
            else
            {
                // The first bit of the remainder is 0, so round down.
                round = 0;
            }

            // Get the unrounded trailing significand.
            uint64_t trailing = (prefix >> (roundBitIndex + 1));

            // Apply the rounding.
            trailing += round;
            return static_cast<typename IEEE::UIntType>(trailing);
        }

        // Get a block of numRequested bits starting with the leading 1-bit of
        // 'this'. The returned number has the prefix stored in the high-order
        // bits. Additional bits are copied and used by the caller for rounding.
        // This function supports conversions from 'float' or 'double'. The
        // input 'numRequested' is 23 for float-subnormal, 24 for float-normal,
        // 52 for double-subnormal or 53 for double-normal.
        uint64_t GetPrefix(int32_t numRequested) const
        {
            // Get information about the bits of the BSNumber.
            auto const& bits = mUInteger.GetBits();
            int32_t const numBits = static_cast<int32_t>(mUInteger.GetNumBits());
            int32_t const leading = (numBits - 1) % 32;
            int32_t const numBlockBits = leading + 1;
            size_t current = static_cast<size_t>(mUInteger.GetNumBlocks() - 1);

            // Copy the most significant block of bits to 'prefix' and shift
            // the leading 1-bit of 'bits' to bit 63. After these operations,
            // 'numBlockBits' of the number of requested bits have been
            // consumed.
            uint64_t prefix = static_cast<uint64_t>(bits[current]);
            int32_t targetIndex = 63;
            uint32_t lshift = static_cast<uint32_t>(targetIndex - leading);
            prefix <<= lshift;
            numRequested -= numBlockBits;
            targetIndex -= numBlockBits;

            if (numRequested > 0 && current > 0)
            {
                // More bits are available. Copy the entire 32-bit into the
                // appropriate location in 'prefix'. For 'float', the entire
                // budget is consumed.
                uint64_t nextBlock = static_cast<uint64_t>(bits[--current]);
                lshift = static_cast<uint32_t>(targetIndex - 31);
                nextBlock <<= lshift;
                prefix |= nextBlock;
                numRequested -= 32;
                targetIndex -= 32;

                if (numRequested > 0 && current > 0)
                {
                    // To reach this code block, the type must be 'double'.
                    // More bits are available. 'targetIndex' is guaranteed
                    // to be smaller than 31 because it started at 63, at
                    // least 1 was subtracted from it for the copy of the
                    // first and 32 was subtracted 32 from it for the copy of
                    // the second block. This implies 'rshift' is positive.
                    nextBlock = static_cast<uint64_t>(bits[--current]);
                    uint32_t rshift = static_cast<uint32_t>(31 - targetIndex);
                    nextBlock >>= rshift;
                    prefix |= nextBlock;
                }
            }

            return prefix;
        }

    private:
        // The number 0 is represented by: mSign = 0, mBiasedExponent = 0 and
        // mUInteger = 0. For nonzero numbers, mSign != 0 and mUInteger > 0.
        int32_t mSign;
        int32_t mBiasedExponent;
        UInteger mUInteger;

        friend class UnitTestBSNumber;
    };

    // Support for conversion from an arbitrary-precision BSNumber to a
    // user-specified-precision BSNumber. The rounding mode is one of the
    // following.
    enum class APRoundingMode
    {
        TO_NEAREST,     // round to nearest ties to even
        DOWNWARD,       // round towards negative infinity
        TOWARD_ZERO,    // round towards zero
        UPWARD          // round towards positive infinity
    };

    template <typename UInteger>
    void Convert(BSNumber<UInteger> const& input, size_t precision,
        APRoundingMode roundingMode, BSNumber<UInteger>& output)
    {
        // The number zero needs no conversion.
        if (input.GetSign() == 0)
        {
            output = BSNumber<UInteger>(0);
            return;
        }

        GTL_ARGUMENT_ASSERT(
            precision > 0,
            "Precision must be positive.");

        size_t const maxNumBlocks = UInteger::GetMaxNumBlocks();
        size_t const numPrecBlocks = (precision + 31) / 32;
        GTL_ARGUMENT_ASSERT(
            numPrecBlocks < maxNumBlocks,
            "The maximum precision has been exceeded.");

        // Let p be the precision and n+1 be the number of bits of the input.
        // If p >= n+1, the required precision is already satisfied by the
        // input.
        if (input.GetUInteger().GetNumBits() <= precision)
        {
            output = input;
            return;
        }

        // At this point, the requested number of bits is smaller than the
        // number of bits in the input. Extract the specified number of bits
        // (precision) from the input.
        size_t const np1mp = static_cast<size_t>(
            input.GetUInteger().GetNumBits() - precision);
        UInteger& outW = output.GetUInteger();
        outW.SetNumBits(precision);
        outW.SetAllBitsToZero();
        size_t const outNumBlocks = outW.GetNumBlocks();
        size_t const precisionM1 = static_cast<size_t>(precision - 1);
        uint32_t const outLeading = static_cast<uint32_t>(precisionM1 % 32);
        uint32_t outMask = (1u << outLeading);
        auto& outBits = outW.GetBits();
        size_t outCurrent = static_cast<size_t>(outNumBlocks - 1);

        UInteger const& inW = input.GetUInteger();
        size_t const inNumBlocks = inW.GetNumBlocks();
        uint32_t const inLeading = static_cast<uint32_t>((inW.GetNumBits() - 1) % 32);
        uint32_t inMask = (1u << inLeading);
        auto const& inBits = inW.GetBits();
        size_t inCurrent = static_cast<size_t>(inNumBlocks - 1);

        int32_t lastBit = -1;
        for (int64_t i = static_cast<int64_t>(precisionM1); i >= 0; --i)
        {
            if (inBits[inCurrent] & inMask)
            {
                outBits[outCurrent] |= outMask;
                lastBit = 1;
            }
            else
            {
                lastBit = 0;
            }

            if (inMask == 0x00000001u)
            {
                --inCurrent;
                inMask = 0x80000000u;
            }
            else
            {
                inMask >>= 1;
            }

            if (outMask == 0x00000001u)
            {
                --outCurrent;
                outMask = 0x80000000u;
            }
            else
            {
                outMask >>= 1;
            }
        }

        // At this point as a sequence of bits, the remainder is
        // r = u_{n-p} ... u_0. Round the the extracted bits based on the
        // specified rounding mode.
        int32_t sign = input.GetSign();
        int32_t outExponent = input.GetExponent();
        if (roundingMode == APRoundingMode::TO_NEAREST)
        {
            // Determine whether u_{n-p} is positive.
            if (((inBits[inCurrent] & inMask) != 0u) && (np1mp > 1 || lastBit == 1))
            {
                // round up
                outExponent += UIntegerALU32<UInteger>::RoundUp(outW);
            }
            // else round down, equivalent to truncating the r bits
        }
        else if (roundingMode == APRoundingMode::UPWARD)
        {
            // The remainder r must be positive because n-p >= 0 and u_0 = 1.
            if (sign > 0)
            {
                // round up
                outExponent += UIntegerALU32<UInteger>::RoundUp(outW);
            }
            // else round down, equivalent to truncating the r bits
        }
        else if (roundingMode == APRoundingMode::DOWNWARD)
        {
            // The remainder r must be positive because n-p >= 0 and u_0 = 1.
            if (sign < 0)
            {
                // Round down. This is the round-up operation applied to
                // w, but the final sign is negative which amounts to
                // rounding down.
                outExponent += UIntegerALU32<UInteger>::RoundUp(outW);
            }
            // else round down, equivalent to truncating the r bits
        }
        else if (roundingMode != APRoundingMode::TOWARD_ZERO)
        {
            // Currently, no additional implementation-dependent modes
            // are supported for rounding.
            GTL_RUNTIME_ERROR(
                "Implementation-dependent rounding mode not supported.");
        }
        // else roundingMode == APRoundingMode::TOWARDZERO. Truncate the r
        // bits, which requires no additional work.

        // Shift the bits if necessary to obtain the invariant that BSNumber
        // objects have bit patterns that are odd integers.
        if (outW.GetNumBits() > 0 && (outW.GetBits()[0] & 1u) == 0)
        {
            UInteger temp = outW;
            outExponent += UIntegerALU32<UInteger>::ShiftRightToOdd(temp, outW);
        }

        // Do not use SetExponent(outExponent) at this step. The number of
        // requested bits is 'precision' but outW.GetNumBits() will be
        // different when round-up occurs, and SetExponent accesses
        // outW.GetNumBits().
        output.SetSign(sign);
        output.SetBiasedExponent(outExponent - static_cast<int32_t>(precision - 1));
#if defined (GTL_THROW_ON_INVALID_BSNUMBER)
        GTL_RUNTIME_ASSERT(
            output.IsValid(),
            "Invalid BSNumber.");
#endif
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
        output.mValue = static_cast<double>(output);
#endif
    }
}

namespace std
{
    // TODO: Allow for implementations of the math functions in which a
    // specified precision is used when computing the result.

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> abs(gtl::BSNumber<UInteger> const& x)
    {
        return (x.GetSign() >= 0 ? x : -x);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> acos(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::acos(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> acosh(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::acosh(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> asin(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::asin(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> asinh(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::asinh(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> atan(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::atan(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> atanh(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::atanh(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> atan2(gtl::BSNumber<UInteger> const& y, gtl::BSNumber<UInteger> const& x)
    {
        double dy = static_cast<double>(y);
        double dx = static_cast<double>(x);
        double result = std::atan2(dy, dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    // TODO: Implement an exact ceil function.
    template <typename UInteger>
    inline gtl::BSNumber<UInteger> ceil(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::ceil(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> cos(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::cos(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> cosh(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::cosh(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> exp(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::exp(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> exp2(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::exp2(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> fabs(gtl::BSNumber<UInteger> const& x)
    {
        return (x.GetSign() >= 0 ? x : -x);
    }

    // TODO: Implement an exact floor function.
    template <typename UInteger>
    inline gtl::BSNumber<UInteger> floor(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::floor(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> fmod(gtl::BSNumber<UInteger> const& x, gtl::BSNumber<UInteger> const& y)
    {
        double dx = static_cast<double>(x);
        double dy = static_cast<double>(y);
        double result = std::fmod(dx, dy);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> frexp(gtl::BSNumber<UInteger> const& x, int32_t* exponent)
    {
        if (x.GetSign() != 0)
        {
            gtl::BSNumber<UInteger> result = x;
            *exponent = result.GetExponent() + 1;
            result.SetExponent(-1);
            return result;
        }
        else
        {
            *exponent = 0;
            return gtl::BSNumber<UInteger>(0);
        }
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> ldexp(gtl::BSNumber<UInteger> const& x, int32_t exponent)
    {
        gtl::BSNumber<UInteger> result = x;
        result.SetBiasedExponent(result.GetBiasedExponent() + exponent);
        return result;
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> log(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::log(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> log2(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::log2(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> log10(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::log10(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> pow(gtl::BSNumber<UInteger> const& x, gtl::BSNumber<UInteger> const& y)
    {
        double dx = static_cast<double>(x);
        double dy = static_cast<double>(y);
        double result = std::pow(dx, dy);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> remainder(gtl::BSNumber<UInteger> const& x, gtl::BSNumber<UInteger> const& y)
    {
        double dx = static_cast<double>(x);
        double dy = static_cast<double>(y);
        double result = std::remainder(dx, dy);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> sin(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::sin(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> sinh(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::sinh(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> sqrt(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::sqrt(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> tan(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::tan(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline gtl::BSNumber<UInteger> tanh(gtl::BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = std::tanh(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }
}

namespace gtl
{
    template <typename UInteger>
    inline BSNumber<UInteger> atandivpi(BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = atandivpi(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> atan2divpi(BSNumber<UInteger> const& y, BSNumber<UInteger> const& x)
    {
        double dy = static_cast<double>(y);
        double dx = static_cast<double>(x);
        double result = atan2divpi(dy, dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> clamp(BSNumber<UInteger> const& x, BSNumber<UInteger> const& xmin, BSNumber<UInteger> const& xmax)
    {
        return (x <= xmin ? xmin : (x >= xmax ? xmax : x));
    }

    template <typename UInteger>
    inline BSNumber<UInteger> cospi(BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = cospi(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> exp10(BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = exp10(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> invsqrt(BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = invsqrt(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline int32_t isign(BSNumber<UInteger> const& x)
    {
        return (x.GetSign() > 0 ? 1 : (x.GetSign() < 0 ? -1 : 0));
    }

    template <typename UInteger>
    inline BSNumber<UInteger> saturate(BSNumber<UInteger> const& x)
    {
        if (x.GetSign() <= 0)
        {
            return BSNumber<UInteger>(0);
        }
        else if (x.GetExponent() >= 0)
        {
            return BSNumber<UInteger>(1);
        }
        else
        {
            return x;
        }
    }

    template <typename UInteger>
    inline BSNumber<UInteger> sign(BSNumber<UInteger> const& x)
    {
        if (x.GetSign() > 0)
        {
            return BSNumber<UInteger>(1);
        }
        else if (x.GetSign() < 0)
        {
            return BSNumber<UInteger>(-1);
        }
        else
        {
            return BSNumber<UInteger>(0);
        }
    }

    template <typename UInteger>
    inline BSNumber<UInteger> sinpi(BSNumber<UInteger> const& x)
    {
        double dx = static_cast<double>(x);
        double result = sinpi(dx);
        return static_cast<gtl::BSNumber<UInteger>>(result);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> sqr(BSNumber<UInteger> const& x)
    {
        return x * x;
    }

    // See the comments in TypeTraits.h about trait is_arbitrary_precision.
    template <typename UInteger>
    struct _is_arbitrary_precision_internal<BSNumber<UInteger>> : std::true_type {};
}
