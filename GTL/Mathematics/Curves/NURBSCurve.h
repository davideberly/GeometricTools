// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

#include <GTL/Mathematics/Curves/ParametricCurve.h>
#include <GTL/Mathematics/Curves/BasisFunction.h>

namespace gtl
{
    template <typename T, size_t N>
    class NURBSCurve : public ParametricCurve<T, N>
    {
    public:
        // If the input controls is non-null, a copy is made of the controls.
        // To defer setting the control points or weights, pass null pointers
        // and later access the control points or weights by using the
        // appropriate member functions. The domain is t in [t[d],t[n]], where
        // t[d] and t[n] are knots with d the degree and n the number of
        // control points.
        NURBSCurve(
            typename BasisFunction<T>::Input const& input,
            Vector<T, N> const* controls, T const* weights)
            :
            ParametricCurve<T, N>(C_<T>(0), C_<T>(1)),
            mBasisFunction(input)
        {
            // The mBasisFunction stores the domain, but copies are stored in
            // ParametricCurve.
            this->mTime.front() = mBasisFunction.GetMinDomain();
            this->mTime.back() = mBasisFunction.GetMaxDomain();

            // The replication of control points for periodic splines is
            // avoided by wrapping the i-loop index in Evaluate.
            mControls.resize(input.numControls);
            mWeights.resize(input.numControls);
            if (controls)
            {
                std::copy(controls, controls + input.numControls, mControls.begin());
            }
            else
            {
                Vector<T, N> zero{};
                std::fill(mControls.begin(), mControls.end(), zero);
            }
            if (weights)
            {
                std::copy(weights, weights + input.numControls, mWeights.begin());
            }
            else
            {
                std::fill(mWeights.begin(), mWeights.end(), C_<T>(0));
            }
        }

        // Member access.
        inline BasisFunction<T> const& GetBasisFunction() const
        {
            return mBasisFunction;
        }

        inline size_t GetNumControls() const
        {
            return mControls.size();
        }

        inline Vector<T, N> const* GetControls() const
        {
            return mControls.data();
        }

        inline Vector<T, N>* GetControls()
        {
            return mControls.data();
        }

        inline T const* GetWeights() const
        {
            return mWeights.data();
        }

        inline T* GetWeights()
        {
            return mWeights.data();
        }

        void SetControl(size_t i, Vector<T, N> const& control)
        {
            GTL_ARGUMENT_ASSERT(
                i < GetNumControls(),
                "Invalid index.");

            mControls[i] = control;
        }

        Vector<T, N> const& GetControl(size_t i) const
        {
            GTL_ARGUMENT_ASSERT(
                i < GetNumControls(),
                "Invalid index.");

            return mControls[i];
        }

        void SetWeight(size_t i, T weight)
        {
            GTL_ARGUMENT_ASSERT(
                i < GetNumControls(),
                "Invalid index.");
            
            mWeights[i] = weight;
        }

        T const& GetWeight(size_t i) const
        {
            GTL_ARGUMENT_ASSERT(
                i < GetNumControls(),
                "Invalid index.");

            return mWeights[i];
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
            size_t imin = 0, imax = 0;
            mBasisFunction.Evaluate(t, order, imin, imax);

            // Compute position.
            Vector<T, N> X{};
            T w;
            Compute(0, imin, imax, X, w);
            T invW = C_<T>(1) / w;
            jet[0] = invW * X;

            if (order >= 1)
            {
                // Compute first derivative.
                Vector<T, N> XDer1{};
                T wDer1;
                Compute(1, imin, imax, XDer1, wDer1);
                jet[1] = invW * (XDer1 - wDer1 * jet[0]);

                if (order >= 2)
                {
                    // Compute second derivative.
                    Vector<T, N> XDer2{};
                    T wDer2;
                    Compute(2, imin, imax, XDer2, wDer2);
                    jet[2] = invW * (XDer2 - C_<T>(2) * wDer1 * jet[1] - wDer2 * jet[0]);

                    if (order == 3)
                    {
                        // Compute third derivative.
                        Vector<T, N> XDer3{};
                        T wDer3;
                        Compute(3, imin, imax, XDer3, wDer3);
                        jet[3] = invW * (XDer3 - C_<T>(3) * wDer1 * jet[2] -
                            C_<T>(3) * wDer2 * jet[1] - wDer3 * jet[0]);
                    }
                }
            }
        }

    protected:
        // Support for Evaluate(...).
        void Compute(size_t order, size_t imin, size_t imax, Vector<T, N>& X, T& w) const
        {
            // The j-index introduces a tiny amount of overhead in order to
            // handle both aperiodic and periodic splines.  For aperiodic
            // splines, j = i always.

            size_t const numControls = GetNumControls();
            MakeZero(X);
            w = C_<T>(0);
            for (size_t i = imin; i <= imax; ++i)
            {
                size_t j = (i >= numControls ? i - numControls : i);
                T tmp = mBasisFunction.GetValue(order, i) * mWeights[j];
                X += tmp * mControls[j];
                w += tmp;
            }
        }

        BasisFunction<T> mBasisFunction;
        std::vector<Vector<T, N>> mControls;
        std::vector<T> mWeights;
    };
}
