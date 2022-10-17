// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

#include <GTL/Mathematics/Surfaces/ParametricSurface.h>
#include <GTL/Mathematics/Curves/BasisFunction.h>

namespace gtl
{
    template <typename T, size_t N>
    class NURBSSurface : public ParametricSurface<T, N>
    {
    public:
        // Construction. If the input controls is non-null, a copy is made of
        // the controls. To defer setting the control points or weights, pass
        // null pointers and later access the control points or weights via
        // GetControls(), GetWeights(), SetControl(), or SetWeight() member
        // functions. The 'controls' and 'weights' must be stored in
        // row-major order, attribute[i0 + numControls0*i1]. As a 2D array,
        // this corresponds to attribute2D[i1][i0].
        NURBSSurface(
            std::array<typename BasisFunction<T>::Input, 2> const& input,
            Vector<T, N> const* controls, T const* weights)
            :
            ParametricSurface<T, N>(
                C_<T>(0), C_<T>(1),
                C_<T>(0), C_<T>(1),
                true)
        {
            for (size_t i = 0; i < 2; ++i)
            {
                mNumControls[i] = input[i].numControls;
                mBasisFunction[i].Create(input[i]);
            }

            // The mBasisFunction stores the domain, but copies are stored in
            // ParametricSurface.
            this->mUMin = mBasisFunction[0].GetMinDomain();
            this->mUMax = mBasisFunction[0].GetMaxDomain();
            this->mVMin = mBasisFunction[1].GetMinDomain();
            this->mVMax = mBasisFunction[1].GetMaxDomain();

            // The replication of control points for periodic splines is
            // avoided by wrapping the i-loop index in Evaluate.
            size_t numControls = mNumControls[0] * mNumControls[1];
            mControls.resize(numControls);
            mWeights.resize(numControls);
            if (controls)
            {
                std::copy(controls, controls + numControls, mControls.begin());
            }
            else
            {
                Vector<T, N> zero{};
                std::fill(mControls.begin(), mControls.end(), zero);
            }
            if (weights)
            {
                std::copy(weights, weights + numControls, mWeights.begin());
            }
            else
            {
                std::fill(mWeights.begin(), mWeights.end(), C_<T>(0));
            }
        }

        // Member access. The index 'dim' must be in {0,1}.
        BasisFunction<T> const& GetBasisFunction(size_t dim) const
        {
            GTL_ARGUMENT_ASSERT(
                dim <= 1,
                "Invalid dimension.");

            return mBasisFunction[dim];
        }

