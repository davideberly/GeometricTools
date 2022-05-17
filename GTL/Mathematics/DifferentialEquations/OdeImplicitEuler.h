// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.17

#pragma once

// An implicit Euler's method for numerical approximation of solutions to
// dx/dt = F(t,x), where x(t) is a vector-valued function of a real-valued
// variable t. The numerical method is
//   t[0] and x[0] are user-specified initial conditions
//   t[i+1] = t[i] + h, where h > 0 is the step size
//   x[i+1] = x[i] + h * F(t[i+1], x[i+1]), i >= 0
// This is an implicit equation in x[i+1]. Define
//   G(z) = x[i] + h * F(t[i+1], z) - z
//   G'(z) = h * dF/dx(t[i+1], z) - 1
// where x[i], t[i+1] and h are considered to be constants. Newton's
// method can be used to solve G(z) = 0 with
//   z[0] = x[i], tNext = t[i+1]
//   z[j+1] = z[j] - G(z[j]) / G'(z[j]), j >= 0
//   = z[j] + (z[0] + h * F(tNext, z[j]) - z[j])/(1 - h * dF/dx(tNext, z[j]))
// The number of iterations is a parameter to the constructor.

#include <GTL/Mathematics/DifferentialEquations/OdeSolver.h>
#include <GTL/Mathematics/MatrixAnalysis/GaussianElimination.h>
#include <cstddef>
#include <functional>

namespace gtl
{
    template <typename T, size_t...> class OdeImplicitEuler;

    template <typename T>
    class OdeImplicitEuler<T, 1> : public OdeSolver<T, 1>
    {
    public:
        // Construction and destruction.
        virtual ~OdeImplicitEuler() = default;

        OdeImplicitEuler(T const& tDelta,
            std::function<T(T const&, T const&)> const& F,
            std::function<T(T const&, T const&)> const& DF,
            size_t numNewtonIterations)
            :
            OdeSolver<T, 1>(tDelta, F),
            mDerivativeFunction(DF),
            mNumNewtonIterations(numNewtonIterations)
        {
            GTL_ARGUMENT_ASSERT(
                numNewtonIterations > 0,
                "The number of Newton iterations must be positive.");
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). You may
        // allow xIn and xOut to be the same object.
        virtual void Update(T const& tIn, T const& xIn,
            T& tOut, T& xOut) override
        {
            T z = xIn;  // z[0] = x[i]
            T tNext = tIn + this->mTDelta;  // t[i+1] = t[i] + h

            for (size_t j = 0; j < mNumNewtonIterations; ++j)
            {
                T denom = C_<T>(1) - this->mTDelta * this->mDerivativeFunction(tNext, z);
                if (denom != C_<T>(0))
                {
                    T numer = xIn + this->mTDelta * this->mFunction(tNext, z) - z;
                    if (numer != C_<T>(0))
                    {
                        T update = numer / denom;
                        z += update;  // z[j+1] = z[j] - G(z[j])/G'(z[j])
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            tOut = tNext;  // = t[i + 1]
            xOut = z;  // = x[i+1]
        }

    private:
        std::function<T(T const&, T const&)> mDerivativeFunction;
        size_t mNumNewtonIterations;
    
        friend class UnitTestOdeImplicitEuler;
    };

    template <typename T, size_t N>
    class OdeImplicitEuler<T, N> : public OdeSolver<T, N>
    {
    public:
        // Construction and destruction.
        virtual ~OdeImplicitEuler() = default;

        // The caller is resonsible for ensuring that DF returns an N-by-N
        // matrix when F is an N-by-1 function.
        OdeImplicitEuler(T const& tDelta,
            std::function<Vector<T, N>(T, Vector<T, N> const&)> const& F,
            std::function<Matrix<T, N, N>(T, Vector<T, N> const&)> const& DF,
            size_t numNewtonIterations)
            :
            OdeSolver<T, N>(tDelta, F),
            mDerivativeFunction(DF),
            mNumNewtonIterations(numNewtonIterations)
        {
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). You may
        // allow xIn and xOut to be the same object.
        virtual void Update(T const& tIn, Vector<T, N> const& xIn,
            T& tOut, Vector<T, N>& xOut) override
        {
            T determinant = C_<T>(0);
            Vector<T, N> z = xIn;  // z[0] = x[i]
            T tNext = tIn + this->mTDelta;  // t[i+1] = t[i] + h

            for (size_t j = 0; j < mNumNewtonIterations; ++j)
            {
                auto DG = (-this->mTDelta) * this->mDerivativeFunction(tNext, z);
                for (size_t k = 0; k < N; ++k)
                {
                    DG(k, k) += C_<T>(1);
                }
                auto inverseDG = Inverse(DG, &determinant);
                if (determinant != C_<T>(0))
                {
                    auto numer = xIn + this->mTDelta * this->mFunction(tNext, z) - z;
                    if (!IsZero(numer))
                    {
                        auto update = inverseDG * numer;
                        z += update;  // z[j+1] = z[j] - G(z[j])/G'(z[j])
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            tOut = tNext;  // = t[i+1]
            xOut = z;  // = x[i+1]
        }

    private:
        std::function<Matrix<T, N, N>(T const&, Vector<T, N> const&)> mDerivativeFunction;
        size_t mNumNewtonIterations;

        friend class UnitTestOdeImplicitEuler;
    };

    template <typename T>
    class OdeImplicitEuler<T> : public OdeSolver<T>
    {
    public:
        // Construction and destruction.
        virtual ~OdeImplicitEuler() = default;

        // The caller is resonsible for ensuring that DF returns an N-by-N
        // matrix when F is an N-by-1 function.
        OdeImplicitEuler(T const& tDelta,
            std::function<Vector<T>(T const&, Vector<T> const&)> const& F,
            std::function<Matrix<T>(T const&, Vector<T> const&)> const& DF,
            size_t numNewtonIterations)
            :
            OdeSolver<T>(tDelta, F),
            mDerivativeFunction(DF),
            mNumNewtonIterations(numNewtonIterations)
        {
        }

        // Estimate x(t + tDelta) from x(t) using dx/dt = F(t,x). You may
        // allow xIn and xOut to be the same object.
        virtual void Update(T const& tIn, Vector<T> const& xIn,
            T& tOut, Vector<T>& xOut) override
        {
            T determinant = C_<T>(0);
            Vector<T> z = xIn;  // z[0] = x[i]
            T tNext = tIn + this->mTDelta;  // t[i+1] = t[i] + h

            for (size_t j = 0; j < mNumNewtonIterations; ++j)
            {
                auto DG = (-this->mTDelta) * this->mDerivativeFunction(tNext, z);
                for (size_t k = 0; k < xIn.size(); ++k)
                {
                    DG(k, k) += C_<T>(1);
                }
                auto inverseDG = Inverse(DG, &determinant);
                if (determinant != C_<T>(0))
                {
                    auto numer = xIn + this->mTDelta * this->mFunction(tNext, z) - z;
                    if (!IsZero(numer))
                    {
                        auto update = inverseDG * numer;
                        z += update;  // z[j+1] = z[j] - G(z[j])/G'(z[j])
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            tOut = tNext;  // = t[i+1]
            xOut = z;  // = x[i+1]
        }

    private:
        std::function<Matrix<T>(T const&, Vector<T> const&)> mDerivativeFunction;
        size_t mNumNewtonIterations;

        friend class UnitTestOdeImplicitEuler;
    };
}
