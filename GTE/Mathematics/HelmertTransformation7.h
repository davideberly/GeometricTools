// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.2.2024.11.11

#pragma once

// Implementation of the 7-parameter Helmert transformation. It is designed
// to rotate, translate, and uniformly scale one 3D point set to be as close
// as possible to another 3D point set. Details are provided in
//   https://www.geometrictools.com/Documentation/HelmertTransformation.pdf

#include <Mathematics/Logger.h>
#include <Mathematics/Matrix3x3.h>
#include <cmath>
#include <cstddef>
#include <vector>

namespace gte
{
    template <typename T>
    class HelmertTransformation7
    {
    public:
        HelmertTransformation7()
            :
            mNumPoints(0),
            mU{},
            mV{},
            mLeft{},
            mRight{},
            mRotate{},
            mRotate0{},
            mRotate1{},
            mRotate2{},
            mTranslate(Vector3<T>::Zero())
        {
        }

        std::size_t Execute(std::vector<Vector3<T>> const& p, std::vector<Vector3<T>> const& q,
            std::size_t numIterations, Matrix3x3<T>& rotate, Vector3<T>& translate, T& scale,
            T& function)
        {
            // The input points p[i] and q[i] must correspond for 0 <= i < n,
            // where n is the number of points (n = p.size() = q.size()). The
            // outputs require 7 parameters: 3 for rotation (Euler angles),
            // 3 for translation, and 1 for uniform scale.
            mNumPoints = p.size();
            LogAssert(
                mNumPoints >= 7 && q.size() == mNumPoints,
                "Invalid input.");

            // Translate the centroid of the q-points to the origin. This
            // simplifies the function to be minimized, the only parameter
            // being the rotation matrix (function of 3 Euler angles). Also,
            // compute the centroid of the p-points.
            Vector3<T> pAverage = Vector3<T>::Zero();
            Vector3<T> qAverage = Vector3<T>::Zero();
            for (std::size_t i = 0; i < mNumPoints; ++i)
            {
                pAverage += p[i];
                qAverage += q[i];
            }
            pAverage /= static_cast<T>(mNumPoints);
            qAverage /= static_cast<T>(mNumPoints);

            // Translate by the centroid of q. The p-values are translated to
            // u-values and the q-values are translated to the v-values. The
            // average of the v-values is the zero vector.
            mU.resize(mNumPoints);
            mV.resize(mNumPoints);
            mLeft.resize(mNumPoints);
            mRight.resize(mNumPoints);
            for (std::size_t i = 0; i < mNumPoints; ++i)
            {
                mU[i] = p[i] - qAverage;
                mV[i] = q[i] - qAverage;
            }

            Matrix3x3<T> A{};
            for (std::int32_t r = 0; r < 3; ++r)
            {
                for (std::int32_t c = 0; c < 3; ++c)
                {
                    A(r, c) = 0.0;
                    for (std::size_t i = 0; i < p.size(); ++i)
                    {
                        A(r, c) += mU[i][r] * mV[i][c];
                    }
                }
            }

            // The initial rotation matrix is the identity.
            mRotate.MakeIdentity();
            mRotate0.MakeIdentity();
            mRotate1.MakeIdentity();
            mRotate2.MakeIdentity();

            // The translation does not vary during the iterations.
            mTranslate = pAverage - qAverage;

            T F = UpdateF(mRotate);
            std::size_t iteration{};
            for (iteration = 0; iteration < numIterations; ++iteration)
            {
                bool updated0 = UpdateEulerAngle0(F);
                bool updated1 = UpdateEulerAngle1(F);
                bool updated2 = UpdateEulerAngle2(F);
                if (!updated0 && !updated1 && !updated2)
                {
                    break;
                }
            }

            rotate = mRotate;
            scale = UpdateScale(mRotate);
            translate = mTranslate + qAverage - scale * (rotate * qAverage);
            function = F;
            return iteration;
        }

    private:
        T UpdateScale(Matrix3x3<T> const& rotate)
        {
            T const zero = static_cast<T>(0);
            T numer = zero, denom = zero;
            for (std::size_t i = 0; i < mNumPoints; ++i)
            {
                numer += Dot(mU[i], rotate * mV[i]);
                denom += Dot(mV[i], mV[i]);
            }
            T scale = numer / denom;
            return scale;
        }

        T UpdateF(Matrix3x3<T> const& rotate)
        {
            T scale = UpdateScale(rotate);;
            T function = static_cast<T>(0);
            for (std::size_t i = 0; i < mNumPoints; ++i)
            {
                Vector3<T> term = scale * (rotate * mV[i]) + mTranslate - mU[i];
                function += Dot(term, term);
            }
            function /= static_cast<T>(mNumPoints);
            return function;
        }

