// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <GTL/Utility/Exceptions.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

// Template metaprogramming to support Vector classes and operations.
namespace gtl
{
    // The primary template for Vector classes.
    template <typename T, size_t...> class Vector;

    template <typename T, size_t Dimension>
    struct VectorTraits
    {
        using value_type = T;
        static size_t constexpr N = Dimension;
    };
}

// Implementation for vectors whose sizes are known at compile time.
namespace gtl
{
    template <typename T, size_t N>
    class Vector<T, N> : public VectorTraits<T, N>
    {
    public:
        // Construction and destruction.

        // All elements of the vector are initialized to zero.
        Vector()
            :
            mContainer{}
        {
            static_assert(
                N > 0,
                "The dimension must be positive.");

            fill(C_<T>(0));
        }

        // Create a vector from an initializer list with N elements.
        Vector(std::initializer_list<T> const& elements)
            :
            mContainer{}
        {
            static_assert(
                N > 0,
                "The dimension must be positive.");

            GTL_ARGUMENT_ASSERT(
                elements.size() == N,
                "Invalid length for std::initializer_list.");

            std::copy(elements.begin(), elements.end(), mContainer.begin());
        }

        // Create a vector from a std::array with N elements.
        Vector(std::array<T, N> const& elements)
            :
            mContainer(elements)
        {
            static_assert(
                N > 0,
                "The dimension must be positive.");
        }

        // Create a vector from a std::vector with N elements.
        Vector(std::vector<T> const& elements)
            :
            mContainer{}
        {
            static_assert(
                N > 0,
                "The dimension must be positive.");

            GTL_ARGUMENT_ASSERT(
                elements.size() == N,
                "Invalid length for std::vector.");

            std::copy(elements.begin(), elements.end(), mContainer.begin());
        }

        ~Vector() = default;

        // Copy semantics.
        Vector(Vector const& other)
            :
            mContainer{}
        {
            *this = other;
        }

        Vector& operator=(Vector const& other)
        {
            mContainer = other.mContainer;
            return *this;
        }

        // Move semantics.
        Vector(Vector&& other) noexcept
            :
            mContainer{}
        {
            *this = std::move(other);
        }

        Vector& operator=(Vector&& other) noexcept
        {
            mContainer = std::move(other.mContainer);
            return *this;
        }

        // Size and data access.
        inline size_t constexpr size() const noexcept
        {
            return N;
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

        // Set all elements to the specified value.
        inline void fill(T const& value)
        {
            mContainer.fill(value);
        }

        inline static Vector<T, N> Zero()
        {
            return Vector<T, N>{}; // zero
        }

    private:
        std::array<T, N> mContainer;

        friend class UnitTestVector;
    };
}

// Implementation for vectors whose sizes are known only at run time.
namespace gtl
{
    template <typename T>
    class Vector<T> : public VectorTraits<T, 0>
    {
    public:
        // Construction and destruction.

        // Create an empty vector or a non-empty vector whose elements are
        // initialized to zero.
        Vector(size_t numElements = 0)
            :
            mContainer(numElements, C_<T>(0))
        {
        }

        // Create a vector from an initializer list. An empty input list
        // causes the default constructor to be called, not this constructor.
        Vector(std::initializer_list<T> const& elements)
            :
            mContainer(elements.size())
        {
            std::copy(elements.begin(), elements.end(), mContainer.begin());
        }

        // Create a vector from a std::array.
        template <size_t N>
        Vector(std::array<T, N> const& elements)
            :
            mContainer(elements.size())
        {
            std::copy(elements.begin(), elements.end(), mContainer.begin());
        }

        // Create a vector from a std::vector.
        Vector(std::vector<T> const& elements)
            :
            mContainer(elements)
        {
        }

        // Resize the vector to support deferred construction.
        void resize(size_t numElements)
        {
            mContainer.resize(numElements);
        }

        ~Vector() = default;


        // Copy semantics.
        Vector(Vector const& other)
            :
            mContainer{}
        {
            *this = other;
        }

        Vector& operator=(Vector const& other)
        {
            mContainer = other.mContainer;
            return *this;
        }

        // Move semantics.
        Vector(Vector&& other) noexcept
            :
            mContainer{}
        {
            *this = std::move(other);
        }

        Vector& operator=(Vector&& other) noexcept
        {
            mContainer = std::move(other.mContainer);
            return *this;
        }

        // Size and data access.
        inline size_t const size() const noexcept
        {
            return mContainer.size();
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

        // Set all elements to the specified value.
        inline void fill(T const& value)
        {
            std::fill(mContainer.begin(), mContainer.end(), value);
        }

        inline static Vector<T> Zero(size_t numElements)
        {
            return Vector<T>(numElements);
        }

    private:
        std::vector<T> mContainer;

        friend class UnitTestVector;
    };
}

// Operations for Vector classes whose sizes are known at compile time.
namespace gtl
{
    // Allow sorting and comparing of objects.
    template <typename T, size_t N>
    bool operator==(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (v0[i] != v1[i])
            {
                return false;
            }
        }
        return true;
    }

    template <typename T, size_t N>
    bool operator!=(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        return !operator==(v0, v1);
    }

    template <typename T, size_t N>
    bool operator<(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (v0[i] < v1[i])
            {
                return true;
            }
            if (v0[i] > v1[i])
            {
                return false;
            }
        }
        return false;
    }

    template <typename T, size_t N>
    bool operator<=(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        return !operator<(v1, v0);
    }

    template <typename T, size_t N>
    bool operator>(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        return operator<(v1, v0);
    }

    template <typename T, size_t N>
    bool operator>=(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        return !operator<(v0, v1);
    }

    // Set all vector elements to zero.
    template <typename T, size_t N>
    void MakeZero(Vector<T, N>& v)
    {
        v.fill(C_<T>(0));
    }

