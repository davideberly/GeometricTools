// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// Compute geodesics on a B-spline height field. The algorithm for
// constructing geodesics that is implemented here uses a multiresolution
// approach. A description of the algorithm is in the document
// https://www.geometrictools.com/Documentation/RiemannianGeodesics.pdf

#include <GTL/Mathematics/Curves/RiemannianGeodesic.h>
#include <GTL/Mathematics/Surfaces/BSplineSurface.h>

namespace gtl
{
    template <typename T>
    class BSplineGeodesic : public RiemannianGeodesic<T>
    {
    public:
        BSplineGeodesic(
            BSplineSurface<T, 3> const& spline,
            size_t numIntegralSamples,
            size_t numSearchSamples,
            size_t numSubdivisions,
            size_t numRefinements,
            T const& derivativeStep,
            T const& searchRadius)
            :
            RiemannianGeodesic<T>(2, numIntegralSamples, numSearchSamples,
                numSubdivisions, numRefinements, derivativeStep, searchRadius),
            mSpline(&spline)
        {
        }

        virtual ~BSplineGeodesic() = default;

    private:
        virtual void ComputeMetric(Vector<T> const& point) override
        {
            mSpline->Evaluate(point[0], point[1], 2, mJet.data());
            Vector3<T> const& der0 = mJet[1];
            Vector3<T> const& der1 = mJet[2];

            this->mMetric(0, 0) = Dot(der0, der0);
            this->mMetric(0, 1) = Dot(der0, der1);
            this->mMetric(1, 0) = this->mMetric(0, 1);
            this->mMetric(1, 1) = Dot(der1, der1);
        }

        virtual void ComputeChristoffel1(Vector<T> const&) override
        {
            Vector3<T> const& der0 = mJet[1];
            Vector3<T> const& der1 = mJet[2];
            Vector3<T> const& der00 = mJet[3];
            Vector3<T> const& der01 = mJet[4];
            Vector3<T> const& der11 = mJet[5];

            this->mChristoffel1[0](0, 0) = Dot(der00, der0);
            this->mChristoffel1[0](0, 1) = Dot(der01, der0);
            this->mChristoffel1[0](1, 0) = this->mChristoffel1[0](0, 1);
            this->mChristoffel1[0](1, 1) = Dot(der11, der0);

            this->mChristoffel1[1](0, 0) = Dot(der00, der1);
            this->mChristoffel1[1](0, 1) = Dot(der01, der1);
            this->mChristoffel1[1](1, 0) = this->mChristoffel1[1](0, 1);
            this->mChristoffel1[1](1, 1) = Dot(der11, der1);
        }

        BSplineSurface<T, 3> const* mSpline;

        // We are guaranteed that RiemannianGeodesic calls ComputeMetric
        // before ComputeChristoffel1. Therefore, we can compute the B-spline
        // first- and second-order derivatives in ComputeMetric and cache
        // the results for use in ComputeChristoffel1.
        std::array<Vector3<T>, 6> mJet;
    };
}
