// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.17

#pragma once

// The differential equation is dx/dt = F(t,x), where x(t) is a vector-valued
// function of a real-valued variable t. The initial condition is x(t0) = x0.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <cstddef>
#include <functional>

namespace gtl
{
    template <typename T, size_t...> class OdeSolver;

    template <typename T>
    class OdeSolver<T, 1>
    {
    public:
        // Abstract base class.
        virtual ~OdeSolver() = default;

    protected:
        OdeSolver(T const& tDelta,
            std::function<T(T const&, T const&)> const& F)
            :
            mTDelta(tDelta),
            mFunction(F)
        {
        }

    public:
        // Member access.
        inline void SetTDelta(T const& tDelta)
        {
            mTDelta = tDelta;
        }

        inline T const& GetTDelta() const
        {
            return mTDelta;
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). The derived
        // classes implement this so that it is possible for xIn and xOut to
        // be the same object.
        virtual void Update(T const& tIn, T const& xIn,
            T& tOut, T& xOut) = 0;

    protected:
        T mTDelta;
        std::function<T(T const&, T const&)> mFunction;

    private:
        friend class UnitTestOdeSolver;
    };

    template <typename T, size_t N>
    class OdeSolver<T, N>
    {
    public:
        // Abstract base class.
        virtual ~OdeSolver() = default;

    protected:
        OdeSolver(T const& tDelta,
            std::function<Vector<T, N>(T const&, Vector<T, N> const&)> const& F)
            :
            mTDelta(tDelta),
            mFunction(F)
        {
        }

    public:
        // Member access.
        inline void SetTDelta(T const& tDelta)
        {
            mTDelta = tDelta;
        }

        inline T const& GetTDelta() const
        {
            return mTDelta;
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). The derived
        // classes implement this so that it is possible for xIn and xOut to
        // be the same object.
        virtual void Update(T const& tIn, Vector<T, N> const& xIn,
            T& tOut, Vector<T, N>& xOut) = 0;

    protected:
        T mTDelta;
        std::function<Vector<T, N>(T const&, Vector<T, N> const&)> mFunction;

    private:
        friend class UnitTestOdeSolver;
    };

    template <typename T>
    class OdeSolver<T>
    {
    public:
        // Abstract base class.
        virtual ~OdeSolver() = default;

    protected:
        OdeSolver(T const& tDelta,
            std::function<Vector<T>(T const&, Vector<T> const&)> const& F)
            :
            mTDelta(tDelta),
            mFunction(F)
        {
        }

    public:
        // Member access.
        inline void SetTDelta(T const& tDelta)
        {
            mTDelta = tDelta;
        }

        inline T const& GetTDelta() const
        {
            return mTDelta;
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). The derived
        // classes implement this so that it is possible for xIn and xOut to
        // be the same object.
        virtual void Update(T const& tIn, Vector<T> const& xIn,
            T& tOut, Vector<T>& xOut) = 0;

    protected:
        T mTDelta;
        std::function<Vector<T>(T const&, Vector<T> const&)> mFunction;

    private:
        friend class UnitTestOdeSolver;
    };
}