    // Test whether the vector is the zero vector.
    template <typename T, size_t N>
    bool IsZero(Vector<T, N> const& v)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (v[i] != C_<T>(0))
            {
                return false;
            }
        }
        return true;
    }

    // Set all vector elements to one.
    template <typename T, size_t N>
    void MakeOne(Vector<T, N>& v)
    {
        v.fill(C_<T>(1));
    }

    // Test whether the vector is the one vector.
    template <typename T, size_t N>
    bool IsOne(Vector<T, N> const& v)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (v[i] != C_<T>(1))
            {
                return false;
            }
        }
        return true;
    }

    // For 0 <= d < N, set element d to 1 and all other elements to 0. If d
    // is out of range, then set all elements to 0. This function is a
    // convenience for creating the standard Euclidean basis for vectors.
    template <typename T, size_t N>
    void MakeBasis(size_t d, Vector<T, N>& v)
    {
        GTL_OUTOFRANGE_ASSERT(
            d < N,
            "Invalid dimension.");

        v.fill(C_<T>(0));
        v[d] = C_<T>(1);
    }

    // Test whether the vector is the basis vector whose d-th element is 1
    // and all other elements are 0.
    template <typename T, size_t N>
    bool IsBasis(size_t d, Vector<T, N> const& v)
    {
        GTL_OUTOFRANGE_ASSERT(
            d < N,
            "Invalid dimension.");

        for (size_t i = 0; i < N; ++i)
        {
            if (i != d)
            {
                if (v[i] != C_<T>(0))
                {
                    return false;
                }
            }
            else
            {
                if (v[i] != C_<T>(1))
                {
                    return false;
                }
            }
        }
        return true;
    }

    // Unary operations.
    template <typename T, size_t N>
    Vector<T, N> operator+(Vector<T, N> const& v)
    {
        return v;
    }

    template <typename T, size_t N>
    Vector<T, N> operator-(Vector<T, N> const& v)
    {
        Vector<T, N> result{};
        for (size_t i = 0; i < N; ++i)
        {
            result[i] = -v[i];
        }
        return result;
    }

    // Linear-algebraic operations.
    template <typename T, size_t N>
    Vector<T, N> operator+(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        Vector<T, N> result = v0;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] += v1[i];
        }
        return result;
    }

    template <typename T, size_t N>
    Vector<T, N>& operator+=(Vector<T, N>& v0, Vector<T, N> const& v1)
    {
        for (size_t i = 0; i < N; ++i)
        {
            v0[i] += v1[i];
        }
        return v0;
    }

    template <typename T, size_t N>
    Vector<T, N> operator-(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        Vector<T, N> result = v0;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] -= v1[i];
        }
        return result;
    }

    template <typename T, size_t N>
    Vector<T, N>& operator-=(Vector<T, N>& v0, Vector<T, N> const& v1)
    {
        for (size_t i = 0; i < N; ++i)
        {
            v0[i] -= v1[i];
        }
        return v0;
    }

    template <typename T, size_t N>
    Vector<T, N> operator*(Vector<T, N> const& v, T const& scalar)
    {
        Vector<T, N> result = v;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] *= scalar;
        }
        return result;
    }

    template <typename T, size_t N>
    Vector<T, N> operator*(T const& scalar, Vector<T, N> const& v)
    {
        Vector<T, N> result = v;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] *= scalar;
        }
        return result;
    }

    template <typename T, size_t N>
    Vector<T, N>& operator*=(Vector<T, N>& v, T const& scalar)
    {
        v = std::move(v * scalar);
        return v;
    }

    template <typename T, size_t N>
    Vector<T, N> operator/(Vector<T, N> const& v, T const& scalar)
    {
        Vector<T, N> result = v;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] /= scalar;
        }
        return result;
    }

    template <typename T, size_t N>
    Vector<T, N>& operator/=(Vector<T, N>& v, T const& scalar)
    {
        v = std::move(v / scalar);
        return v;
    }

    // Componentwise algebraic operations.
    template <typename T, size_t N>
    Vector<T, N> operator*(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        Vector<T, N> result = v0;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] *= v1[i];
        }
        return result;
    }

    template <typename T, size_t N>
    Vector<T, N>& operator*=(Vector<T, N>& v0, Vector<T, N> const& v1)
    {
        for (size_t i = 0; i < N; ++i)
        {
            v0[i] *= v1[i];
        }
        return v0;
    }

    template <typename T, size_t N>
    Vector<T, N> operator/(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        Vector<T, N> result = v0;
        for (size_t i = 0; i < N; ++i)
        {
            result[i] /= v1[i];
        }
        return result;
    }

    template <typename T, size_t N>
    Vector<T, N>& operator/=(Vector<T, N>& v0, Vector<T, N> const& v1)
    {
        for (size_t i = 0; i < N; ++i)
        {
            v0[i] /= v1[i];
        }
        return v0;
    }

    // Compute the dot product of two vectors.
    template <typename T, size_t N>
    T Dot(Vector<T, N> const& v0, Vector<T, N> const& v1)
    {
        T dot = C_<T>(0);
        for (size_t i = 0; i < N; ++i)
        {
            dot += v0[i] * v1[i];
        }
        return dot;
    }

    // Compute the length of a vector.
    template <typename T, size_t N>
    T Length(Vector<T, N> const& v)
    {
        return std::sqrt(Dot(v, v));
    }

    // Normalize the input v to unit length and return the original length.
    template <typename T, size_t N>
    T Normalize(Vector<T, N>& v)
    {
        T length = Length(v);
        if (length > C_<T>(0))
        {
            v /= length;
        }
        else
        {
            for (size_t i = 0; i < N; ++i)
            {
                v[i] = C_<T>(0);
            }
        }
        return length;
    }

    // Normalize the input v to unit length and return the original length.
    // The algorithm is robust to floating-point rounding errors.
    template <typename T, size_t N>
    T NormalizeRobust(Vector<T, N>& v)
    {
        T cmax = C_<T>(0);
        for (size_t i = 0; i < N; ++i)
        {
            T c = std::fabs(v[i]);
            if (c > cmax)
            {
                cmax = c;
            }
        }

        if (cmax > C_<T>(0))
        {
            int32_t cmaxExponent{};
            (void)std::frexp(cmax, &cmaxExponent);
            T length = C_<T>(0);
            for (size_t i = 0; i < N; ++i)
            {
                int32_t vExponent{};
                T vReduced = std::frexp(v[i], &vExponent);
                v[i] = std::ldexp(vReduced, vExponent - cmaxExponent);
                length += v[i] * v[i];
            }
            length = std::sqrt(length);
            for (size_t i = 0; i < N; ++i)
            {
                v[i] /= length;
            }
            length = std::ldexp(length, cmaxExponent);
            return length;
        }
        else
        {
            return C_<T>(0);
        }
    }

    // Gram-Schmidt orthonormalization to generate orthonormal vectors from
    // the linearly independent inputs. The function returns the smallest
    // length of the unnormalized vectors computed during the process. If
    // this value is nearly zero, it is possible that the inputs are linearly
    // dependent (within numerical round-off errors).
    template <typename T, size_t N>
    T Orthonormalize(std::vector<Vector<T, N>>& v)
    {
        GTL_OUTOFRANGE_ASSERT(
            0 < v.size() && v.size() <= N,
            "Invalid size of input v.");

        T minLength = Normalize(v[0]);
        for (size_t i = 1; i < v.size(); ++i)
        {
            for (size_t j = 0; j < i; ++j)
            {
                T dot = Dot(v[i], v[j]);
                v[i] -= v[j] * dot;
            }

            T length = Normalize(v[i]);
            if (length < minLength)
            {
                minLength = length;
            }
        }
        return minLength;
    }

    // Construct a single vector orthogonal to the nonzero input vector. If
    // the maximum absolute component occurs at index i, then the orthogonal
    // vector U has u[i] = v[i+1], u[i+1] = -v[i], and all other components
    // zero. The index addition i+1 is computed modulo N. If the input vector
    // is zero, the output vector is zero. If the input vector is empty, the
    // output vector is empty. If you want the output to be a unit-length
    // vector, set unitLength to 'true'.
    template <typename T, size_t N>
    Vector<T, N> GetOrthogonal(Vector<T, N> const& v, bool unitLength)
    {
        T cmax = C_<T>(0);
        size_t imax = 0;
        for (size_t i = 0; i < N; ++i)
        {
            T c = std::fabs(v[i]);
            if (c > cmax)
            {
                cmax = c;
                imax = i;
            }
        }

        Vector<T, N> result{};
        if (cmax > C_<T>(0))
        {
            size_t inext = imax + 1;
            if (inext == N)
            {
                inext = 0;
            }
            result[imax] = v[inext];
            result[inext] = -v[imax];
            if (unitLength)
            {
                Normalize(result);
            }
        }
        return result;
    }

    // Compute the axis-aligned bounding box of the vectors.
    template <typename T, size_t N>
    std::pair<Vector<T, N>, Vector<T, N>> ComputeExtremes(std::vector<Vector<T, N>> const& v)
    {
        GTL_OUTOFRANGE_ASSERT(
            v.size() > 0,
            "The input must have at least one vector.");

        std::pair<Vector<T, N>, Vector<T, N>> extreme{};
        extreme.first = v[0];
        extreme.second = extreme.first;
        for (size_t j = 1; j < v.size(); ++j)
        {
            auto const& vec = v[j];
            for (size_t i = 0; i < N; ++i)
            {
                if (vec[i] < extreme.first[i])
                {
                    extreme.first[i] = vec[i];
                }
                else if (vec[i] > extreme.second[i])
                {
                    extreme.second[i] = vec[i];
                }
            }
        }

        return extreme;
    }

    // Lift n-tuple v to homogeneous (n+1)-tuple (v,last).
    template <typename T, size_t N>
    Vector<T, N + 1> HLift(Vector<T, N> const& v, T const& last)
    {
        Vector<T, N + 1> result{};
        for (size_t i = 0; i < N; ++i)
        {
            result[i] = v[i];
        }
        result[N] = last;
        return result;
    }

    // Project homogeneous n-tuple v = (u,v[n-1]) to (n-1)-tuple u.
    template <typename T, size_t N>
    Vector<T, N - 1> HProject(Vector<T, N> const& v)
    {
        static_assert(
            N > 1,
            "Invalid dimension for a projection.");

        Vector<T, N - 1> result{};
        for (size_t i = 0; i < N - 1; ++i)
        {
            result[i] = v[i];
        }
        return result;
    }

    // Lift n-tuple v = (w0,w1) to (n+1)-tuple u = (w0,u[inject],w1). By
    // inference, w0 is a (inject)-tuple [nonexistent when inject=0] and w1
    // is a (n-inject)-tuple [nonexistent when inject=n].
    template <typename T, size_t N>
    Vector<T, N + 1> Lift(Vector<T, N> const& v, size_t inject, T const& value)
    {
        GTL_OUTOFRANGE_ASSERT(
            inject <= N,
            "Invalid index.");

        Vector<T, N + 1> result{};
        size_t i{};
        for (i = 0; i < inject; ++i)
        {
            result[i] = v[i];
        }
        result[i] = value;
        size_t j = i;
        for (++j; i < N; ++i, ++j)
        {
            result[j] = v[i];
        }
        return result;
    }

    // Project n-tuple v = (w0,v[reject],w1) to (n-1)-tuple u = (w0,w1). By
    // inference, w0 is a (reject)-tuple [nonexistent when reject=0] and w1
    // is a (n-1-reject)-tuple [nonexistent when reject=n-1].
    template <typename T, size_t N>
    Vector<T, N - 1> Project(Vector<T, N> const& v, size_t reject)
    {
        static_assert(
            N > 1,
            "Invalid dimension for a projection.");

        GTL_OUTOFRANGE_ASSERT(
            reject < N,
            "Invalid index.");

        Vector<T, N - 1> result{};
        for (size_t i = 0, j = 0; i < N - 1; ++i, ++j)
        {
            if (j == reject)
            {
                ++j;
            }
            result[i] = v[j];
        }
        return result;
    }
}

