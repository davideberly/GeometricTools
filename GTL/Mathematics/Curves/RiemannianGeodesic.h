// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// Computing geodesics on a surface is a differential geometric topic that
// involves Riemannian geometry. The algorithm for constructing geodesics
// that is implemented here uses a multiresolution approach. A description
// of the algorithm is in the document
// https://www.geometrictools.com/Documentation/RiemannianGeodesics.pdf
// The refinementCallback is for use by applications to obtain in-algorithm
// reporting of information about the subdivisions. In the sample
// applications, this is used to draw the current estimate of the geodesic
// curves.

#include <GTL/Mathematics/MatrixAnalysis/GaussianElimination.h>

namespace gtl
{
    // The type T must be a floating-point type.
    template <typename T>
    class RiemannianGeodesic
    {
    public:
        // Construction and destruction. The input dimension must be two or
        // larger.

        // The input parameters are described next.
        // 1. The integral samples are the number of samples used in the
        //    Trapezoid Rule numerical integrator.
        // 2. The search samples are the number of samples taken along a ray
        //    for the steepest descent algorithm used to refine the vertices
        //    of the polyline approximation to the geodesic curve.
        // 3. The number of subdivisions indicates how many times the polyline
        //    segments should be subdivided. The number of polyline vertices
        //    will be pow(2,subdivisions)+1.
        // 4. The number of refinements per subdivision. Setting this to a
        //    positive value appears necessary when the geodesic curve has a
        //    large length.
        // 5. The derivative step is the value of h used for centered
        //    difference approximations df/dx = (f(x+h)-f(x-h))/(2*h) in the
        //    steepest descent algorithm.
        // 6. The search radius is the distance over which the steepest
        //    descent algorithm searches for a minimum on the line whose
        //    direction is the estimated gradient. The default of 1 means the
        //    search interval is [-L,L], where L is the length of the gradient.
        //    If the search radius is r, then the interval is [-r*L,r*L].
        RiemannianGeodesic(
            size_t dimension,
            size_t numIntegralSamples,
            size_t numSearchSamples,
            size_t numSubdivisions,
            size_t numRefinements,
            T const& derivativeStep,
            T const& searchRadius)
            :
            refinementCallback([]() {}),
            mDimension(dimension),
            mNumIntegralSamples(numIntegralSamples),
            mNumSearchSamples(numSearchSamples),
            mNumSubdivisions(numSubdivisions),
            mNumRefinements(numRefinements),
            mDerivativeStep(derivativeStep),
            mSearchRadius(searchRadius),
            mIntegralStep(C_<T>(1) / static_cast<T>(mNumIntegralSamples - 1)),
            mSearchStep(C_<T>(1) / static_cast<T>(mNumSearchSamples)),
            mDerivativeFactor(C_<T>(1, 2) / mDerivativeStep),
            mSubdivision(0),
            mRefinement(0),
            mCurrentQuantity(0),
            mMetric(mDimension, mDimension),
            mMetricInverse(mDimension, mDimension),
            mChristoffel1(mDimension),
            mChristoffel2(mDimension),
            mMetricDerivative(mDimension)
        {
            GTL_ARGUMENT_ASSERT(
                mDimension >= 2,
                "The dimension must be at least 2.");

            GTL_ARGUMENT_ASSERT(
                mNumIntegralSamples >= 2,
                "The number of integral samples must be at least 2.");

            for (size_t i = 0; i < mDimension; ++i)
            {
                mChristoffel1[i].resize(mDimension, mDimension);
                mChristoffel2[i].resize(mDimension, mDimension);
                mMetricDerivative[i].resize(mDimension, mDimension);
            }
        }

        RiemannianGeodesic(size_t dimension)
            :
            RiemannianGeodesic(dimension, 16, 32, 7, 8, static_cast<T>(0.0001), C_<T>(1))
        {
        }

        virtual ~RiemannianGeodesic() = default;

        // Member access.
        inline size_t GetDimension() const
        {
            return mDimension;
        }

        inline size_t GetNumIntegralSamples() const
        {
            return mNumIntegralSamples;
        }

        inline size_t GetNumSearchSamples() const
        {
            return mNumSearchSamples;
        }

        inline size_t GetNumSubdivisions() const
        {
            return mNumSubdivisions;
        }

        inline size_t GetNumRefinements() const
        {
            return mNumRefinements;
        }

        inline T GetDerivativeStep() const
        {
            return mDerivativeStep;
        }

        inline T GetSearchRadius() const
        {
            return mSearchRadius;
        }

