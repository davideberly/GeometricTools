// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The BSplineReduction class is an implementation of the algorithm in
// https://www.geometrictools.com/Documentation/BSplineReduction.pdf
// for least-squares fitting of points in the continuous sense by
// an L2 integral norm.  The least-squares fitting implemented in the
// file BSplineCurveFit.h is in the discrete sense by an L2 summation.
// The intended use for this class is to take an open B-spline curve,
// defined by its control points and degree, and reducing the number of
// control points dramatically to obtain another curve that is close to
// the original one.

#include <Mathematics/BasisFunction.h>
#include <Mathematics/ParametricSurface.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    template <int32_t N, typename Real>
    class BSplineSurface : public ParametricSurface<N, Real>
    {
    public:
        // Construction.  If the input controls is non-null, a copy is made of
        // the controls.  To defer setting the control points, pass a null
        // pointer and later access the control points via GetControls() or
        // SetControl() member functions.  The input 'controls' must be stored
        // in row-major order, control[i0 + numControls0*i1].  As a 2D array,
        // this corresponds to control2D[i1][i0].
        BSplineSurface(BasisFunctionInput<Real> const input[2], Vector<N, Real> const* controls)
            :
            ParametricSurface<N, Real>((Real)0, (Real)1, (Real)0, (Real)1, true)
        {
            for (int32_t i = 0; i < 2; ++i)
            {
                mNumControls[i] = input[i].numControls;
                mBasisFunction[i].Create(input[i]);
            }

            // The mBasisFunction stores the domain but so does
            // ParametricCurve.
            this->mUMin = mBasisFunction[0].GetMinDomain();
            this->mUMax = mBasisFunction[0].GetMaxDomain();
            this->mVMin = mBasisFunction[1].GetMinDomain();
            this->mVMax = mBasisFunction[1].GetMaxDomain();

            // The replication of control points for periodic splines is
            // avoided by wrapping the i-loop index in Evaluate.
            int32_t numControls = mNumControls[0] * mNumControls[1];
            mControls.resize(numControls);
            if (controls)
            {
                std::copy(controls, controls + numControls, mControls.begin());
            }
            else
            {
                Vector<N, Real> zero{ (Real)0 };
                std::fill(mControls.begin(), mControls.end(), zero);
            }
            this->mConstructed = true;
        }

        // Member access.  The index 'dim' must be in {0,1}.
        inline BasisFunction<Real> const& GetBasisFunction(int32_t dim) const
        {
            return mBasisFunction[dim];
        }

        inline int32_t GetNumControls(int32_t dim) const
        {
            return mNumControls[dim];
        }

        inline Vector<N, Real>* GetControls()
        {
            return mControls.data();
        }

        inline Vector<N, Real> const* GetControls() const
        {
            return mControls.data();
        }

        void SetControl(int32_t i0, int32_t i1, Vector<N, Real> const& control)
        {
            if (0 <= i0 && i0 < GetNumControls(0)
                && 0 <= i1 && i1 < GetNumControls(1))
            {
                mControls[i0 + static_cast<size_t>(mNumControls[0]) * i1] = control;
            }
        }

        Vector<N, Real> const& GetControl(int32_t i0, int32_t i1) const
        {
            if (0 <= i0 && i0 < GetNumControls(0) && 0 <= i1 && i1 < GetNumControls(1))
            {
                return mControls[i0 + static_cast<size_t>(mNumControls[0]) * i1];
            }
            else
            {
                return mControls[0];
            }
        }

        // Evaluation of the surface.  The function supports derivative
        // calculation through order 2; that is, order <= 2 is required.  If
        // you want only the position, pass in order of 0.  If you want the
        // position and first-order derivatives, pass in order of 1, and so
        // on.  The output array 'jet' must have enough storage to support the
        // maximum order.  The values are ordered as: position X; first-order
        // derivatives dX/du, dX/dv; second-order derivatives d2X/du2,
        // d2X/dudv, d2X/dv2.
        virtual void Evaluate(Real u, Real v, uint32_t order, Vector<N, Real>* jet) const override
        {
            uint32_t const supOrder = ParametricSurface<N, Real>::SUP_ORDER;
            if (!this->mConstructed || order >= supOrder)
            {
                // Return a zero-valued jet for invalid state.
                for (uint32_t i = 0; i < supOrder; ++i)
                {
                    jet[i].MakeZero();
                }
                return;
            }

            int32_t iumin{}, iumax{}, ivmin{}, ivmax{};
            mBasisFunction[0].Evaluate(u, order, iumin, iumax);
            mBasisFunction[1].Evaluate(v, order, ivmin, ivmax);

            // Compute position.
            jet[0] = Compute(0, 0, iumin, iumax, ivmin, ivmax);
            if (order >= 1)
            {
                // Compute first-order derivatives.
                jet[1] = Compute(1, 0, iumin, iumax, ivmin, ivmax);
                jet[2] = Compute(0, 1, iumin, iumax, ivmin, ivmax);
                if (order >= 2)
                {
                    // Compute second-order derivatives.
                    jet[3] = Compute(2, 0, iumin, iumax, ivmin, ivmax);
                    jet[4] = Compute(1, 1, iumin, iumax, ivmin, ivmax);
                    jet[5] = Compute(0, 2, iumin, iumax, ivmin, ivmax);
                }
            }
        }

    private:
        // Support for Evaluate(...).
        Vector<N, Real> Compute(uint32_t uOrder, uint32_t vOrder,
            int32_t iumin, int32_t iumax, int32_t ivmin, int32_t ivmax) const
        {
            // The j*-indices introduce a tiny amount of overhead in order to
            // handle both aperiodic and periodic splines.  For aperiodic
            // splines, j* = i* always.

            int32_t const numControls0 = mNumControls[0];
            int32_t const numControls1 = mNumControls[1];
            Vector<N, Real> result{};
            result.MakeZero();
            for (int32_t iv = ivmin; iv <= ivmax; ++iv)
            {
                Real tmpv = mBasisFunction[1].GetValue(vOrder, iv);
                int32_t jv = (iv >= numControls1 ? iv - numControls1 : iv);
                for (int32_t iu = iumin; iu <= iumax; ++iu)
                {
                    Real tmpu = mBasisFunction[0].GetValue(uOrder, iu);
                    int32_t ju = (iu >= numControls0 ? iu - numControls0 : iu);
                    result += (tmpu * tmpv) * mControls[ju + static_cast<size_t>(numControls0) * jv];
                }
            }
            return result;
        }

        std::array<BasisFunction<Real>, 2> mBasisFunction;
        std::array<int32_t, 2> mNumControls;
        std::vector<Vector<N, Real>> mControls;
    };
}