// Operations for Vector classes whose sizes are known only at run time.
namespace gtl
{
    // Allow sorting and comparing of objects.
    template <typename T>
    bool operator==(Vector<T> const& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        for (size_t i = 0; i < v0.size(); ++i)
        {
            if (v0[i] != v1[i])
            {
                return false;
            }
        }
        return true;
    }

    template <typename T>
    bool operator!=(Vector<T> const& v0, Vector<T> const& v1)
    {
        return !operator==(v0, v1);
    }

    template <typename T>
    bool operator<(Vector<T> const& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        for (size_t i = 0; i < v0.size(); ++i)
        {
            if (v0[i] < v1[i])
            {
                return true;
            }
            if (v0[i] > v1[i])
            {
                return false;
            }
        }
        return false;
    }

    template <typename T>
    bool operator<=(Vector<T> const& v0, Vector<T> const& v1)
    {
        return !operator<(v1, v0);
    }

    template <typename T>
    bool operator>(Vector<T> const& v0, Vector<T> const& v1)
    {
        return operator<(v1, v0);
    }

    template <typename T>
    bool operator>=(Vector<T> const& v0, Vector<T> const& v1)
    {
        return !operator<(v0, v1);
    }

    // Set all vector elements to zero.
    template <typename T>
    void MakeZero(Vector<T>& v)
    {
        v.fill(C_<T>(0));
    }

    // Test whether the vector is the zero vector.
    template <typename T>
    bool IsZero(Vector<T> const& v)
    {
        if (v.size() == 0)
        {
            return false;
        }

        for (size_t i = 0; i < v.size(); ++i)
        {
            if (v[i] != C_<T>(0))
            {
                return false;
            }
        }
        return true;
    }

    // Set all vector elements to one.
    template <typename T>
    void MakeOne(Vector<T>& v)
    {
        v.fill(C_<T>(1));
    }

    // Test whether the vector is the one IsVectorRT.
    template <typename T>
    bool IsOne(Vector<T> const& v)
    {
        if (v.size() == 0)
        {
            return false;
        }

        for (size_t i = 0; i < v.size(); ++i)
        {
            if (v[i] != C_<T>(1))
            {
                return false;
            }
        }
        return true;
    }

    // For 0 <= d < N, set element d to 1 and all other elements to 0. If d
    // is out of range, then set all elements to 0. This function is a
    // convenience for creating the standard Euclidean basis for vectors.
    template <typename T>
    void MakeBasis(size_t d, Vector<T>& v)
    {
        GTL_LENGTH_ASSERT(
            d < v.size(),
            "Invalid dimension.");

        v.fill(C_<T>(0));
        v[d] = C_<T>(1);
    }

    // Test whether the vector is the basis vector whose d-th element is 1
    // and all other elements are 0.
    template <typename T>
    bool IsBasis(size_t d, Vector<T> const& v)
    {
        GTL_OUTOFRANGE_ASSERT(
            d < v.size(),
            "Invalid dimension.");

        for (size_t i = 0; i < v.size(); ++i)
        {
            if (i != d)
            {
                if (v[i] != C_<T>(0))
                {
                    return false;
                }
            }
            else
            {
                if (v[i] != C_<T>(1))
                {
                    return false;
                }
            }
        }
        return true;
    }

    // Unary operations.
    template <typename T>
    Vector<T> operator+(Vector<T> const& v)
    {
        return v;
    }

    template <typename T>
    Vector<T> operator-(Vector<T> const& v)
    {
        Vector<T> result(v.size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            result[i] = -v[i];
        }
        return result;
    }

    // Linear-algebraic operations.
    template <typename T>
    Vector<T> operator+(Vector<T> const& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        Vector<T> result = v0;
        for (size_t i = 0; i < v0.size(); ++i)
        {
            result[i] += v1[i];
        }
        return result;
    }

    template <typename T>
    Vector<T>& operator+=(Vector<T>& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        for (size_t i = 0; i < v0.size(); ++i)
        {
            v0[i] += v1[i];
        }
        return v0;
    }

    template <typename T>
    Vector<T> operator-(Vector<T> const& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        Vector<T> result = v0;
        for (size_t i = 0; i < v0.size(); ++i)
        {
            result[i] -= v1[i];
        }
        return result;
    }

    template <typename T>
    Vector<T>& operator-=(Vector<T>& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        for (size_t i = 0; i < v0.size(); ++i)
        {
            v0[i] -= v1[i];
        }
        return v0;
    }

    template <typename T>
    Vector<T> operator*(Vector<T> const& v, T const& scalar)
    {
        Vector<T> result = v;
        for (size_t i = 0; i < v.size(); ++i)
        {
            result[i] *= scalar;
        }
        return result;
    }

    template <typename T>
    Vector<T> operator*(T const& scalar, Vector<T> const& v)
    {
        Vector<T> result = v;
        for (size_t i = 0; i < v.size(); ++i)
        {
            result[i] *= scalar;
        }
        return result;
    }

    template <typename T>
    Vector<T>& operator*=(Vector<T>& v, T const& scalar)
    {
        v = std::move(v * scalar);
        return v;
    }

    template <typename T>
    Vector<T> operator/(Vector<T> const& v, T const& scalar)
    {
        Vector<T> result = v;
        for (size_t i = 0; i < v.size(); ++i)
        {
            result[i] /= scalar;
        }
        return result;
    }

    template <typename T>
    Vector<T>& operator/=(Vector<T>& v, T const& scalar)
    {
        v = std::move(v / scalar);
        return v;
    }

    // Componentwise algebraic operations.
    template <typename T>
    Vector<T> operator*(Vector<T> const& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        Vector<T> result = v0;
        for (size_t i = 0; i < v0.size(); ++i)
        {
            result[i] *= v1[i];
        }
        return result;
    }

