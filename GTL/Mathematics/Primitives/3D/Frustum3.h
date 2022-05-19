// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// Orthogonal frustum. Let E be the origin, D be the direction vector, U be
// the up vector, and R be the right vector. Let u > 0 and r > 0 be the
// extents in the U and R directions, respectively. Let n and f be the
// extents in the D direction with 0 < n < f. The four corners of the frustum
// in the near plane are E + n*D + s0*u*U + s1*r*R where |s0| = |s1| = 1 (four
// choices). The four corners of the frustum in the far plane are
// E + f*D + (f/n)*(s0*u*U + s1*r*R) where |s0| = |s1| = 1 (four choices).

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cstddef>

namespace gtl
{
    template <typename T>
    class Frustum3
    {
    public:
        using value_type = T;

        // Construction. The default constructor sets all member to zero.
        Frustum3()
            :
            origin{},
            dVector{},
            uVector{},
            rVector{},
            dMin(C_<T>(0)),
            dMax(C_<T>(0)),
            uBound(C_<T>(0)),
            rBound(C_<T>(0)),
            mDRatio(C_<T>(0)),
            mMTwoUF(C_<T>(0)),
            mMTwoRF(C_<T>(0))
        {
            Update();
        }

        Frustum3(Vector3<T> const& inOrigin, Vector3<T> const& inDVector,
            Vector3<T> const& inUVector, Vector3<T> const& inRVector,
            T const& inDMin, T const& inDMax, T const& inUBound, T const& inRBound)
            :
            origin(inOrigin),
            dVector(inDVector),
            uVector(inUVector),
            rVector(inRVector),
            dMin(inDMin),
            dMax(inDMax),
            uBound(inUBound),
            rBound(inRBound),
            mDRatio(C_<T>(0)),
            mMTwoUF(C_<T>(0)),
            mMTwoRF(C_<T>(0))
        {
            Update();
        }

        // The Update() function must be called whenever changes are made to
        // dMin, dMax, uBound or rBound. The values mDRatio, mMTwoUF and
        // mMTwoRF are dependent on the changes, so call the Get*() accessors
        // only after the Update() call.
        void Update()
        {
            T const negTwoTimesdMax = -C_<T>(2) * dMax;
            mDRatio = (dMin != C_<T>(0) ? dMax / dMin : C_<T>(0));
            mMTwoUF = negTwoTimesdMax * uBound;
            mMTwoRF = negTwoTimesdMax * rBound;
        }

        inline T const& GetDRatio() const
        {
            return mDRatio;
        }

        inline T const& GetMTwoUF() const
        {
            return mMTwoUF;
        }

        inline T const& GetMTwoRF() const
        {
            return mMTwoRF;
        }

        // Using the notation at the top of the header file, the first four
        // vertices are those of the near face. These are listed in
        // counterclockwise order as viewed by the observer at the origin.
        // The last four vertices are those of the far face. These are listed
        // in counterclockwise order as viewed by the observer at the origin:
        //   vertex[0] = E + n * D - u * U - r * R
        //   vertex[1] = E + n * D - u * U + r * R
        //   vertex[2] = E + n * D + u * U - r + R
        //   vertex[3] = E + n * D + u * U + r + R
        //   vertex[4] = E + f * D + (f / n) * (-u * U - r * R)
        //   vertex[5] = E + f * D + (f / n) * (-u * U + r * R)
        //   vertex[6] = E + f * D + (f / n) * (+u * U - r + R)
        //   vertex[7] = E + f * D + (f / n) * (+u * U + r + R)
        void ComputeVertices(std::array<Vector3<T>, 8>& vertex) const
        {
            Vector3<T> dScaled = dMin * dVector;
            Vector3<T> uScaled = uBound * uVector;
            Vector3<T> rScaled = rBound * rVector;

            vertex[0] = dScaled - uScaled - rScaled;
            vertex[1] = dScaled - uScaled + rScaled;
            vertex[2] = dScaled + uScaled + rScaled;
            vertex[3] = dScaled + uScaled - rScaled;

            for (size_t i = 0, ip = 4; i < 4; ++i, ++ip)
            {
                vertex[ip] = origin + mDRatio * vertex[i];
                vertex[i] += origin;
            }
        }

        Vector3<T> origin, dVector, uVector, rVector;
        T dMin, dMax, uBound, rBound;

    private:
        // Quantities derived from the constructor inputs.
        T mDRatio, mMTwoUF, mMTwoRF;

        friend class UnitTestFrustum3;
    };

    // Comparisons to support sorted containers.
    template <typename T>
    bool operator==(Frustum3<T> const& frustum0, Frustum3<T> const& frustum1)
    {
        return frustum0.origin == frustum1.origin
            && frustum0.dVector == frustum1.dVector
            && frustum0.uVector == frustum1.uVector
            && frustum0.rVector == frustum1.rVector
            && frustum0.dMin == frustum1.dMin
            && frustum0.dMax == frustum1.dMax
            && frustum0.uBound == frustum1.uBound
            && frustum0.rBound == frustum1.rBound;
    }

    template <typename T>
    bool operator!=(Frustum3<T> const& frustum0, Frustum3<T> const& frustum1)
    {
        return !operator==(frustum0, frustum1);
    }

    template <typename T>
    bool operator<(Frustum3<T> const& frustum0, Frustum3<T> const& frustum1)
    {
        if (frustum0.origin < frustum1.origin)
        {
            return true;
        }

        if (frustum0.origin > frustum1.origin)
        {
            return false;
        }

        if (frustum0.dVector < frustum1.dVector)
        {
            return true;
        }

        if (frustum0.dVector > frustum1.dVector)
        {
            return false;
        }

        if (frustum0.uVector < frustum1.uVector)
        {
            return true;
        }

        if (frustum0.uVector > frustum1.uVector)
        {
            return false;
        }

        if (frustum0.rVector < frustum1.rVector)
        {
            return true;
        }

        if (frustum0.rVector > frustum1.rVector)
        {
            return false;
        }

        if (frustum0.dMin < frustum1.dMin)
        {
            return true;
        }

        if (frustum0.dMin > frustum1.dMin)
        {
            return false;
        }

        if (frustum0.dMax < frustum1.dMax)
        {
            return true;
        }

        if (frustum0.dMax > frustum1.dMax)
        {
            return false;
        }

        if (frustum0.uBound < frustum1.uBound)
        {
            return true;
        }

        if (frustum0.uBound > frustum1.uBound)
        {
            return false;
        }

        return frustum0.rBound < frustum1.rBound;
    }

    template <typename T>
    bool operator<=(Frustum3<T> const& frustum0, Frustum3<T> const& frustum1)
    {
        return !operator<(frustum1, frustum0);
    }

    template <typename T>
    bool operator>(Frustum3<T> const& frustum0, Frustum3<T> const& frustum1)
    {
        return operator<(frustum1, frustum0);
    }

    template <typename T>
    bool operator>=(Frustum3<T> const& frustum0, Frustum3<T> const& frustum1)
    {
        return !operator<(frustum0, frustum1);
    }
}