        // Returns the length of the line segment connecting the points.
        T ComputeSegmentLength(Vector<T> const& point0, Vector<T> const& point1)
        {
            // The Trapezoid Rule is used for integration of the length
            // integral. The ComputeMetric function internally modifies
            // mMetric, which means the qForm values are actually varying
            // even though diff does not.
            Vector<T> diff = point1 - point0;
            Vector<T> temp(mDimension);

            // Evaluate the integrand at point0.
            ComputeMetric(point0);
            T qForm = Dot(diff, mMetric * diff);
            GTL_RUNTIME_ASSERT(
                qForm > C_<T>(0),
                "Unexpected condition.");
            T length = std::sqrt(qForm);

            // Evaluate the integrand at point1.
            ComputeMetric(point1);
            qForm = Dot(diff, mMetric * diff);
            GTL_RUNTIME_ASSERT(
                qForm > C_<T>(0),
                "Unexpected condition.");
            length += std::sqrt(qForm);
            length *= C_<T>(1, 2);

            size_t const imax = mNumIntegralSamples - 2;
            for (size_t i = 1; i <= imax; ++i)
            {
                // Evaluate the integrand at point0 + t * (point1 - point0).
                T t = mIntegralStep * static_cast<T>(i);
                temp = point0 + t * diff;
                ComputeMetric(temp);
                qForm = Dot(diff, mMetric * diff);
                GTL_RUNTIME_ASSERT(
                    qForm > C_<T>(0),
                    "Unexpected condition.");
                length += std::sqrt(qForm);
            }
            length *= mIntegralStep;
            return length;
        }

        // Compute the total length of the polyline. The lengths of the
        // segments are computed relative to the metric tensor.
        T ComputeTotalLength(size_t quantity, std::vector<Vector<T>> const& path)
        {
            GTL_ARGUMENT_ASSERT(
                quantity >= 2,
                "The path must have at least two points.");

            size_t const imax = quantity - 2;
            T length = ComputeSegmentLength(path[0], path[1]);
            for (size_t i = 1; i <= imax; ++i)
            {
                length += ComputeSegmentLength(path[i], path[i + 1]);
            }
            return length;
        }

        // Returns a polyline approximation to a geodesic curve connecting the
        // points.
        void ComputeGeodesic(Vector<T> const& end0, Vector<T> const& end1,
            size_t& quantity, std::vector<Vector<T>>& path)
        {
            GTL_ARGUMENT_ASSERT(
                mNumSubdivisions < 32,
                "The number of subdivisions has exceeded the maximum.");

            quantity = (static_cast<size_t>(1) << mNumSubdivisions) + 1;
            path.resize(quantity);
            for (size_t i = 0; i < quantity; ++i)
            {
                path[i].resize(mDimension);
            }

            mCurrentQuantity = 2;
            path[0] = end0;
            path[1] = end1;

            for (mSubdivision = 1; mSubdivision <= mNumSubdivisions; ++mSubdivision)
            {
                // A subdivision essentially doubles the number of points.
                size_t newQuantity = 2 * mCurrentQuantity - 1;
                GTL_RUNTIME_ASSERT(
                    newQuantity <= quantity,
                    "Unexpected condition.");

                // Copy the old points so that there are slots for the
                // midpoints during the subdivision, the slots interleaved
                // between the old points.
                for (size_t i = mCurrentQuantity - 1; i > 0; --i)
                {
                    path[2 * i] = path[i];
                }

                // Subdivide the polyline.
                for (size_t i = 0; i <= mCurrentQuantity - 2; ++i)
                {
                    Subdivide(path[2 * i], path[2 * i + 1], path[2 * i + 2]);
                }

                mCurrentQuantity = newQuantity;

                // Refine the current polyline vertices.
                for (mRefinement = 1; mRefinement <= mNumRefinements; ++mRefinement)
                {
                    for (size_t i = 1; i <= mCurrentQuantity - 2; ++i)
                    {
                        Refine(path[i - 1], path[i], path[i + 1]);
                    }
                }
            }

            GTL_RUNTIME_ASSERT(
                mCurrentQuantity == quantity,
                "Unexpected condition.");

            mSubdivision = 0;
            mRefinement = 0;
            mCurrentQuantity = 0;
        }