    template <typename T>
    Vector<T>& operator*=(Vector<T>& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        for (size_t i = 0; i < v0.size(); ++i)
        {
            v0[i] *= v1[i];
        }
        return v0;
    }

    template <typename T>
    Vector<T> operator/(Vector<T> const& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        Vector<T> result = v0;
        for (size_t i = 0; i < v0.size(); ++i)
        {
            result[i] /= v1[i];
        }
        return result;
    }

    template <typename T>
    Vector<T>& operator/=(Vector<T>& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        for (size_t i = 0; i < v0.size(); ++i)
        {
            v0[i] /= v1[i];
        }
        return v0;
    }

    // Compute the dot product of two vectors.
    template <typename T>
    T Dot(Vector<T> const& v0, Vector<T> const& v1)
    {
        GTL_LENGTH_ASSERT(
            v0.size() == v1.size(),
            "Mismatched sizes.");

        T dot = C_<T>(0);
        for (size_t i = 0; i < v0.size(); ++i)
        {
            dot += v0[i] * v1[i];
        }
        return dot;
    }

    // Compute the length of a vector.
    template <typename T>
    T Length(Vector<T> const& v)
    {
        return std::sqrt(Dot(v, v));
    }

    // Normalize the input v to unit length and return the original length.
    template <typename T>
    T Normalize(Vector<T>& v)
    {
        T length = Length(v);
        if (length > C_<T>(0))
        {
            v /= length;
        }
        else
        {
            for (size_t i = 0; i < v.size(); ++i)
            {
                v[i] = C_<T>(0);
            }
        }
        return length;
    }

    // Normalize the input v to unit length and return the original length.
    // The algorithm is robust to floating-point rounding errors.
    template <typename T>
    T NormalizeRobust(Vector<T>& v)
    {
        T cmax = C_<T>(0);
        for (size_t i = 0; i < v.size(); ++i)
        {
            T c = std::fabs(v[i]);
            if (c > cmax)
            {
                cmax = c;
            }
        }

        if (cmax > C_<T>(0))
        {
            int32_t cmaxExponent{};
            (void)std::frexp(cmax, &cmaxExponent);
            T length = C_<T>(0);
            for (size_t i = 0; i < v.size(); ++i)
            {
                int32_t vExponent{};
                T vReduced = std::frexp(v[i], &vExponent);
                v[i] = std::ldexp(vReduced, vExponent - cmaxExponent);
                length += v[i] * v[i];
            }
            length = std::sqrt(length);
            for (size_t i = 0; i < v.size(); ++i)
            {
                v[i] /= length;
            }
            length = std::ldexp(length, cmaxExponent);
            return length;
        }
        else
        {
            return C_<T>(0);
        }
    }

    // Gram-Schmidt orthonormalization to generate orthonormal vectors from
    // the linearly independent inputs. The function returns the smallest
    // length of the unnormalized vectors computed during the process. If
    // this value is nearly zero, it is possible that the inputs are linearly
    // dependent (within numerical round-off errors).
    template <typename T>
    T Orthonormalize(std::vector<Vector<T>>& v)
    {
        GTL_LENGTH_ASSERT(
            0 < v.size() && v.size() <= v[0].size(),
            "Mismatched sizes.");

        T minLength = Normalize(v[0]);
        for (size_t i = 1; i < v.size(); ++i)
        {
            GTL_LENGTH_ASSERT(
                v[i].size() == v[0].size(),
                "Mismatched sizes.");

            for (size_t j = 0; j < i; ++j)
            {
                T dot = Dot(v[i], v[j]);
                v[i] -= v[j] * dot;
            }

            T length = Normalize(v[i]);
            if (length < minLength)
            {
                minLength = length;
            }
        }
        return minLength;
    }

    // Construct a single vector orthogonal to the nonzero input vector. If
    // the maximum absolute component occurs at index i, then the orthogonal
    // vector U has u[i] = v[i+1], u[i+1] = -v[i], and all other components
    // zero. The index addition i+1 is computed modulo N. If the input vector
    // is zero, the output vector is zero. If the input vector is empty, the
    // output vector is empty. If you want the output to be a unit-length
    // vector, set unitLength to 'true'.
    template <typename T>
    Vector<T> GetOrthogonal(Vector<T> const& v, bool unitLength)
    {
        GTL_OUTOFRANGE_ASSERT(
            v.size() > 0,
            "The dimension must be positive.");

        T cmax = C_<T>(0);
        size_t imax = 0;
        for (size_t i = 0; i < v.size(); ++i)
        {
            T c = std::fabs(v[i]);
            if (c > cmax)
            {
                cmax = c;
                imax = i;
            }
        }

        Vector<T> result(v.size());
        if (cmax > C_<T>(0))
        {
            size_t inext = imax + 1;
            if (inext == v.size())
            {
                inext = 0;
            }
            result[imax] = v[inext];
            result[inext] = -v[imax];
            if (unitLength)
            {
                Normalize(result);
            }
        }
        return result;
    }

    // Compute the axis-aligned bounding box of the vectors.
    template <typename T>
    std::pair<Vector<T>, Vector<T>> ComputeExtremes(std::vector<Vector<T>> const& v)
    {
        GTL_OUTOFRANGE_ASSERT(
            v.size() > 0 && v[0].size() > 0,
            "The input must have at least one vector.");

        std::pair<Vector<T>, Vector<T>> extreme;
        extreme.first = v[0];
        extreme.second = extreme.first;
        for (size_t j = 1; j < v.size(); ++j)
        {
            GTL_LENGTH_ASSERT(
                v[j].size() == v[0].size(),
                "Mismatched sizes.");

            auto const& vec = v[j];
            for (size_t i = 0; i < v[0].size(); ++i)
            {
                if (vec[i] < extreme.first[i])
                {
                    extreme.first[i] = vec[i];
                }
                else if (vec[i] > extreme.second[i])
                {
                    extreme.second[i] = vec[i];
                }
            }
        }

        return extreme;
    }

    // Lift n-tuple v to homogeneous (n+1)-tuple (v,last).
    template <typename T>
    Vector<T> HLift(Vector<T> const& v, T const& last)
    {
        GTL_OUTOFRANGE_ASSERT(
            v.size() > 0,
            "The dimension must be positive.");

        Vector<T> result(v.size() + 1);
        for (size_t i = 0; i < v.size(); ++i)
        {
            result[i] = v[i];
        }
        result[v.size()] = last;
        return result;
    }

    // Project homogeneous n-tuple v = (u,v[n-1]) to (n-1)-tuple u.
    template <typename T>
    Vector<T> HProject(Vector<T> const& v)
    {
        size_t const size = v.size();
        GTL_OUTOFRANGE_ASSERT(
            size > 1,
            "Invalid dimension for a projection.");

        size_t const sizem1 = size - 1;
        Vector<T> result(sizem1);
        for (size_t i = 0; i< sizem1; ++i)
        {
            result[i] = v[i];
        }
        return result;
    }

    // Lift n-tuple v = (w0,w1) to (n+1)-tuple u = (w0,u[inject],w1). By
    // inference, w0 is a (inject)-tuple [nonexistent when inject=0] and w1
    // is a (n-inject)-tuple [nonexistent when inject=n].
    template <typename T>
    Vector<T> Lift(Vector<T> const& v, size_t inject, T const& value)
    {
        size_t const size = v.size();
        GTL_OUTOFRANGE_ASSERT(
            size > 0 && inject <= size,
            "The dimension must be positive and the index must be valid.");

        Vector<T> result(size + 1);
        size_t i{};
        for (i = 0; i < inject; ++i)
        {
            result[i] = v[i];
        }
        result[i] = value;
        size_t j = i;
        for (++j; i < size; ++i, ++j)
        {
            result[j] = v[i];
        }
        return result;
    }

