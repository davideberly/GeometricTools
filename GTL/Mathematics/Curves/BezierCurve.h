// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

#include <GTL/Mathematics/Curves/ParametricCurve.h>
#include <GTL/Utility/Multiarray.h>

namespace gtl
{
    template <typename T, size_t N>
    class BezierCurve : public ParametricCurve<T, N>
    {
    public:
        // The number of control points must be degree + 1. If the input
        // controls is non-null, a copy is made of the controls. To defer
        // setting the control points, pass a null pointer and later access
        // the control points by using the appropriate member functions. The
        // domain is t in [0,1].
        BezierCurve(size_t degree, Vector<T, N> const* controls)
            :
            ParametricCurve<T, N>(C_<T>(0), C_<T>(1)),
            mDegree(degree),
            mNumControls(degree + 1),
            mControls{},
            mChoose{ mNumControls, mNumControls }
        {
            GTL_ARGUMENT_ASSERT(
                degree >= 2,
                "Invalid degree.");

            // Copy the controls.
            mControls[0].resize(mNumControls);
            if (controls)
            {
                std::copy(controls, controls + mNumControls, mControls[0].begin());
            }
            else
            {
                Vector<T, N> zero{};
                std::fill(mControls[0].begin(), mControls[0].end(), zero);
            }

            ComputeControlPointDifferences();

            // Compute combinatorial values Choose(n,k) and store in
            // mChoose(n, k). The values mChoose(r, c) are invalid for r < c;
            // that is, only the entries for r >= c are accessed.
            mChoose(0, 0) = C_<T>(1);
            mChoose(1, 0) = C_<T>(1);
            mChoose(1, 1) = C_<T>(1);
            for (size_t i = 2; i <= mDegree; ++i)
            {
                mChoose(i, 0) = C_<T>(1);
                mChoose(i, i) = C_<T>(1);
                for (size_t j = 1; j < i; ++j)
                {
                    mChoose(i, j) = mChoose(i - 1, j - 1) + mChoose(i - 1, j);
                }
            }
        }

        virtual ~BezierCurve() = default;

        // Member access.
        inline size_t GetDegree() const
        {
            return mDegree;
        }

        inline size_t GetNumControls() const
        {
            return mNumControls;
        }

        inline Vector<T, N> const* GetControls() const
        {
            return mControls[0].data();
        }

        void SetControl(size_t i, Vector<T, N> const& control)
        {
            GTL_ARGUMENT_ASSERT(
                i < GetNumControls(),
                "Invalid index.");

            mControls[0][i] = control;
        }

        Vector<T, N> const& GetControl(size_t i) const
        {
            GTL_ARGUMENT_ASSERT(
                i < GetNumControls(),
                "Invalid index.");

            return mControls[0][i];
        }

        // Evaluation of the curve. It is required that order <= 3, which
        // allows computing derivatives through order 3. If you want only the
        // position, pass in order of 0. If you want the position and first
        // derivative, pass in order of 1, and so on. The output array 'jet'
        // must have order+1 elements for a specified order. The values are
        // ordered as:
        //   jet[0] contains position X
        //   jet[1] contains first-order derivative dX/dt
        //   jet[2] contains second-order derivative d2X/dt2
        //   jet[3] contains third-order derivative d3X/dt3
        // For order 4 and larger, jet[i] is the zero vector for i >= 4.
        virtual void Evaluate(T const& t, size_t order, Vector<T, N>* jet) const override
        {
            // Compute position.
            T omt = C_<T>(1) - t;
            jet[0] = Compute(t, omt, 0);
            if (order >= 1)
            {
                // Lazy construction and evaluation. The CPU cost occurs only
                // once, on its first call.
                ComputeControlPointDifferences();

                // Compute first derivative.
                jet[1] = Compute(t, omt, 1);
                if (order >= 2)
                {
                    // Compute second derivative.
                    jet[2] = Compute(t, omt, 2);
                    if (order >= 3)
                    {
                        // Compute third derivative.
                        if (mDegree >= 3)
                        {
                            jet[3] = Compute(t, omt, 3);
                        }
                        else
                        {
                            MakeZero(jet[3]);
                        }

                        for (size_t i = 4; i <= order; ++i)
                        {
                            MakeZero(jet[i]);
                        }
                    }
                }
            }
        }

    protected:
        // Support for derivative computations. If the constructor was passed
        // a non-null controls pointer, the control-point differences are
        // computed during the construction. If the controls pointer is null,
        // the control-point differences are computed in the first call of
        // Evaluate().
        void ComputeControlPointDifferences() const
        {
            if (mControls[1].size() > 0)
            {
                // The control-point differences have already been computed.
                return;
            }

            // Compute first-order differences.
            mControls[1].resize(mNumControls - 1);
            for (size_t i = 0; i < mNumControls - 1; ++i)
            {
                mControls[1][i] = mControls[0][i + 1] - mControls[0][i];
            }

            // Compute second-order differences.
            mControls[2].resize(mNumControls - 2);
            for (size_t i = 0; i < mNumControls - 2; ++i)
            {
                mControls[2][i] = mControls[1][i + 1] - mControls[1][i];
            }

            // Compute third-order differences.
            if (mDegree >= 3)
            {
                mControls[3].resize(mNumControls - 3);
                for (size_t i = 0; i < mNumControls - 3; ++i)
                {
                    mControls[3][i] = mControls[2][i + 1] - mControls[2][i];
                }
            }
        }

        // Support for Evaluate(...).
        Vector<T, N> Compute(T const& t, T const& omt, size_t order) const
        {
            Vector<T, N> result = omt * mControls[order][0];

            T tpow = t;
            size_t isup = mDegree - order;
            for (size_t i = 1; i < isup; ++i)
            {
                T c = mChoose(isup, i) * tpow;
                result = (result + c * mControls[order][i]) * omt;
                tpow *= t;
            }
            result = (result + tpow * mControls[order][isup]);

            size_t multiplier = 1;
            for (size_t i = 0; i < order; ++i)
            {
                multiplier *= mDegree - i;
            }
            result *= static_cast<T>(multiplier);

            return result;
        }

        size_t mDegree, mNumControls;
        mutable std::array<std::vector<Vector<T, N>>, 4> mControls;
        Multiarray<T, true> mChoose;
    };
}
