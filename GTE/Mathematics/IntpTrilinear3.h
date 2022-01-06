// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Logger.h>
#include <array>

// The interpolator is for uniformly spaced(x,y z)-values.  The input samples
// must be stored in lexicographical order to represent f(x,y,z); that is,
// F[c + xBound*(r + yBound*s)] corresponds to f(x,y,z), where c is the index
// corresponding to x, r is the index corresponding to y, and s is the index
// corresponding to z.

namespace gte
{
    template <typename Real>
    class IntpTrilinear3
    {
    public:
        // Construction.
        IntpTrilinear3(int32_t xBound, int32_t yBound, int32_t zBound, Real xMin,
            Real xSpacing, Real yMin, Real ySpacing, Real zMin, Real zSpacing, Real const* F)
            :
            mXBound(xBound),
            mYBound(yBound),
            mZBound(zBound),
            mQuantity(xBound* yBound* zBound),
            mXMin(xMin),
            mXSpacing(xSpacing),
            mYMin(yMin),
            mYSpacing(ySpacing),
            mZMin(zMin),
            mZSpacing(zSpacing),
            mF(F)
        {
            // At least a 2x2x2 block of data points are needed to construct
            // the trilinear interpolation.
            LogAssert(xBound >= 2 && yBound >= 2 && zBound >= 2 && F != nullptr
                && xSpacing > (Real)0 && ySpacing > (Real)0 && zSpacing > (Real)0,
                "Invalid input.");

            mXMax = mXMin + mXSpacing * static_cast<Real>(mXBound) - static_cast<Real>(1);
            mInvXSpacing = (Real)1 / mXSpacing;
            mYMax = mYMin + mYSpacing * static_cast<Real>(mYBound) - static_cast<Real>(1);
            mInvYSpacing = (Real)1 / mYSpacing;
            mZMax = mZMin + mZSpacing * static_cast<Real>(mZBound) - static_cast<Real>(1);
            mInvZSpacing = (Real)1 / mZSpacing;

            mBlend[0][0] = (Real)1;
            mBlend[0][1] = (Real)-1;
            mBlend[1][0] = (Real)0;
            mBlend[1][1] = (Real)1;
        }

        // Member access.
        inline int32_t GetXBound() const
        {
            return mXBound;
        }

        inline int32_t GetYBound() const
        {
            return mYBound;
        }

        inline int32_t GetZBound() const
        {
            return mZBound;
        }

        inline int32_t GetQuantity() const
        {
            return mQuantity;
        }

        inline Real const* GetF() const
        {
            return mF;
        }

        inline Real GetXMin() const
        {
            return mXMin;
        }

        inline Real GetXMax() const
        {
            return mXMax;
        }

        inline Real GetXSpacing() const
        {
            return mXSpacing;
        }

        inline Real GetYMin() const
        {
            return mYMin;
        }

        inline Real GetYMax() const
        {
            return mYMax;
        }

        inline Real GetYSpacing() const
        {
            return mYSpacing;
        }

        inline Real GetZMin() const
        {
            return mZMin;
        }

        inline Real GetZMax() const
        {
            return mZMax;
        }

        inline Real GetZSpacing() const
        {
            return mZSpacing;
        }

        // Evaluate the function and its derivatives.  The functions clamp the
        // inputs to xmin <= x <= xmax, ymin <= y <= ymax and
        // zmin <= z <= zmax.  The first operator is for function evaluation.
        // The second operator is for function or derivative evaluations.  The
        // xOrder argument is the order of the x-derivative, the yOrder
        // argument is the order of the y-derivative, and the zOrder argument
        // is the order of the z-derivative.  All orders are zero to get the
        // function value itself.
        Real operator()(Real x, Real y, Real z) const
        {
            // Compute x-index and clamp to image.
            Real xIndex = (x - mXMin) * mInvXSpacing;
            int32_t ix = static_cast<int32_t>(xIndex);
            if (ix < 0)
            {
                ix = 0;
            }
            else if (ix >= mXBound)
            {
                ix = mXBound - 1;
            }

            // Compute y-index and clamp to image.
            Real yIndex = (y - mYMin) * mInvYSpacing;
            int32_t iy = static_cast<int32_t>(yIndex);
            if (iy < 0)
            {
                iy = 0;
            }
            else if (iy >= mYBound)
            {
                iy = mYBound - 1;
            }

            // Compute z-index and clamp to image.
            Real zIndex = (z - mZMin) * mInvZSpacing;
            int32_t iz = static_cast<int32_t>(zIndex);
            if (iz < 0)
            {
                iz = 0;
            }
            else if (iz >= mZBound)
            {
                iz = mZBound - 1;
            }

            std::array<Real, 2> U;
            U[0] = (Real)1;
            U[1] = xIndex - ix;

            std::array<Real, 2> V;
            V[0] = (Real)1;
            V[1] = yIndex - iy;

            std::array<Real, 2> W;
            W[0] = (Real)1;
            W[1] = zIndex - iz;

            // Compute P = M*U, Q = M*V, R = M*W.
            std::array<Real, 2> P, Q, R;
            for (int32_t row = 0; row < 2; ++row)
            {
                P[row] = (Real)0;
                Q[row] = (Real)0;
                R[row] = (Real)0;
                for (int32_t col = 0; col < 2; ++col)
                {
                    P[row] += mBlend[row][col] * U[col];
                    Q[row] += mBlend[row][col] * V[col];
                    R[row] += mBlend[row][col] * W[col];
                }
            }

            // compute the tensor product (M*U)(M*V)(M*W)*D where D is the 2x2x2
            // subimage containing (x,y,z)
            Real result = (Real)0;
            for (int32_t slice = 0; slice < 2; ++slice)
            {
                int32_t zClamp = iz + slice;
                if (zClamp >= mZBound)
                {
                    zClamp = mZBound - 1;
                }

                for (int32_t row = 0; row < 2; ++row)
                {
                    int32_t yClamp = iy + row;
                    if (yClamp >= mYBound)
                    {
                        yClamp = mYBound - 1;
                    }

                    for (int32_t col = 0; col < 2; ++col)
                    {
                        int32_t xClamp = ix + col;
                        if (xClamp >= mXBound)
                        {
                            xClamp = mXBound - 1;
                        }

                        result += P[col] * Q[row] * R[slice] *
                            mF[xClamp + mXBound * (yClamp + mYBound * zClamp)];
                    }
                }
            }

            return result;
        }