    // Project n-tuple v = (w0,v[reject],w1) to (n-1)-tuple u = (w0,w1). By
    // inference, w0 is a (reject)-tuple [nonexistent when reject=0] and w1
    // is a (n-1-reject)-tuple [nonexistent when reject=n-1].
    template <typename T>
    Vector<T> Project(Vector<T> const& v, size_t reject)
    {
        size_t const size = v.size();
        GTL_OUTOFRANGE_ASSERT(
            size > 1 && reject < size,
            "The dimension must be at least 2 and the index must be valid.");

        size_t const sizem1 = size - 1;
        Vector<T> result(sizem1);
        for (size_t i = 0, j = 0; i < sizem1; ++i, ++j)
        {
            if (j == reject)
            {
                ++j;
            }
            result[i] = v[j];
        }
        return result;
    }
}

// Additional support for 2D vectors.
namespace gtl
{
    // Template alias for convenience.
    template <typename T> using Vector2 = Vector<T, 2>;

    // Compute the perpendicular using the formal determinant
    //   perp = det{{e0,e1},{x0,x1}}
    //   = (x1,-x0)
    // where e0 = (1,0), e1 = (0,1) and v = (x0,x1).
    template <typename T>
    Vector2<T> Perp(Vector2<T> const& v)
    {
        return Vector2<T>{ v[1], -v[0] };
    }

    // Compute the normalized perpendicular.
    template <typename T>
    Vector2<T> UnitPerp(Vector2<T> const& v)
    {
        Vector2<T> unitPerp{ v[1], -v[0] };
        Normalize(unitPerp);
        return unitPerp;
    }

    // Compute Dot((x0,x1),Perp(y0,y1)) = x0*y1 - x1*y0, where v0 = (x0,x1)
    // and v1 = (y0,y1).
    template <typename T>
    T DotPerp(Vector2<T> const& v0, Vector2<T> const& v1)
    {
        return Dot(v0, Perp(v1));
    }

    // Compute a right-handed orthonormal basis from a nonzero vector v0. The
    // function returns true when the basis is computed successfully, in which
    // case the matrix [v0 v1] is a rotation matrix. If the function returns
    // false, the outputs v0 and v1 are invalid.
    template <typename T>
    bool ComputeOrthonormalBasis(Vector2<T>& v0, Vector2<T>& v1)
    {
        Normalize(v0);
        if (IsZero(v0))
        {
            MakeZero(v1);
            return false;
        }

        v1 = -Perp(v0);
        return true;
    }

    // Compute a right-handed orthogonal basis from a nonzero vector v0. The
    // returned vector has the same length as v1. The function returns true
    // when the basis is computed successfully. If the function returns false,
    // the outputs v0 and v1 are invalid. This is an error-free function when
    // using arbitrary-precision arithmetic.
    template <typename T>
    bool ComputeOrthogonalBasis(Vector2<T>& v0, Vector2<T>& v1)
    {
        if (IsZero(v0))
        {
            MakeZero(v1);
            return false;
        }

        v1 = -Perp(v0);
        return true;
    }

    // Compute the barycentric coordinates of the point P with respect to the
    // triangle <V0,V1,V2>, P = b0*V0 + b1*V1 + b2*V2, where b0 + b1 + b2 = 1.
    // The return value is 'true' iff {V0,V1,V2} is a linearly independent
    // set. Numerically, this is measured by |det[V0 V1 V2]| <= epsilon. The
    // values bary[] are valid only when the return value is 'true' but set to
    // zero when the return value is 'false'.
    template <typename T>
    bool ComputeBarycentrics(Vector2<T> const& p, Vector2<T> const& v0,
        Vector2<T> const& v1, Vector2<T> const& v2, T const& epsilon,
        std::array<T, 3>& bary)
    {
        GTL_ARGUMENT_ASSERT(
            epsilon >= C_<T>(0),
            "Epsilon must be nonnegative.");

        // Compute the vectors relative to V2 of the triangle.
        std::array<Vector2<T>, 3> diff = { v0 - v2, v1 - v2, p - v2 };

        T det = DotPerp(diff[0], diff[1]);
        if (det < -epsilon || det > epsilon)
        {
            bary[0] = DotPerp(diff[2], diff[1]) / det;
            bary[1] = DotPerp(diff[0], diff[2]) / det;
            bary[2] = C_<T>(1) - bary[0] - bary[1];
            return true;
        }

        for (size_t i = 0; i < 3; ++i)
        {
            bary[i] = C_<T>(0);
        }
        return false;
    }

    // Get intrinsic information about the input array of vectors.
    template <typename T>
    class Intrinsics2
    {
    public:
        Intrinsics2()
            :
            epsilon(C_<T>(0)),
            dimension(0),
            min{ C_<T>(0), C_<T>(0) },
            max{ C_<T>(0), C_<T>(0) },
            maxRange(C_<T>(0)),
            origin(),
            direction(),
            extreme{ 0, 0, 0 },
            extremeCCW(false)
        {
        }

        void operator()(std::vector<Vector2<T>> const& points, T const& inEpsilon)
        {
            operator()(points.size(), points.data(), inEpsilon);
        }

        void operator()(size_t numPoints, Vector2<T> const* points, T const& inEpsilon)
        {
            GTL_ARGUMENT_ASSERT(
                numPoints > 0 && points != nullptr && inEpsilon >= C_<T>(0),
                "Invalid number of points, points pointer or epsilon.");

            epsilon = inEpsilon;
            dimension = 0;
            min = { C_<T>(0), C_<T>(0) };
            max = { C_<T>(0), C_<T>(0) };
            maxRange = C_<T>(0);
            origin = { C_<T>(0), C_<T>(0) };
            direction[0] = { C_<T>(0), C_<T>(0) };
            direction[1] = { C_<T>(0), C_<T>(0) };
            extreme = { 0, 0, 0 };
            extremeCCW = false;

            // Compute the axis-aligned bounding box for the input vectors.
            // Keep track of the indices into 'points' for the current min
            // and max.
            std::array<size_t, 2> indexMin = { 0, 0 };
            std::array<size_t, 2> indexMax = { 0, 0 };
            for (size_t j = 0; j < 2; ++j)
            {
                min[j] = points[0][j];
                max[j] = min[j];
                indexMin[j] = 0;
                indexMax[j] = 0;
            }

            for (size_t i = 1; i < numPoints; ++i)
            {
                for (size_t j = 0; j < 2; ++j)
                {
                    if (points[i][j] < min[j])
                    {
                        min[j] = points[i][j];
                        indexMin[j] = i;
                    }
                    else if (points[i][j] > max[j])
                    {
                        max[j] = points[i][j];
                        indexMax[j] = i;
                    }
                }
            }

            // Determine the maximum range for the bounding box.
            maxRange = max[0] - min[0];
            extreme[0] = indexMin[0];
            extreme[1] = indexMax[0];
            T range = max[1] - min[1];
            if (range > maxRange)
            {
                maxRange = range;
                extreme[0] = indexMin[1];
                extreme[1] = indexMax[1];
            }

            // The origin is either the vector of minimum x0-value or the
            // vector of minimum x1-value.
            origin = points[extreme[0]];

            // Test whether the vector set is (nearly) a vector.
            if (maxRange <= epsilon)
            {
                dimension = 0;
                for (size_t j = 0; j < 2; ++j)
                {
                    extreme[j + 1] = extreme[0];
                }
                return;
            }

            // Test whether the vector set is (nearly) a line segment. We need
            // direction[1] to span the orthogonal complement of direction[0].
            direction[0] = points[extreme[1]] - origin;
            Normalize(direction[0]);
            direction[1] = -Perp(direction[0]);

            // Compute the maximum distance of the points from the line
            // origin + t * direction[0].
            T maxDistance = C_<T>(0);
            T maxSign = C_<T>(0);
            extreme[2] = extreme[0];
            for (size_t i = 0; i < numPoints; ++i)
            {
                Vector2<T> diff = points[i] - origin;
                T distance = Dot(direction[1], diff);
                T sign{};
                if (distance > C_<T>(0))
                {
                    sign = C_<T>(1);
                }
                else if (distance < C_<T>(0))
                {
                    sign = -C_<T>(1);
                }
                else
                {
                    sign = C_<T>(0);
                }
                distance = std::fabs(distance);
                if (distance > maxDistance)
                {
                    maxDistance = distance;
                    maxSign = sign;
                    extreme[2] = i;
                }
            }

            if (maxDistance <= epsilon * maxRange)
            {
                // The points are (nearly) on the line
                // origin + t * direction[0].
                dimension = 1;
                extreme[2] = extreme[1];
            }
            else
            {
                dimension = 2;
                extremeCCW = (maxSign > C_<T>(0));
            }
        }

