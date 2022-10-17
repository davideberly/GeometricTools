// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// Compute geodesics on an ellipsoid. The algorithm for constructing
// geodesics that is implemented here uses a multiresolution approach. A
// description of the algorithm is in the document
// https://www.geometrictools.com/Documentation/RiemannianGeodesics.pdf
// To compute the geodesic path connecting two parameter points (u0,v0)
// and (u1,v1):
//
// float a, b, c;  // the extents of the ellipsoid
// EllipsoidGeodesic<float> EG(a,b,c);
// Vector<float> param0(2), param1(2);
// param0[0] = u0;
// param0[1] = v0;
// param1[0] = u1;
// param1[1] = v1;
//
// size_t quantity = 0;
// std:vector<Vector<float>> path;
// EG.ComputeGeodesic(param0, param1, quantity, path);

#include <GTL/Mathematics/Curves/RiemannianGeodesic.h>

namespace gtl
{
    template <typename T>
    class EllipsoidGeodesic : public RiemannianGeodesic<T>
    {
    public:
        // The ellipsoid is (x/a)^2 + (y/b)^2 + (z/c)^2 = 1, where xExtent is
        // 'a', yExtent is 'b', and zExtent is 'c'. The surface is represented
        // parametrically by angles u and v, say
        //   P(u,v) = (x(u,v),y(u,v),z(u,v)),
        //   P(u,v) =(a*cos(u)*sin(v), b*sin(u)*sin(v), c*cos(v))
        // with 0 <= u < 2*pi and 0 <= v <= pi. The first-order derivatives
        // are
        //   dP/du = (-a*sin(u)*sin(v), b*cos(u)*sin(v), 0)
        //   dP/dv = (a*cos(u)*cos(v), b*sin(u)*cos(v), -c*sin(v))
        // The metric tensor elements are
        //   g_{00} = Dot(dP/du,dP/du)
        //   g_{01} = Dot(dP/du,dP/dv)
        //   g_{10} = g_{01}
        //   g_{11} = Dot(dP/dv,dP/dv)

        EllipsoidGeodesic(T const& xExtent, T const& yExtent, T const& zExtent)
            :
            RiemannianGeodesic<T>(2),
            mXExtent(xExtent),
            mYExtent(yExtent),
            mZExtent(zExtent)
        {
        }

        virtual ~EllipsoidGeodesic() = default;

        Vector3<T> ComputePosition(Vector<T> const& point)
        {
            T cos0 = std::cos(point[0]);
            T sin0 = std::sin(point[0]);
            T cos1 = std::cos(point[1]);
            T sin1 = std::sin(point[1]);

            return Vector3<T>
            {
                mXExtent * cos0 * sin1,
                mYExtent * sin0 * sin1,
                mZExtent * cos1
            };
        }

    private:
        virtual void ComputeMetric(Vector<T> const& point) override
        {
            mCos0 = std::cos(point[0]);
            mSin0 = std::sin(point[0]);
            mCos1 = std::cos(point[1]);
            mSin1 = std::sin(point[1]);

            mDer0 =
            {
                -mXExtent * mSin0 * mSin1,
                mYExtent * mCos0 * mSin1,
                C_<T>(0)
            };

            mDer1 =
            {
                mXExtent * mCos0 * mCos1,
                mYExtent * mSin0 * mCos1,
                -mZExtent * mSin1 };

            this->mMetric(0, 0) = Dot(mDer0, mDer0);
            this->mMetric(0, 1) = Dot(mDer0, mDer1);
            this->mMetric(1, 0) = this->mMetric(0, 1);
            this->mMetric(1, 1) = Dot(mDer1, mDer1);
        }

        virtual void ComputeChristoffel1(Vector<T> const&) override
        {
            Vector3<T> der00
            {
                -mXExtent * mCos0 * mSin1,
                -mYExtent * mSin0 * mSin1,
                C_<T>(0)
            };

            Vector3<T> der01
            {
                -mXExtent * mSin0 * mCos1,
                mYExtent * mCos0 * mCos1,
                C_<T>(0)
            };

            Vector3<T> der11
            {
                -mXExtent * mCos0 * mSin1,
                -mYExtent * mSin0 * mSin1,
                -mZExtent * mCos1
            };

            this->mChristoffel1[0](0, 0) = Dot(der00, mDer0);
            this->mChristoffel1[0](0, 1) = Dot(der01, mDer0);
            this->mChristoffel1[0](1, 0) = this->mChristoffel1[0](0, 1);
            this->mChristoffel1[0](1, 1) = Dot(der11, mDer0);

            this->mChristoffel1[1](0, 0) = Dot(der00, mDer1);
            this->mChristoffel1[1](0, 1) = Dot(der01, mDer1);
            this->mChristoffel1[1](1, 0) = this->mChristoffel1[1](0, 1);
            this->mChristoffel1[1](1, 1) = Dot(der11, mDer1);
        }

        // The ellipsoid axis half-lengths.
        T mXExtent, mYExtent, mZExtent;

        // We are guaranteed that RiemannianGeodesic calls ComputeMetric
        // before ComputeChristoffel1. Therefore, we can compute the surface
        // first- and second-order derivatives in ComputeMetric and cache the
        // results for use in ComputeChristoffel1.
        T mCos0, mSin0, mCos1, mSin1;
        Vector3<T> mDer0, mDer1;
    };
}