        bool UpdateEulerAngle0(T& F)
        {
            T const zero = static_cast<T>(0), one = static_cast<T>(1);

            Matrix3x3<T> R1R2 = mRotate1 * mRotate2;
            auto const& left = mU;
            auto& right = mRight;
            for (std::size_t i = 0; i < mNumPoints; ++i)
            {
                right[i] = R1R2 * mV[i];
            }

            T sn = zero, cs = zero;
            for (std::size_t i = 0; i < mNumPoints; ++i)
            {
                sn += left[i][1] * right[i][0] - left[i][0] * right[i][1];
                cs += left[i][0] * right[i][0] + left[i][1] * right[i][1];
            }

            T length = std::sqrt(sn * sn + cs * cs);
            if (length > zero)
            {
                sn /= length;
                cs /= length;
            }
            else
            {
                sn = zero;
                cs = one;
            }

            Matrix3x3<T> rotate0{};
            rotate0(0, 0) = cs;   rotate0(0, 1) =  -sn; rotate0(0, 2) = zero;
            rotate0(1, 0) = sn;   rotate0(1, 1) =   cs; rotate0(1, 2) = zero;
            rotate0(2, 0) = zero; rotate0(2, 1) = zero; rotate0(2, 2) = one;

            Matrix3x3<T> updateRotate = rotate0 * R1R2;
            T updateF = UpdateF(updateRotate);
            if (updateF < F)
            {
                mRotate0 = rotate0;
                mRotate = updateRotate;
                F = updateF;
                return true;
            }
            else
            {
                return false;
            }
        }

        bool UpdateEulerAngle1(T& F)
        {
            T const zero = static_cast<T>(0), one = static_cast<T>(1);

            auto& left = mLeft;
            auto& right = mRight;
            for (std::size_t i = 0; i < mNumPoints; ++i)
            {
                left[i] = mU[i] * mRotate0;
                right[i] = mRotate2 * mV[i];
            }

            T sn = zero, cs = zero;
            for (std::size_t i = 0; i < mNumPoints; ++i)
            {
                sn += left[i][0] * right[i][2] - left[i][2] * right[i][0];
                cs += left[i][0] * right[i][0] + left[i][2] * right[i][2];
            }

            T length = std::sqrt(sn * sn + cs * cs);
            if (length > zero)
            {
                sn /= length;
                cs /= length;
            }
            else
            {
                sn = zero;
                cs = one;
            }

            Matrix3x3<T> rotate1{};
            rotate1(0, 0) =   cs; rotate1(0, 1) = zero; rotate1(0, 2) = sn;
            rotate1(1, 0) = zero; rotate1(1, 1) = one;  rotate1(1, 2) = zero;
            rotate1(2, 0) =  -sn; rotate1(2, 1) = zero; rotate1(2, 2) = cs;

            Matrix3x3<T> updateRotate = mRotate0 * rotate1 * mRotate2;
            T updateF = UpdateF(updateRotate);
            if (updateF < F)
            {
                mRotate1 = rotate1;
                mRotate = updateRotate;
                F = updateF;
                return true;
            }
            else
            {
                return false;
            }
        }

        bool UpdateEulerAngle2(T& F)
        {
            T const zero = static_cast<T>(0), one = static_cast<T>(1);

            Matrix3x3<T> R0R1 = mRotate0 * mRotate1;
            auto& left = mLeft;
            auto const& right = mV;
            for (std::size_t i = 0; i < mNumPoints; ++i)
            {
                left[i] = mU[i] * R0R1;
            }

            T sn = zero, cs = zero;
            for (std::size_t i = 0; i < mNumPoints; ++i)
            {
                sn += left[i][2] * right[i][1] - left[i][1] * right[i][2];
                cs += left[i][1] * right[i][1] + left[i][2] * right[i][2];
            }

            T length = std::sqrt(sn * sn + cs * cs);
            if (length > zero)
            {
                sn /= length;
                cs /= length;
            }
            else
            {
                sn = zero;
                cs = one;
            }

            Matrix3x3<T> rotate2{};
            rotate2(0, 0) = one;  rotate2(0, 1) = zero; rotate2(0, 2) = zero;
            rotate2(1, 0) = zero; rotate2(1, 1) = cs;   rotate2(1, 2) = -sn;
            rotate2(2, 0) = zero; rotate2(2, 1) = sn;   rotate2(2, 2) =  cs;

            Matrix3x3<T> updateRotate = R0R1 * rotate2;
            T updateF = UpdateF(updateRotate);
            if (updateF < F)
            {
                mRotate2 = rotate2;
                mRotate = updateRotate;
                F = updateF;
                return true;
            }
            else
            {
                return false;
            }
        }

        std::size_t mNumPoints;
        std::vector<Vector3<T>> mU, mV, mLeft, mRight;
        Matrix3x3<T> mRotate, mRotate0, mRotate1, mRotate2;
        Vector3<T> mTranslate;
    };
}