        // A nonnegative tolerance that is used to determine the intrinsic
        // dimension of the set.
        T epsilon;

        // The intrinsic dimension of the input set, computed based on the
        // nonnegative tolerance epsilon.
        size_t dimension;

        // Axis-aligned bounding box of the input set. The maximum range is
        // the larger of max[0]-min[0] and max[1]-min[1].
        std::array<T, 2> min, max;
        T maxRange;

        // Coordinate system. The origin is valid for any dimension d. The
        // unit-length direction vector is valid only for 0 <= i < d. The
        // extreme index is relative to the array of input points, and is also
        // valid only for 0 <= i < d. If d = 0, all points are effectively
        // the same, but the use of an epsilon may lead to an extreme index
        // that is not zero. If d = 1, all points effectively lie on a line
        // segment. If d = 2, the points are not collinear.
        Vector2<T> origin;
        std::array<Vector2<T>, 2> direction;

        // The indices that define the maximum dimensional extents. The
        // values extreme[0] and extreme[1] are the indices for the points
        // that define the largest extent in one of the coordinate axis
        // directions. If the dimension is 2, then extreme[2] is the index
        // for the point that generates the largest extent in the direction
        // perpendicular to the line through the points corresponding to
        // extreme[0] and extreme[1]. The triangle formed by the points
        // V[extreme[0]], V[extreme[1]] and V[extreme[2]] is clockwise or
        // counterclockwise, the condition stored in extremeCCW.
        std::array<size_t, 3> extreme;
        bool extremeCCW;
    };
}

// Additional support for 3D vectors.
namespace gtl
{
    // Template alias for convenience.
    template <typename T> using Vector3 = Vector<T, 3>;

    // Compute the cross product using the formal determinant
    //   cross = det{{e0,e1,e2},{x0,x1,x2},{y0,y1,y2}}
    //   = (x1*y2-x2*y1, x2*y0-x0*y2, x0*y1-x1*y0)
    // where e0 = (1,0,0), e1 = (0,1,0), e2 = (0,0,1), v0 = (x0,x1,x2) and
    // v1 = (y0,y1,y2).
    template <typename T>
    Vector3<T> Cross(Vector3<T> const& v0, Vector3<T> const& v1)
    {
        return Vector3<T>
        {
            v0[1] * v1[2] - v0[2] * v1[1],
            v0[2] * v1[0] - v0[0] * v1[2],
            v0[0] * v1[1] - v0[1] * v1[0]
        };
    }

    // Compute the normalized cross product.
    template <typename T>
    Vector3<T> UnitCross(Vector3<T> const& v0, Vector3<T> const& v1)
    {
        Vector3<T> unitCross = Cross(v0, v1);
        Normalize(unitCross);
        return unitCross;
    }

    // Compute Dot((x0,x1,x2),Cross((y0,y1,y2),(z0,z1,z2)), the triple scalar
    // product of three vectors, where v0 = (x0,x1,x2), v1 = (y0,y1,y2) and
    // v2 is (z0,z1,z2).
    template <typename T>
    T DotCross(Vector3<T> const& v0, Vector3<T> const& v1, Vector3<T> const& v2)
    {
        return Dot(Cross(v0, v1), v2);
    }

    // Compute a right-handed orthonormal basis from one nonzero vector v0 or
    // from two linearly independent vectors v0 and v1. The function returns
    // true when the basis is computed successfully, in which case the matrix
    // [v0 v1 v2] is a rotation matrix. If the function returns false, the
    // outputs v0, v1 and v2 are invalid.
    template <typename T>
    bool ComputeOrthonormalBasis(size_t numInputs, Vector3<T>& v0,
        Vector3<T>& v1, Vector3<T>& v2)
    {
        GTL_ARGUMENT_ASSERT(
            1 <= numInputs && numInputs <= 3,
            "Invalid number of inputs.");

        Normalize(v0);
        if (IsZero(v0))
        {
            MakeZero(v1);
            MakeZero(v2);
            return false;
        }

        if (numInputs == 1)
        {
            if (std::fabs(v0[0]) > std::fabs(v0[1]))
            {
                v1 = { -v0[2], C_<T>(0), v0[0] };
            }
            else
            {
                v1 = { C_<T>(0), v0[2], -v0[1] };
            }
        }
        else // numInputs >= 2
        {
            v1 -= Dot(v1, v0) * v0;
        }

        Normalize(v1);
        if (IsZero(v1))
        {
            MakeZero(v2);
            return false;
        }

        v2 = UnitCross(v0, v1);
        return !IsZero(v2);
    }

    // Compute a right-handed orthogonal basis from one nonzero vector v0
    // or from two linearly independent vectors v0 and v1. The function
    // returns true when the basis is computed successfully. If the function
    // returns false, the outputs v0, v1 and v2 are invalid. The returned
    // vectors are computed error free when using arbitrary-precision
    // arithmetic.
    template <typename T>
    bool ComputeOrthogonalBasis(size_t numInputs, Vector3<T>& v0,
        Vector3<T>& v1, Vector3<T>& v2)
    {
        GTL_ARGUMENT_ASSERT(
            1 <= numInputs && numInputs <= 3,
            "Invalid number of inputs.");

        if (numInputs == 1)
        {
            if (std::fabs(v0[0]) > std::fabs(v0[1]))
            {
                v1 = { -v0[2], C_<T>(0), v0[0] };
            }
            else
            {
                v1 = { C_<T>(0), v0[2], -v0[1] };
            }
        }
        else // numInputs == 2 || numInputs == 3
        {
            v1 = Dot(v0, v0) * v1 - Dot(v1, v0) * v0;
        }

        if (IsZero(v1))
        {
            MakeZero(v2);
            return false;
        }

        v2 = Cross(v0, v1);
        return !IsZero(v2);
    }

    // Compute the barycentric coordinates of the point P with respect to the
    // tetrahedron <V0,V1,V2,V3>, P = b0*V0 + b1*V1 + b2*V2 + b3*V3, where
    // b0 + b1 + b2 + b3 = 1. The return value is 'true' iff {V0,V1,V2,V3} is
    // a linearly independent set.  Numerically, this is measured by
    // |det[V0 V1 V2 V3]| <= epsilon. The values bary[] are valid only when
    // the return value is 'true' but set to zero when the return value is
    // 'false'.
    template <typename T>
    bool ComputeBarycentrics(Vector3<T> const& p, Vector3<T> const& v0,
        Vector3<T> const& v1, Vector3<T> const& v2, Vector3<T> const& v3,
        T const& epsilon, std::array<T, 4>& bary)
    {
        GTL_ARGUMENT_ASSERT(
            epsilon >= C_<T>(0),
            "Epsilon must be nonnegative.");

        // Compute the vectors relative to V3 of the tetrahedron.
        std::array<Vector3<T>, 4> diff = { v0 - v3, v1 - v3, v2 - v3, p - v3 };

        T det = DotCross(diff[0], diff[1], diff[2]);
        if (det < -epsilon || det > epsilon)
        {
            bary[0] = DotCross(diff[3], diff[1], diff[2]) / det;
            bary[1] = DotCross(diff[3], diff[2], diff[0]) / det;
            bary[2] = DotCross(diff[3], diff[0], diff[1]) / det;
            bary[3] = C_<T>(1) - bary[0] - bary[1] - bary[2];
            return true;
        }

        for (size_t i = 0; i < 4; ++i)
        {
            bary[i] = C_<T>(0);
        }
        return false;
    }