        size_t GetNumControls(size_t dim) const
        {
            GTL_ARGUMENT_ASSERT(
                dim <= 1,
                "Invalid dimension.");

            return mNumControls[dim];
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

        void SetControl(size_t i0, size_t i1, Vector<T, N> const& control)
        {
            GTL_ARGUMENT_ASSERT(
                i0 < GetNumControls(0) && i1 < GetNumControls(1),
                "Invalid index.");

            mControls[i0 + mNumControls[0] * i1] = control;
        }

        Vector<T, N> const& GetControl(size_t i0, size_t i1) const
        {
            GTL_ARGUMENT_ASSERT(
                i0 < GetNumControls(0) && i1 < GetNumControls(1),
                "Invalid index.");
            
            return mControls[i0 + mNumControls[0] * i1];
        }

        void SetWeight(size_t i0, size_t i1, T const& weight)
        {
            GTL_ARGUMENT_ASSERT(
                i0 < GetNumControls(0) && i1 < GetNumControls(1),
                "Invalid index.");
            
            mWeights[i0 + mNumControls[0] * i1] = weight;
        }

        T const& GetWeight(size_t i0, size_t i1) const
        {
            GTL_ARGUMENT_ASSERT(
                i0 < GetNumControls(0) && i1 < GetNumControls(1),
                "Invalid index.");
            
            return mWeights[i0 + mNumControls[0] * i1];
        }

        // Evaluation of the surface. It is required that order <= 2, which
        // allows computing derivatives through order 2. If you want only the
        // position, pass in order of 0. If you want the position and first
        // derivatives, pass in order of 1, and so on. The output array 'jet'
        // must have enough storage to support the specified order. The values
        // are ordered as:
        //   jet[0] contains position X
        //   jet[1] contains first-order derivative dX/du
        //   jet[2] contains first-order derivative dX/dv
        //   jet[3] contains second-order derivative d2X/du2
        //   jet[4] contains second-order derivative d2X/dudv
        //   jet[5] contains second-order derivative d2X/dv2
        // and so on.
        virtual void Evaluate(T const& u, T const& v, size_t order, Vector<T, N>* jet) const override
        {
            size_t iumin = 0, iumax = 0, ivmin = 0, ivmax = 0;
            mBasisFunction[0].Evaluate(u, order, iumin, iumax);
            mBasisFunction[1].Evaluate(v, order, ivmin, ivmax);

            // Compute position.
            Vector<T, N> X{};
            T w = C_<T>(0);
            Compute(0, 0, iumin, iumax, ivmin, ivmax, X, w);
            T invW = C_<T>(1) / w;
            jet[0] = invW * X;

            if (order >= 1)
            {
                // Compute first-order derivatives.
                Vector<T, N> XDerU{};
                T wDerU = C_<T>(0);
                Compute(1, 0, iumin, iumax, ivmin, ivmax, XDerU, wDerU);
                jet[1] = invW * (XDerU - wDerU * jet[0]);

                Vector<T, N> XDerV{};
                T wDerV = C_<T>(0);
                Compute(0, 1, iumin, iumax, ivmin, ivmax, XDerV, wDerV);
                jet[2] = invW * (XDerV - wDerV * jet[0]);

                if (order >= 2)
                {
                    // Compute second-order derivatives.
                    Vector<T, N> XDerUU{};
                    T wDerUU = C_<T>(0);
                    Compute(2, 0, iumin, iumax, ivmin, ivmax, XDerUU, wDerUU);
                    jet[3] = invW * (XDerUU - C_<T>(2) * wDerU * jet[1] - wDerUU * jet[0]);

                    Vector<T, N> XDerUV{};
                    T wDerUV = C_<T>(0);
                    Compute(1, 1, iumin, iumax, ivmin, ivmax, XDerUV, wDerUV);
                    jet[4] = invW * (XDerUV - wDerU * jet[2] - wDerV * jet[1]
                        - wDerUV * jet[0]);

                    Vector<T, N> XDerVV{};
                    T wDerVV = C_<T>(0);
                    Compute(0, 2, iumin, iumax, ivmin, ivmax, XDerVV, wDerVV);
                    jet[5] = invW * (XDerVV - C_<T>(2) * wDerV * jet[2] - wDerVV * jet[0]);
                }
            }
        }

    protected:
        // Support for Evaluate(...).
        void Compute(size_t uOrder, size_t vOrder, size_t iumin, size_t iumax,
            size_t ivmin, size_t ivmax, Vector<T, N>& X, T& w) const
        {
            // The j*-indices introduce a tiny amount of overhead in order to handle
            // both aperiodic and periodic splines.  For aperiodic splines, j* = i*
            // always.

            size_t const numControls0 = mNumControls[0];
            size_t const numControls1 = mNumControls[1];
            MakeZero(X);
            w = C_<T>(0);
            for (size_t iv = ivmin; iv <= ivmax; ++iv)
            {
                T tmpv = mBasisFunction[1].GetValue(vOrder, iv);
                size_t jv = (iv >= numControls1 ? iv - numControls1 : iv);
                for (size_t iu = iumin; iu <= iumax; ++iu)
                {
                    T tmpu = mBasisFunction[0].GetValue(uOrder, iu);
                    size_t ju = (iu >= numControls0 ? iu - numControls0 : iu);
                    size_t index = ju + numControls0 * jv;
                    T tmp = tmpu * tmpv * mWeights[index];
                    X += tmp * mControls[index];
                    w += tmp;
                }
            }
        }

        std::array<BasisFunction<T>, 2> mBasisFunction;
        std::array<size_t, 2> mNumControls;
        std::vector<Vector<T, N>> mControls;
        std::vector<T> mWeights;
    };
}