        // Start with the midpoint M of the line segment (E0,E1) and use a
        // steepest descent algorithm to move M so that
        //     Length(E0,M) + Length(M,E1) < Length(E0,E1)
        // This is essentially a relaxation scheme that inserts points into
        // the current polyline approximation to the geodesic curve.
        bool Subdivide(Vector<T> const& end0, Vector<T>& mid, Vector<T> const& end1)
        {
            mid = C_<T>(1, 2) * (end0 + end1);
            std::function<void(void)> save = refinementCallback;
            refinementCallback = []() {};
            bool changed = Refine(end0, mid, end1);
            refinementCallback = save;
            return changed;
        }

        // Apply the steepest descent algorithm to move the midpoint M of the
        // line segment (E0,E1) so that
        //     Length(E0,M) + Length(M,E1) < Length(E0,E1)
        // This is essentially a relaxation scheme that inserts points into
        // the current polyline approximation to the geodesic curve.
        bool Refine(Vector<T> const& end0, Vector<T>& mid, Vector<T> const& end1)
        {
            // Estimate the gradient vector for the function
            // F(m) = Length(e0,m) + Length(m,e1).
            Vector<T> temp = mid, gradient(mDimension);
            for (size_t i = 0; i < mDimension; ++i)
            {
                temp[i] = mid[i] + mDerivativeStep;
                gradient[i] = ComputeSegmentLength(end0, temp);
                gradient[i] += ComputeSegmentLength(temp, end1);

                temp[i] = mid[i] - mDerivativeStep;
                gradient[i] -= ComputeSegmentLength(end0, temp);
                gradient[i] -= ComputeSegmentLength(temp, end1);

                temp[i] = mid[i];
                gradient[i] *= mDerivativeFactor;
            }

            // Compute the length sum for the current midpoint.
            T length0 = ComputeSegmentLength(end0, mid);
            T length1 = ComputeSegmentLength(mid, end1);
            T oldLength = length0 + length1;

            T tRay, newLength;
            Vector<T> pRay(mDimension);

            T multiplier = mSearchStep * mSearchRadius;
            T minLength = oldLength;
            Vector<T> minPoint = mid;
            size_t const jmax = 2 * mNumSearchSamples + 1;
            int32_t i = -static_cast<int32_t>(mNumSearchSamples);
            for (size_t j = 0; j <= jmax; ++j, ++i)
            {
                tRay = multiplier * static_cast<T>(i);
                pRay = mid - tRay * gradient;
                length0 = ComputeSegmentLength(end0, pRay);
                length1 = ComputeSegmentLength(end1, pRay);
                newLength = length0 + length1;
                if (newLength < minLength)
                {
                    minLength = newLength;
                    minPoint = pRay;
                }
            }

            mid = minPoint;
            refinementCallback();
            return minLength < oldLength;
        }

        // Information to be used during the callback.
        inline size_t GetSubdivision() const
        {
            return mSubdivision;
        }

        inline size_t GetRefinement() const
        {
            return mRefinement;
        }

        inline size_t GetCurrentQuantity() const
        {
            return mCurrentQuantity;
        }

        // Curvature computations to measure how close the approximating
        // polyline is to a geodesic.

        // Returns the total curvature of the line segment connecting the
        // points.
        T ComputeSegmentCurvature(Vector<T> const& point0, Vector<T> const& point1)
        {
            // The Trapezoid Rule is used for integration of the curvature
            // integral. The ComputeIntegrand function internally modifies
            // mMetric, which means the curvature values are actually varying
            // even though diff does not.
            Vector<T> diff = point1 - point0;
            Vector<T> temp(mDimension);

            // Evaluate the integrand at point0.
            T curvature = ComputeIntegrand(point0, diff);

            // Evaluate the integrand at point1.
            curvature += ComputeIntegrand(point1, diff);
            curvature *= C_<T>(1, 2);

            size_t const imax = mNumIntegralSamples - 2;
            for (size_t i = 1; i <= imax; ++i)
            {
                // Evaluate the integrand at point0+t*(point1-point0).
                T t = mIntegralStep * static_cast<T>(i);
                temp = point0 + t * diff;
                curvature += ComputeIntegrand(temp, diff);
            }
            curvature *= mIntegralStep;
            return curvature;
        }

        // Compute the total curvature of the polyline. The curvatures of the
        // segments are computed relative to the metric tensor.
        T ComputeTotalCurvature(size_t quantity, std::vector<Vector<T>> const& path)
        {
            GTL_ARGUMENT_ASSERT(
                quantity >= 2,
                "Path must have at least two points.");

            T curvature = ComputeSegmentCurvature(path[0], path[1]);
            size_t const imax = quantity - 2;
            for (size_t i = 1; i <= imax; ++i)
            {
                curvature += ComputeSegmentCurvature(path[i], path[i + 1]);
            }
            return curvature;
        }