    // Get intrinsic information about the input array of vectors.
    template <typename T>
    class Intrinsics3
    {
    public:
        Intrinsics3()
            :
            epsilon(C_<T>(0)),
            dimension(0),
            min{ C_<T>(0), C_<T>(0), C_<T>(0) },
            max{ C_<T>(0), C_<T>(0), C_<T>(0) },
            maxRange(C_<T>(0)),
            origin(),
            direction(),
            extreme{ 0, 0, 0, 0 },
            extremeCCW(false)
        {
        }

        void operator()(std::vector<Vector3<T>> const& points, T const& inEpsilon)
        {
            operator()(points.size(), points.data(), inEpsilon);
        }

        void operator()(size_t numPoints, Vector3<T> const* points, T const& inEpsilon)
        {
            GTL_ARGUMENT_ASSERT(
                numPoints > 0 && points != nullptr && inEpsilon >= C_<T>(0),
                "Invalid number of points, points pointer or epsilon.");

            epsilon = inEpsilon;
            dimension = 0;
            min = { C_<T>(0), C_<T>(0), C_<T>(0) };
            max = { C_<T>(0), C_<T>(0), C_<T>(0) };
            maxRange = C_<T>(0);
            origin = { C_<T>(0), C_<T>(0), C_<T>(0) };
            direction[0] = { C_<T>(0), C_<T>(0), C_<T>(0) };
            direction[1] = { C_<T>(0), C_<T>(0), C_<T>(0) };
            extreme = { 0, 0, 0, 0 };
            extremeCCW = false;

            // Compute the axis-aligned bounding box for the input vectors.
            // Keep track of the indices into 'points' for the current min
            // and max.
            std::array<size_t, 3> indexMin = { 0, 0, 0 };
            std::array<size_t, 3> indexMax = { 0, 0, 0 };
            for (size_t j = 0; j < 3; ++j)
            {
                min[j] = points[0][j];
                max[j] = min[j];
                indexMin[j] = 0;
                indexMax[j] = 0;
            }

            for (size_t i = 1; i < numPoints; ++i)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    if (points[i][j] < min[j])
                    {
                        min[j] = points[i][j];
                        indexMin[j] = i;
                    }
                    else if (points[i][j] > max[j])
                    {
                        max[j] = points[i][j];
                        indexMax[j] = i;
                    }
                }
            }

            // Determine the maximum range for the bounding box.
            maxRange = max[0] - min[0];
            extreme[0] = indexMin[0];
            extreme[1] = indexMax[0];
            T range = max[1] - min[1];
            if (range > maxRange)
            {
                maxRange = range;
                extreme[0] = indexMin[1];
                extreme[1] = indexMax[1];
            }
            range = max[2] - min[2];
            if (range > maxRange)
            {
                maxRange = range;
                extreme[0] = indexMin[2];
                extreme[1] = indexMax[2];
            }

            // The origin is either the vector of minimum x0-value, the vector of
            // minimum x1-value or the vector of minimum x2-value.
            origin = points[extreme[0]];

            // Test whether the vector set is (nearly) a vector.
            if (maxRange <= epsilon)
            {
                dimension = 0;
                for (size_t j = 0; j < 3; ++j)
                {
                    extreme[j + 1] = extreme[0];
                }
                return;
            }

            // Test whether the vector set is (nearly) a line segment. We need
            // {direction[2],direction[3]} to span the orthogonal complement of
            // direction[0].
            direction[0] = points[extreme[1]] - origin;
            Normalize(direction[0]);
            if (std::fabs(direction[0][0]) > std::fabs(direction[0][1]))
            {
                direction[1] = Vector3<T>{ -direction[0][2], C_<T>(0), direction[0][0] };
            }
            else
            {
                direction[1] = Vector3<T>{ C_<T>(0), direction[0][2], -direction[0][1] };
            }
            Normalize(direction[1]);
            direction[2] = Cross(direction[0], direction[1]);

            // Compute the maximum distance of the points from the line
            // origin + t * direction[0].
            T maxDistance = C_<T>(0);
            T distance{}, dot{};
            extreme[2] = extreme[0];
            for (size_t i = 0; i < numPoints; ++i)
            {
                Vector3<T> diff = points[i] - origin;
                dot = Dot(direction[0], diff);
                Vector3<T> proj = diff - dot * direction[0];
                distance = Length(proj);
                if (distance > maxDistance)
                {
                    maxDistance = distance;
                    extreme[2] = i;
                }
            }

            if (maxDistance <= epsilon * maxRange)
            {
                // The points are (nearly) on the line
                // origin + t * direction[0].
                dimension = 1;
                extreme[2] = extreme[1];
                extreme[3] = extreme[1];
                return;
            }

            // Test whether the vector set is (nearly) a planar polygon. The
            // point points[extreme[2]] is farthest from the line:
            // origin + t * direction[0]. The point points[extreme[2]]-origin
            // is not necessarily perpendicular to direction[0]. Project out
            // the direction[0] component so that the result is perpendicular
            // to direction[0].
            direction[1] = points[extreme[2]] - origin;
            dot = Dot(direction[0], direction[1]);
            direction[1] -= dot * direction[0];
            Normalize(direction[1]);

            // We need direction[2] to span the orthogonal complement of
            // {direction[0], direction[1]}.
            direction[2] = Cross(direction[0], direction[1]);

            // Compute the maximum distance of the points from the plane
            // origin + t0 * direction[0] + t1 * direction[1].
            maxDistance = C_<T>(0);
            T maxSign = C_<T>(0);
            extreme[3] = extreme[0];
            for (size_t i = 0; i < numPoints; ++i)
            {
                Vector3<T> diff = points[i] - origin;
                distance = Dot(direction[2], diff);
                T sign{};
                if (distance > C_<T>(0))
                {
                    sign = C_<T>(1);
                }
                else if (distance < C_<T>(0))
                {
                    sign = -C_<T>(1);
                }
                else
                {
                    sign = C_<T>(0);
                }
                distance = std::fabs(distance);
                if (distance > maxDistance)
                {
                    maxDistance = distance;
                    maxSign = sign;
                    extreme[3] = i;
                }
            }

            if (maxDistance <= epsilon * maxRange)
            {
                // The points are (nearly) on the plane
                // origin + t0 * direction[0] + t1 * direction[1].
                dimension = 2;
                extreme[3] = extreme[2];
            }
            else
            {
                dimension = 3;
                extremeCCW = (maxSign > C_<T>(0));
            }
        }

        // A nonnegative tolerance that is used to determine the intrinsic
        // dimension of the set.
        T epsilon;

        // The intrinsic dimension of the input set, computed based on the
        // nonnegative tolerance epsilon.
        size_t dimension;

        // Axis-aligned bounding box of the input set. The maximum range is
        // the largest of max[0]-min[0], max[1]-min[1] and max[2]-min[2].
        std::array<T, 3> min, max;
        T maxRange;

        // Coordinate system. The origin is valid for any dimension d. The
        // unit-length direction vector is valid only for 0 <= i < d. The
        // extreme index is relative to the array of input points, and is also
        // valid only for 0 <= i < d. If d = 0, all points are effectively
        // the same, but the use of an epsilon may lead to an extreme index
        // that is not zero. If d = 1, all points effectively lie on a line
        // segment. If d = 2, all points effectively line on a plane. If
        // d = 3, the points are not coplanar.
        Vector3<T> origin;
        std::array<Vector3<T>, 3> direction;

        // The indices that define the maximum dimensional extents. The
        // values extreme[0] and extreme[1] are the indices for the points
        // that define the largest extent in one of the coordinate axis
        // directions. If the dimension is 2, then extreme[2] is the index
        // for the point that generates the largest extent in the direction
        // perpendicular to the line through the points corresponding to
        // extreme[0] and extreme[1]. Furthermore, if the dimension is 3,
        // then extreme[3] is the index for the point that generates the
        // largest extent in the direction perpendicular to the triangle
        // defined by the other extreme points. The tetrahedron formed by the
        // points V[extreme[0]], V[extreme[1]], V[extreme[2]] and
        // V[extreme[3]] is clockwise or counterclockwise, the condition
        // stored in extremeCCW.
        std::array<size_t, 4> extreme;
        bool extremeCCW;
    };
}

