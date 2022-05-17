// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.17

#pragma once

// Runge-Kutta 4th-order method for numerical approximation of solutions to
// dx/dt = F(t,x), where x(t) is a vector-valued function of a real-valued
// variable t. The initial condition is x(t0) = x0.

#include <GTL/Mathematics/DifferentialEquations/OdeSolver.h>
#include <cstddef>
#include <functional>

namespace gtl
{
    template <typename T, size_t...> class OdeRungeKutta4;

    template <typename T>
    class OdeRungeKutta4<T, 1> : public OdeSolver<T, 1>
    {
    public:
        // Construction and destruction.
        virtual ~OdeRungeKutta4() = default;

        OdeRungeKutta4(T const& tDelta,
            std::function<T(T const&, T const&)> const& F)
            :
            OdeSolver<T, 1>(tDelta, F)
        {
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). You may
        // allow xIn and xOut to be the same object.
        virtual void Update(T const& tIn, T const& xIn,
            T& tOut, T& xOut) override
        {
            // Compute the first step.
            T halfTDelta = C_<T>(1, 2) * this->mTDelta;
            auto fTemp1 = this->mFunction(tIn, xIn);
            auto xTemp = xIn + halfTDelta * fTemp1;

            // Compute the second step.
            T halfT = tIn + halfTDelta;
            auto fTemp2 = this->mFunction(halfT, xTemp);
            xTemp = xIn + halfTDelta * fTemp2;

            // Compute the third step.
            auto fTemp3 = this->mFunction(halfT, xTemp);
            xTemp = xIn + this->mTDelta * fTemp3;

            // Compute the fourth step.
            T sixthTDelta = this->mTDelta / C_<T>(6);
            tOut = tIn + this->mTDelta;
            auto fTemp4 = this->mFunction(tOut, xTemp);
            xOut = xIn + sixthTDelta * (
                fTemp1 + C_<T>(2) * (fTemp2 + fTemp3) + fTemp4);
        }

    private:
        friend class UnitTestOdeRungeKutta4;
    };

    template <typename T, size_t N>
    class OdeRungeKutta4<T, N> : public OdeSolver<T, N>
    {
    public:
        // Construction and destruction.
        virtual ~OdeRungeKutta4() = default;

        OdeRungeKutta4(T const& tDelta,
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
            // Compute the first step.
            T halfTDelta = C_<T>(1, 2) * this->mTDelta;
            auto fTemp1 = this->mFunction(tIn, xIn);
            auto xTemp = xIn + halfTDelta * fTemp1;

            // Compute the second step.
            T halfT = tIn + halfTDelta;
            auto fTemp2 = this->mFunction(halfT, xTemp);
            xTemp = xIn + halfTDelta * fTemp2;

            // Compute the third step.
            auto fTemp3 = this->mFunction(halfT, xTemp);
            xTemp = xIn + this->mTDelta * fTemp3;

            // Compute the fourth step.
            T sixthTDelta = this->mTDelta / C_<T>(6);
            tOut = tIn + this->mTDelta;
            auto fTemp4 = this->mFunction(tOut, xTemp);
            xOut = xIn + sixthTDelta * (
                fTemp1 + C_<T>(2) * (fTemp2 + fTemp3) + fTemp4);
        }

    private:
        friend class UnitTestOdeRungeKutta4;
    };

    template <typename T>
    class OdeRungeKutta4<T> : public OdeSolver<T>
    {
    public:
        // Construction and destruction.
        virtual ~OdeRungeKutta4() = default;

        OdeRungeKutta4(T const& tDelta,
            std::function<Vector<T>(T const&, Vector<T> const&)> const& F)
            :
            OdeSolver<T>(tDelta, F)
        {
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). You may
        // allow xIn and xOut to be the same object.
        virtual void Update(T const& tIn, Vector<T> const& xIn,
            T& tOut, Vector<T>& xOut) override
        {
            // Compute the first step.
            T halfTDelta = C_<T>(1, 2) * this->mTDelta;
            auto fTemp1 = this->mFunction(tIn, xIn);
            auto xTemp = xIn + halfTDelta * fTemp1;

            // Compute the second step.
            T halfT = tIn + halfTDelta;
            auto fTemp2 = this->mFunction(halfT, xTemp);
            xTemp = xIn + halfTDelta * fTemp2;

            // Compute the third step.
            auto fTemp3 = this->mFunction(halfT, xTemp);
            xTemp = xIn + this->mTDelta * fTemp3;

            // Compute the fourth step.
            T sixthTDelta = this->mTDelta / C_<T>(6);
            tOut = tIn + this->mTDelta;
            auto fTemp4 = this->mFunction(tOut, xTemp);
            xOut = xIn + sixthTDelta * (
                fTemp1 + C_<T>(2) * (fTemp2 + fTemp3) + fTemp4);
        }

    private:
        friend class UnitTestOdeRungeKutta4;
    };
}
