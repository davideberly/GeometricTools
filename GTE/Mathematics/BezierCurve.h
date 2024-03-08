// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Array2.h>
#include <Mathematics/ParametricCurve.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <int32_t N, typename Real>
    class BezierCurve : public ParametricCurve<N, Real>
    {
    public:
        // Construction and destruction.  The number of control points must be
        // degree + 1.  This object copies the input array.  The domain is t
        // in [0,1].  To validate construction, create an object as shown:
        //     BezierCurve<N, Real> curve(parameters);
        //     if (!curve) { <constructor failed, handle accordingly>; }
        BezierCurve(int32_t degree, Vector<N, Real> const* controls)
            :
            ParametricCurve<N, Real>((Real)0, (Real)1),
            mDegree(degree),
            mNumControls(degree + 1),
            mChoose(mNumControls, mNumControls)
        {
            LogAssert(degree >= 2 && controls != nullptr, "Invalid input.");

            // Copy the controls.
            mControls[0].resize(mNumControls);
            std::copy(controls, controls + mNumControls, mControls[0].begin());

            // Compute first-order differences.
            mControls[1].resize(static_cast<size_t>(mNumControls) - 1);
            for (int32_t i = 0, ip1 = 1; ip1 < mNumControls; ++i, ++ip1)
            {
                mControls[1][i] = mControls[0][ip1] - mControls[0][i];
            }

            // Compute second-order differences.
            mControls[2].resize(static_cast<size_t>(mNumControls) - 2);
            for (int32_t i = 0, ip1 = 1, ip2 = 2; ip2 < mNumControls; ++i, ++ip1, ++ip2)
            {
                mControls[2][i] = mControls[1][ip1] - mControls[1][i];
            }

            // Compute third-order differences.
            if (degree >= 3)
            {
                mControls[3].resize(static_cast<size_t>(mNumControls) - 3);
                for (int32_t i = 0, ip1 = 1, ip3 = 3; ip3 < mNumControls; ++i, ++ip1, ++ip3)
                {
                    mControls[3][i] = mControls[2][ip1] - mControls[2][i];
                }
            }

            // Compute combinatorial values Choose(n,k) and store in mChoose[n][k].
            // The values mChoose[r][c] are invalid for r < c; that is, we use only
            // the entries for r >= c.
            mChoose[0][0] = (Real)1;
            mChoose[1][0] = (Real)1;
            mChoose[1][1] = (Real)1;
            for (int32_t i = 2; i <= mDegree; ++i)
            {
                mChoose[i][0] = (Real)1;
                mChoose[i][i] = (Real)1;
                for (int32_t j = 1; j < i; ++j)
                {
                    mChoose[i][j] = mChoose[i - 1][j - 1] + mChoose[i - 1][j];
                }
            }

            this->mConstructed = true;
        }

        virtual ~BezierCurve()
        {
        }

        // Member access.
        inline int32_t GetDegree() const
        {
            return mDegree;
        }

        inline int32_t GetNumControls() const
        {
            return mNumControls;
        }

        inline Vector<N, Real> const* GetControls() const
        {
            return &mControls[0][0];
        }

        // Evaluation of the curve.  The function supports derivative
        // calculation through order 3; that is, order <= 3 is required.  If
        // you want/ only the position, pass in order of 0.  If you want the
        // position and first derivative, pass in order of 1, and so on.  The
        // output array 'jet' must have enough storage to support the maximum
        // order.  The values are ordered as: position, first derivative,
        // second derivative, third derivative.
        virtual void Evaluate(Real t, uint32_t order, Vector<N, Real>* jet) const override
        {
            uint32_t const supOrder = ParametricCurve<N, Real>::SUP_ORDER;
            if (!this->mConstructed || order >= supOrder)
            {
                // Return a zero-valued jet for invalid state.
                for (uint32_t i = 0; i < supOrder; ++i)
                {
                    jet[i].MakeZero();
                }
                return;
            }

            // Compute position.
            Real omt = (Real)1 - t;
            jet[0] = Compute(t, omt, 0);
            if (order >= 1)
            {
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
                            jet[3].MakeZero();
                        }
                    }
                }
            }
        }

    protected:
        // Support for Evaluate(...).
        Vector<N, Real> Compute(Real t, Real omt, int32_t order) const
        {
            Vector<N, Real> result = omt * mControls[order][0];

            Real tpow = t;
            int32_t isup = mDegree - order;
            for (int32_t i = 1; i < isup; ++i)
            {
                Real c = mChoose[isup][i] * tpow;
                result = (result + c * mControls[order][i]) * omt;
                tpow *= t;
            }
            result = (result + tpow * mControls[order][isup]);

            int32_t multiplier = 1;
            for (int32_t i = 0; i < order; ++i)
            {
                multiplier *= mDegree - i;
            }
            result *= (Real)multiplier;

            return result;
        }

        int32_t mDegree, mNumControls;
        std::array<std::vector<Vector<N, Real>>, ParametricCurve<N, Real>::SUP_ORDER> mControls;
        Array2<Real> mChoose;
    };
}
