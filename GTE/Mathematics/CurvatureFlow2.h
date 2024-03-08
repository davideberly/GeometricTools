// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/PdeFilter2.h>
#include <cstdint>

namespace gte
{
    template <typename Real>
    class CurvatureFlow2 : public PdeFilter2<Real>
    {
    public:
        CurvatureFlow2(int32_t xBound, int32_t yBound, Real xSpacing,
            Real ySpacing, Real const* data, int32_t const* mask,
            Real borderValue, typename PdeFilter<Real>::ScaleType scaleType)
            :
            PdeFilter2<Real>(xBound, yBound, xSpacing, ySpacing, data, mask,
                borderValue, scaleType)
        {
        }

        virtual ~CurvatureFlow2()
        {
        }

    protected:
        virtual void OnUpdateSingle(int32_t x, int32_t y) override
        {
            this->LookUp9(x, y);

            Real ux = this->mHalfInvDx * (this->mUpz - this->mUmz);
            Real uy = this->mHalfInvDy * (this->mUzp - this->mUzm);
            Real uxx = this->mInvDxDx * (this->mUpz - (Real)2 * this->mUzz + this->mUmz);
            Real uxy = this->mFourthInvDxDy * (this->mUmm + this->mUpp - this->mUmp - this->mUpm);
            Real uyy = this->mInvDyDy * (this->mUzp - (Real)2 * this->mUzz + this->mUzm);

            Real sqrUx = ux * ux;
            Real sqrUy = uy * uy;
            Real denom = sqrUx + sqrUy;
            if (denom > (Real)0)
            {
                Real numer = uxx * sqrUy + uyy * sqrUx - (Real)0.5 * uxy * ux * uy;
                this->mBuffer[this->mDst][y][x] = this->mUzz + this->mTimeStep * numer / denom;
            }
            else
            {
                this->mBuffer[this->mDst][y][x] = this->mUzz;
            }
        }
    };
}