        // This function is executed during each call to Refine.
        std::function<void(void)> refinementCallback;

    protected:
        // Support for ComputeSegmentCurvature.
        T ComputeIntegrand(Vector<T> const& pos, Vector<T> const& der)
        {
            ComputeMetric(pos);
            ComputeChristoffel1(pos);
            ComputeMetricInverse();
            ComputeChristoffel2();

            // g_{ij} * der_{i} * der_{j}
            T qForm0 = Dot(der, mMetric * der);
            GTL_RUNTIME_ASSERT(
                qForm0 > C_<T>(0),
                "Unexpected condition.");

            // gamma_{kij} * der_{k} * der_{i} * der_{j}
            Matrix<T> mat(mDimension, mDimension);
            for (size_t k = 0; k < mDimension; ++k)
            {
                mat += der[k] * mChristoffel1[k];
            }
            // This product can be negative because mat is not guaranteed to
            // be positive semidefinite.  No assertion is added.
            T qForm1 = Dot(der, mat * der);

            T ratio = -qForm1 / qForm0;

            // Compute the acceleration.
            Vector<T> acc = ratio * der;
            for (size_t k = 0; k < mDimension; ++k)
            {
                acc[k] += Dot(der, mChristoffel2[k] * der);
            }

            // Compute the curvature.
            T curvature = std::sqrt(Dot(acc, mMetric * acc));
            return curvature;
        }

        // Compute the metric tensor for the specified point. Derived classes
        // are responsible for implementing this function.
        virtual void ComputeMetric(Vector<T> const& point) = 0;

        // Compute the Christoffel symbols of the first kind for the current
        // point. Derived classes are responsible for implementing this
        // function.
        virtual void ComputeChristoffel1(Vector<T> const& point) = 0;

        // Compute the inverse of the current metric tensor.
        void ComputeMetricInverse()
        {
            T* determinant = nullptr;
            mMetricInverse = Inverse(mMetric, determinant);
        }

        // Compute the derivative of the metric tensor for the current state.
        // This is a triply indexed quantity, the values computed using the
        // Christoffel symbols of the first kind.
        void ComputeMetricDerivative()
        {
            for (size_t derivative = 0; derivative < mDimension; ++derivative)
            {
                for (size_t i0 = 0; i0 < mDimension; ++i0)
                {
                    for (size_t i1 = 0; i1 < mDimension; ++i1)
                    {
                        mMetricDerivative[derivative](i0, i1) =
                            mChristoffel1[derivative](i0, i1) +
                            mChristoffel1[derivative](i1, i0);
                    }
                }
            }
        }

        // Compute the Christoffel symbols of the second kind for the current
        // state.
        void ComputeChristoffel2()
        {
            for (size_t i2 = 0; i2 < mDimension; ++i2)
            {
                for (size_t i0 = 0; i0 < mDimension; ++i0)
                {
                    for (size_t i1 = 0; i1 < mDimension; ++i1)
                    {
                        T value = C_<T>(0);
                        for (size_t j = 0; j < mDimension; ++j)
                        {
                            value += mMetricInverse(i2, j) * mChristoffel1[j](i0, i1);
                        }
                        mChristoffel2[i2](i0, i1) = value;
                    }
                }
            }
        }

        // Parameters to the constructor.
        size_t mDimension;
        size_t mNumIntegralSamples; // default = 16
        size_t mNumSearchSamples;   // default = 32
        size_t mNumSubdivisions;    // default = 7
        size_t mNumRefinements;     // default = 8
        T mDerivativeStep;          // default = 0.0001
        T mSearchRadius;            // default = 1.0

        // Derived tweaking parameters.
        T mIntegralStep;            // = 1 / (mNumIntegralSamples - 1)
        T mSearchStep;              // = 1 / mNumSearchSamples
        T mDerivativeFactor;        // = 1 / (2 * mDerivativeStep)

        // Progress parameters that are useful to refineCallback.
        size_t mSubdivision;
        size_t mRefinement;
        size_t mCurrentQuantity;

        // Mathematical support for computing a geodesic path.
        Matrix<T> mMetric;
        Matrix<T> mMetricInverse;
        std::vector<Matrix<T>> mChristoffel1;
        std::vector<Matrix<T>> mChristoffel2;
        std::vector<Matrix<T>> mMetricDerivative;
    };
}
