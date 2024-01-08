// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/PdeFilter3.h>
#include <cstdint>

namespace gte
{
    template <typename Real>
    class GaussianBlur3 : public PdeFilter3<Real>
    {
    public:
        GaussianBlur3(int32_t xBound, int32_t yBound, int32_t zBound, Real xSpacing,
            Real ySpacing, Real zSpacing, Real const* data, int32_t const* mask,
            Real borderValue, typename PdeFilter<Real>::ScaleType scaleType)
            :
            PdeFilter3<Real>(xBound, yBound, zBound, xSpacing, ySpacing, zSpacing,
                data, mask, borderValue, scaleType)
        {
            mMaximumTimeStep = (Real)0.5 / (this->mInvDxDx + this->mInvDyDy + this->mInvDzDz);
        }

        virtual ~GaussianBlur3()
        {
        }

        inline Real GetMaximumTimeStep() const
        {
            return mMaximumTimeStep;
        }

    protected:
        virtual void OnUpdateSingle(int32_t x, int32_t y, int32_t z) override
        {
            this->LookUp7(x, y, z);

            Real uxx = this->mInvDxDx * (this->mUpzz - (Real)2 * this->mUzzz + this->mUmzz);
            Real uyy = this->mInvDyDy * (this->mUzpz - (Real)2 * this->mUzzz + this->mUzmz);
            Real uzz = this->mInvDzDz * (this->mUzzp - (Real)2 * this->mUzzz + this->mUzzm);

            this->mBuffer[this->mDst][z][y][x] = this->mUzzz + this->mTimeStep * (uxx + uyy + uzz);
        }

        Real mMaximumTimeStep;
    };
}