// Additional support for 4D vectors.
namespace gtl
{
    // Template alias for convenience.
    template <typename T>
    using Vector4 = Vector<T, 4>;

    // Compute the hypercross product using the formal determinant
    //   hcross = det{{e0,e1,e2,e3},{x0,x1,x2,x3},{y0,y1,y2,y3},{z0,z1,z2,z3}}
    // where e0 = (1,0,0,0), e1 = (0,1,0,0), e2 = (0,0,1,0), e3 = (0,0,0,1),
    // v0 = (x0,x1,x2,x3), v1 = (y0,y1,y2,y3) and v2 = (z0,z1,z2,z3).
    template <typename T>
    Vector4<T> HyperCross(Vector4<T> const& v0, Vector4<T> const& v1,
        Vector4<T> const& v2)
    {
        T m01 = v0[0] * v1[1] - v0[1] * v1[0];
        T m02 = v0[0] * v1[2] - v0[2] * v1[0];
        T m03 = v0[0] * v1[3] - v0[3] * v1[0];
        T m12 = v0[1] * v1[2] - v0[2] * v1[1];
        T m13 = v0[1] * v1[3] - v0[3] * v1[1];
        T m23 = v0[2] * v1[3] - v0[3] * v1[2];
        return Vector4<T>
        {
            +m23 * v2[1] - m13 * v2[2] + m12 * v2[3],
            -m23 * v2[0] + m03 * v2[2] - m02 * v2[3],
            +m13 * v2[0] - m03 * v2[1] + m01 * v2[3],
            -m12 * v2[0] + m02 * v2[1] - m01 * v2[2]
        };
    }

    // Compute the normalized hypercross product.
    template <typename T>
    Vector4<T> UnitHyperCross(Vector4<T> const& v0, Vector4<T> const& v1,
        Vector4<T> const& v2)
    {
        Vector4<T> unitHyperCross = HyperCross(v0, v1, v2);
        Normalize(unitHyperCross);
        return unitHyperCross;
    }

    // Compute Dot(HyperCross((x0,x1,x2,x3),(y0,y1,y2,y3),(z0,z1,z2,z3)),
    // (w0,w1,w2,w3)), where v0 = (x0,x1,x2,x3), v1 = (y0,y1,y2,y3),
    // v2 = (z0,z1,z2,z3), and v3 = (w0,w1,w2,w3).
    template <typename T>
    T DotHyperCross(Vector4<T> const& v0, Vector4<T> const& v1,
        Vector4<T> const& v2, Vector4<T> const& v3)
    {
        return Dot(HyperCross(v0, v1, v2), v3);
    }

    // Compute a right-handed orthonormal basis from one nonzero vector v0
    // or from two linearly independent vectors v0 and v1 or from three
    // linearly independent vectors v0, v1 and v2. The function returns the
    // length of v0 (if the number of inputs is 1) or the smallest length of
    // v0 and v1 (if the number of inputs is 2) or the smallest length of the
    // v0, v1 and v2 (if the number of inputs is 3). The matrix
    // [v0 v1 v2 v3] whose columns are the output v0, v1, v2 and v3 is a
    // rotation matrix.

    // Compute a right-handed orthonormal basis from one nonzero vector v0,
    // from two linearly independent vectors v0 and v1, or from three linearly
    // independent vectors v0, v1 and v2. The function returns true when the
    // basis is computed successfully, in which case the matrix [v0 v1 v2 v3]
    // is a rotation matrix. If the function returns false, the outputs v0,
    // v1, v2 and v3 are invalid.
    template <typename T>
    bool ComputeOrthonormalBasis(size_t numInputs, Vector4<T>& v0,
        Vector4<T>& v1, Vector4<T>& v2, Vector4<T>& v3)
    {
        GTL_ARGUMENT_ASSERT(
            1 <= numInputs && numInputs <= 4,
            "Invalid number of inputs.");

        Normalize(v0);
        if (IsZero(v0))
        {
            MakeZero(v1);
            MakeZero(v2);
            MakeZero(v3);
            return false;
        }

        if (numInputs == 1)
        {
            size_t maxIndex = 0;
            T maxAbsValue = std::fabs(v0[0]);
            for (size_t i = 1; i < 4; ++i)
            {
                T absValue = std::fabs(v0[i]);
                if (absValue > maxAbsValue)
                {
                    maxIndex = i;
                    maxAbsValue = absValue;
                }
            }

            if (maxIndex < 2)
            {
                v1 = { -v0[1], v0[0], C_<T>(0), C_<T>(0) };
            }
            else  // maxIndex >= 2
            {
                v1 = { C_<T>(0), C_<T>(0), -v0[3], v0[2] };
            }
        }
        else  // numInputs >= 2
        {
            v1 -= Dot(v1, v0) * v0;
        }

        Normalize(v1);
        if (IsZero(v1))
        {
            MakeZero(v2);
            MakeZero(v3);
            return false;
        }

        if (numInputs < 3)
        {
            // v0 = (x0, y0, z0, w0), v1 = (x1, y1, z1, w1)
            // det = (x0 * y1 - x1 * y0, x0 * z1 - x1 * z0, x0 * w1 - x1 * w0,
            //        y0 * z1 - y1 * z0, y0 * w1 - y1 * w0, z0 * w1 - z1 * w0)
            std::array<T, 6> det =
            {
                v0[0] * v1[1] - v1[0] * v0[1],  // x0 * y1 - x1 * y0
                v0[0] * v1[2] - v1[0] * v0[2],  // x0 * z1 - x1 * z0
                v0[0] * v1[3] - v1[0] * v0[3],  // x0 * w1 - x1 * w0
                v0[1] * v1[2] - v1[1] * v0[2],  // y0 * z1 - y1 * z0
                v0[1] * v1[3] - v1[1] * v0[3],  // y0 * w1 - y1 * w0
                v0[2] * v1[3] - v1[2] * v0[3]   // z0 * w1 - z1 * w0
            };

            size_t maxIndex = 0;
            T maxAbsValue = std::fabs(det[0]);
            for (size_t i = 1; i < 6; ++i)
            {
                T absValue = std::fabs(det[i]);
                if (absValue > maxAbsValue)
                {
                    maxIndex = i;
                    maxAbsValue = absValue;
                }
            }

            if (maxIndex == 0)
            {
                v2 = { -det[4], det[2], C_<T>(0), -det[0] };
            }
            else if (maxIndex <= 2)
            {
                v2 = { det[5], C_<T>(0), -det[2], det[1] };
            }
            else // maxIndex >= 3
            {
                v2 = { C_<T>(0), -det[5], det[4], -det[3] };
            }
        }
        else  // numInputs >= 3
        {
            v2 -= Dot(v0, v2) * v0 + Dot(v1, v2) * v1;
        }

        Normalize(v2);
        if (IsZero(v2))
        {
            MakeZero(v3);
            return false;
        }

        v3 = UnitHyperCross(v0, v1, v2);
        return !IsZero(v3);
    }
}