        Real operator()(int32_t xOrder, int32_t yOrder, int32_t zOrder, Real x, Real y, Real z) const
        {
            // Compute x-index and clamp to image.
            Real xIndex = (x - mXMin) * mInvXSpacing;
            int32_t ix = static_cast<int32_t>(xIndex);
            if (ix < 0)
            {
                ix = 0;
            }
            else if (ix >= mXBound)
            {
                ix = mXBound - 1;
            }

            // Compute y-index and clamp to image.
            Real yIndex = (y - mYMin) * mInvYSpacing;
            int32_t iy = static_cast<int32_t>(yIndex);
            if (iy < 0)
            {
                iy = 0;
            }
            else if (iy >= mYBound)
            {
                iy = mYBound - 1;
            }

            // Compute z-index and clamp to image.
            Real zIndex = (z - mZMin) * mInvZSpacing;
            int32_t iz = static_cast<int32_t>(zIndex);
            if (iz < 0)
            {
                iz = 0;
            }
            else if (iz >= mZBound)
            {
                iz = mZBound - 1;
            }

            std::array<Real, 2> U;
            Real dx, xMult;
            switch (xOrder)
            {
            case 0:
                dx = xIndex - ix;
                U[0] = (Real)1;
                U[1] = dx;
                xMult = (Real)1;
                break;
            case 1:
                dx = xIndex - ix;
                U[0] = (Real)0;
                U[1] = (Real)1;
                xMult = mInvXSpacing;
                break;
            default:
                return (Real)0;
            }

            std::array<Real, 2> V;
            Real dy, yMult;
            switch (yOrder)
            {
            case 0:
                dy = yIndex - iy;
                V[0] = (Real)1;
                V[1] = dy;
                yMult = (Real)1;
                break;
            case 1:
                dy = yIndex - iy;
                V[0] = (Real)0;
                V[1] = (Real)1;
                yMult = mInvYSpacing;
                break;
            default:
                return (Real)0;
            }

            std::array<Real, 2> W;
            Real dz, zMult;
            switch (zOrder)
            {
            case 0:
                dz = zIndex - iz;
                W[0] = (Real)1;
                W[1] = dz;
                zMult = (Real)1;
                break;
            case 1:
                dz = zIndex - iz;
                W[0] = (Real)0;
                W[1] = (Real)1;
                zMult = mInvZSpacing;
                break;
            default:
                return (Real)0;
            }

            // Compute P = M*U, Q = M*V, and R = M*W.
            std::array<Real, 2> P, Q, R;
            for (int32_t row = 0; row < 2; ++row)
            {
                P[row] = (Real)0;
                Q[row] = (Real)0;
                R[row] = (Real)0;
                for (int32_t col = 0; col < 2; ++col)
                {
                    P[row] += mBlend[row][col] * U[col];
                    Q[row] += mBlend[row][col] * V[col];
                    R[row] += mBlend[row][col] * W[col];
                }
            }

            // Compute the tensor product (M*U)(M*V)(M*W)*D where D is the 2x2x2
            // subimage containing (x,y,z).
            Real result = (Real)0;
            for (int32_t slice = 0; slice < 2; ++slice)
            {
                int32_t zClamp = iz + slice;
                if (zClamp >= mZBound)
                {
                    zClamp = mZBound - 1;
                }

                for (int32_t row = 0; row < 2; ++row)
                {
                    int32_t yClamp = iy + row;
                    if (yClamp >= mYBound)
                    {
                        yClamp = mYBound - 1;
                    }

                    for (int32_t col = 0; col < 2; ++col)
                    {
                        int32_t xClamp = ix + col;
                        if (xClamp >= mXBound)
                        {
                            xClamp = mXBound - 1;
                        }

                        result += P[col] * Q[row] * R[slice] *
                            mF[xClamp + mXBound * (yClamp + mYBound * zClamp)];
                    }
                }
            }
            result *= xMult * yMult * zMult;

            return result;
        }

    private:
        int32_t mXBound, mYBound, mZBound, mQuantity;
        Real mXMin, mXMax, mXSpacing, mInvXSpacing;
        Real mYMin, mYMax, mYSpacing, mInvYSpacing;
        Real mZMin, mZMax, mZSpacing, mInvZSpacing;
        Real const* mF;
        std::array<std::array<Real, 2>, 2> mBlend;
    };
}
