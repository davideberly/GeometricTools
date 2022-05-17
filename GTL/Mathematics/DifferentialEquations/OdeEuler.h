// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.17

#pragma once

// Euler's method for numerical approximation of solutions to dx/dt = F(t,x),
// where x(t) is a vector-valued function of a real-valued variable t. The
// initial condition is x(t0) = x0.

#include <GTL/Mathematics/DifferentialEquations/OdeSolver.h>
#include <cstddef>
#include <functional>

namespace gtl
{
    template <typename T, size_t...> class OdeEuler;

    template <typename T>
    class OdeEuler<T, 1> : public OdeSolver<T, 1>
    {
    public:
        // Construction and destruction.
        virtual ~OdeEuler() = default;

        OdeEuler(T const& tDelta, std::function<T(T const&, T const&)> const& F)
            :
            OdeSolver<T, 1>(tDelta, F)
        {
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). You may
        // allow xIn and xOut to be the same object.
        virtual void Update(T const& tIn, T const& xIn,
            T& tOut, T& xOut) override
        {
            auto fValue = this->mFunction(tIn, xIn);
            tOut = tIn + this->mTDelta;
            xOut = xIn + this->mTDelta * fValue;
        }

    private:
        friend class UnitTestOdeEuler;
    };

    template <typename T, size_t N>
    class OdeEuler<T, N> : public OdeSolver<T, N>
    {
    public:
        // Construction and destruction.
        virtual ~OdeEuler() = default;

        OdeEuler(T const& tDelta,
            std::function<Vector<T, N>(T const&, Vector<T, N> const&)> const& F)
            :
            OdeSolver<T, N>(tDelta, F)
        {
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). You may
        // allow xIn and xOut to be the same object.
        virtual void Update(T const& tIn, Vector<T, N> const& xIn,
            T& tOut, Vector<T, N>& xOut) override
        {
            auto fVector = this->mFunction(tIn, xIn);
            tOut = tIn + this->mTDelta;
            xOut = xIn + this->mTDelta * fVector;
        }

    private:
        friend class UnitTestOdeEuler;
    };

    template <typename T>
    class OdeEuler<T> : public OdeSolver<T>
    {
    public:
        // Construction and destruction.
        virtual ~OdeEuler() = default;

        OdeEuler(T const& tDelta,
            std::function<Vector<T>(T const&, Vector<T> const&)> const& F)
            :
            OdeSolver<T>(tDelta, F)
        {
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). You may
        // allow xIn and xOut to be the same object.
        virtual void Update(T const& tIn, Vector<T> const& xIn, T& tOut, Vector<T>& xOut) override
        {
            auto fVector = this->mFunction(tIn, xIn);
            tOut = tIn + this->mTDelta;
            xOut = xIn + this->mTDelta * fVector;
        }

    private:
        friend class UnitTestOdeEuler;
    };
}
