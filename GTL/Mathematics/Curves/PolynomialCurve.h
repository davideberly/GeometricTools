// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

#include <GTL/Mathematics/Curves/ParametricCurve.h>
#include <GTL/Mathematics/Algebra/Polynomial.h>

namespace gtl
{
    template <typename T, size_t N>
    class PolynomialCurve : public ParametricCurve<T, N>
    {
    public:
        // Construction and destruction.  The default constructor creates a
        // polynomial curve with all components set to the constant zero (all
        // degree-0 polynomials).  You can set these to other polynomials
        // using member accessors.
        PolynomialCurve(T const& tmin, T const& tmax)
            :
            ParametricCurve<T, N>(tmin, tmax)
        {
        }

        PolynomialCurve(T const& tmin, T const& tmax,
            std::array<Polynomial1<T>, N> const& components)
            :
            ParametricCurve<T, N>(tmin, tmax)
        {
            for (size_t i = 0; i < N; ++i)
            {
                SetPolynomial(i, components[i]);
            }
        }

        virtual ~PolynomialCurve() = default;


        // Member access.
        void SetPolynomial(size_t i, Polynomial1<T> const& poly)
        {
            mPolynomial[i] = poly;
            mDer1Polynomial[i] = GetDerivative(mPolynomial[i]);
            mDer2Polynomial[i] = GetDerivative(mDer1Polynomial[i]);
            mDer3Polynomial[i] = GetDerivative(mDer2Polynomial[i]);
        }

        inline Polynomial1<T> const& GetPolynomial(size_t i) const
        {
            return mPolynomial[i];
        }

        inline Polynomial1<T> const& GetDer1Polynomial(size_t i) const
        {
            return mDer1Polynomial[i];
        }

        inline Polynomial1<T> const& GetDer2Polynomial(size_t i) const
        {
            return mDer2Polynomial[i];
        }

        inline Polynomial1<T> const& GetDer3Polynomial(size_t i) const
        {
            return mDer3Polynomial[i];
        }

        // Evaluation of the curve. It is required that order <= 3, which
        // allows computing derivatives through order 3. If you want only the
        // position, pass in order of 0. If you want the position and first
        // derivative, pass in order of 1, and so on. The output array 'jet'
        // must have enough storage to support the specified order. The values
        // are ordered as: position, first derivative, second derivative, and
        // so on.
        virtual void Evaluate(T const& t, size_t order, Vector<T, N>* jet) const override
        {
            for (size_t i = 0; i < N; ++i)
            {
                jet[0][i] = mPolynomial[i](t);
            }

            if (order >= 1)
            {
                for (size_t i = 0; i < N; ++i)
                {
                    jet[1][i] = mDer1Polynomial[i](t);
                }

                if (order >= 2)
                {
                    for (size_t i = 0; i < N; ++i)
                    {
                        jet[2][i] = mDer2Polynomial[i](t);
                    }

                    if (order >= 3)
                    {
                        for (size_t i = 0; i < N; ++i)
                        {
                            jet[3][i] = mDer3Polynomial[i](t);
                        }

                        // Derivatives of order 4 and higher are not yet
                        // supported. If you need them, derive a class from
                        // this one and implement Evaluate(...) to handle the
                        // larger orders.
                        for (size_t i = 4; i <= order; ++i)
                        {
                            MakeZero(jet[i]);
                        }
                    }
                }
            }
        }

    protected:
        std::array<Polynomial1<T>, N> mPolynomial;
        std::array<Polynomial1<T>, N> mDer1Polynomial;
        std::array<Polynomial1<T>, N> mDer2Polynomial;
        std::array<Polynomial1<T>, N> mDer3Polynomial;
    };
}
