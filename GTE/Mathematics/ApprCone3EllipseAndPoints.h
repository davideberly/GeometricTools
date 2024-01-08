// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2023.08.08

#pragma once

// An infinite single-sided cone is fit to a 3D ellipse that is known to be
// the intersection of a plane with the cone. The ellipse itself is not
// enough information, producing the cone vertex and cone direction as a
// function of the cone angle. Additional points on the cone are required
// to determine the cone angle. The algorithm description is
// https://www.geometrictools.com/Documentation/FitConeToEllipseAndPoints.pdf

#include <Mathematics/Logger.h>
#include <Mathematics/ApprGaussian3.h>
#include <Mathematics/ApprEllipse2.h>
#include <Mathematics/Cone.h>
#include <Mathematics/Ellipse3.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/Minimize1.h>
#include <Mathematics/OBBTreeOfPoints.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace gte
{
    template <typename T>
    class ApprCone3EllipseAndPoints
    {
    public:
        // The ellipse must be the intersection of a plane with the cone.
        // In an application, typically the ellipse is estimated from point
        // samples of the intersection which are then fitted with the
        // ellipse.
        //
        // The default control parameters appear to be reasonable for
        // applications, but they are exposed to the caller for tuning.

        struct Control
        {
            Control()
                :
                penalty(static_cast<T>(1)),
                maxSubdivisions(8),
                maxBisections(64),
                epsilon(static_cast<T>(1e-08)),
                tolerance(static_cast<T>(1e-04)),
                padding(static_cast<T>(1e-03))
            {
            }

            bool ValidParameters() const
            {
                T const zero = static_cast<T>(0);
                return penalty > zero
                    && maxSubdivisions > 0
                    && maxBisections > 0
                    && epsilon > zero
                    && tolerance > zero
                    && padding > zero;
            }

            // The least-squares error function is updated with the
            // penalty value for a points[i] that is below the plane
            // supporting the cone; that is, when the dot product
            // Dot(coneDirection, points[i] - coneVertex) < 0.
            T penalty;

            // Parameters for Minimizer1<T>.
            int32_t maxSubdivisions;
            int32_t maxBisections = 64;
            T epsilon = static_cast<T>(1e-08);
            T tolerance = static_cast<T>(1e-04);

            // Search for the minimum on [0 + padding, pi/2 - padding] to
            // avoid divisions by zero of the least-squares error function
            // at the endpoints of [0,pi/2].
            T padding;
        };

        static Cone3<T> Fit(Ellipse3<T> const& ellipse,
            std::vector<Vector3<T>> const& points,
            Control control = Control{})
        {
            LogAssert(
                control.ValidParameters(),
                "Invalid control parameter.");

            T sigma0{}, sigma1{};

            auto F = [&ellipse, &points, &sigma0, &sigma1, &control](T const& theta)
            {
                T const zero = static_cast<T>(0);

                auto cone = ComputeCone(theta, sigma0, sigma1, ellipse);

                T error = zero;
                for (auto const& point : points)
                {
                    auto diff = point - cone.ray.origin;
                    auto dot = Dot(cone.ray.direction, diff);
                    if (dot >= zero)
                    {
                        auto sqrLen = Dot(diff, diff);
                        auto quad = dot * dot - cone.cosAngleSqr * sqrLen;
                        auto term = quad * quad;
                        error += term;
                    }
                    else
                    {
                        error += control.penalty;
                    }
                }

                error = std::sqrt(error) / static_cast<T>(points.size());
                return error;
            };

            T const one = static_cast<T>(1);
            Minimize1<T> minimizer(F, control.maxSubdivisions,
                control.maxBisections, control.epsilon, control.tolerance);
            T t0 = static_cast<T>(control.padding);
            T t1 = static_cast<T>(GTE_C_HALF_PI) - control.padding;
            T tmin{}, fmin{};
            T minError = -one;
            Cone3<T> minCone{};

            sigma0 = one;
            sigma1 = one;
            minimizer.GetMinimum(t0, t1, tmin, fmin);
            if (t0 < tmin && tmin < t1)
            {
                if (minError == -one || fmin < minError)
                {
                    minError = fmin;
                    minCone = ComputeCone(tmin, sigma0, sigma1, ellipse);
                }
            }

            sigma0 = one;
            sigma1 = -one;
            minimizer.GetMinimum(t0, t1, tmin, fmin);
            if (t0 < tmin && tmin < t1)
            {
                if (minError == -one || fmin < minError)
                {
                    minError = fmin;
                    minCone = ComputeCone(tmin, sigma0, sigma1, ellipse);
                }
            }

            sigma0 = -one;
            sigma1 = one;
            minimizer.GetMinimum(t0, t1, tmin, fmin);
            if (t0 < tmin && tmin < t1)
            {
                if (minError == -one || fmin < minError)
                {
                    minError = fmin;
                    minCone = ComputeCone(tmin, sigma0, sigma1, ellipse);
                }
            }

            sigma0 = -one;
            sigma1 = -one;
            minimizer.GetMinimum(t0, t1, tmin, fmin);
            if (t0 < tmin && tmin < t1)
            {
                if (minError == -one || fmin < minError)
                {
                    minError = fmin;
                    minCone = ComputeCone(tmin, sigma0, sigma1, ellipse);
                }
            }

            LogAssert(
                minError != -one,
                "Failed to find fitted cone.");

            return minCone;
        }

    private:
        static Cone3<T> ComputeCone(T theta, T sigma0, T sigma1,
            Ellipse3<T> const& ellipse)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);

            auto const& C = ellipse.center;
            auto const& N = ellipse.normal;
            auto const& U = ellipse.axis[0];
            auto const& a = ellipse.extent[0];
            auto const& b = ellipse.extent[1];
            auto bDivA = b / a;
            auto eSqr = std::max(zero, one - bDivA * bDivA);
            auto omesqr = one - eSqr;
            auto e = std::sqrt(eSqr);

            auto snTheta = std::sin(theta);
            auto csTheta = std::cos(theta);
            auto snPhi = sigma0 * e * csTheta;
            auto snPhiSqr = snPhi * snPhi;
            auto csPhi = sigma1 * std::sqrt(std::max(zero, one - snPhiSqr));
            auto h = a * omesqr * csTheta / (snTheta * std::fabs(csPhi));
            auto D = csPhi * N + snPhi * U;
            auto snThetaSqr = snTheta * snTheta;
            auto csThetaSqr = csTheta * csTheta;
            auto Q = C - ((h * snPhi * snThetaSqr) / (csThetaSqr - snPhiSqr)) * U;
            auto K = Q - h * D;

            Cone3<T> cone{};
            cone.MakeInfiniteCone();
            cone.SetAngle(theta);
            cone.ray.origin = K;
            cone.ray.direction = D;
            return cone;
        }
    };

    // If the points contain only elliptical cross sections of intersection of
    // planes with the cone, extract the ellipses so that one of them can be
    // used as input to ApprCone3EllipseAndPoints.
    template <typename T>
    class ApprCone3ExtractEllipses
    {
    public:
        ApprCone3ExtractEllipses()
            :
            mBoxExtentEpsilon(static_cast<T>(0)),
            mCosAngleEpsilon(static_cast<T>(0)),
            mOBBTree{},
            mPlanes{},
            mIndices{},
            mBoxes{},
            mEllipses{}
        {
        }

        // The boxExtentEpsilon determines when a box is deemed "flat."
        // The cosAngleEpsilon is used to decide when two flat boxes are
        // in the same plane.
        void Extract(std::vector<Vector3<T>> const& points,
            T boxExtentEpsilon, T cosAngleEpsilon,
            std::vector<Ellipse3<T>>& ellipses)
        {
            T const zero = static_cast<T>(0);
            mBoxExtentEpsilon = std::max(boxExtentEpsilon, zero);
            mCosAngleEpsilon = std::max(cosAngleEpsilon, zero);
            mOBBTree.clear();
            mPlanes.clear();
            mIndices.clear();
            mBoxes.clear();
            mEllipses.clear();

            CreateOBBTree(points);
            LocatePlanes(0);
            AssociatePointsWithPlanes(points);

            mEllipses.resize(mIndices.size());
            for (size_t i = 0; i < mIndices.size(); ++i)
            {
                mEllipses[i] = ComputeEllipse(points, mIndices[i]);
            }

            ellipses = mEllipses;
        }

        // Access the ellipses extracted from the input points.
        inline std::vector<Ellipse3<T>> const& GetEllipses() const
        {
            return mEllipses;
        }

        // Accessors for informational purposes.
        inline std::vector<OBBNode<T>> const&
        GetOBBTree() const
        {
            return mOBBTree;
        }

        inline std::vector<Plane3<T>> const& GetPlanes() const
        {
            return mPlanes;
        }

        inline std::vector<std::vector<int32_t>> const& GetIndices() const
        {
            return mIndices;
        }

        inline std::vector<OrientedBox3<T>> const& GetBoxes() const
        {
            return mBoxes;
        }

    private:
        void CreateOBBTree(std::vector<Vector3<T>> const& points)
        {
            OBBTreeOfPoints<T> creator{};
            creator.Create(points);
            mOBBTree = creator.GetNodes();
        }

        void LocatePlanes(size_t nodeIndex)
        {
            auto const& node = mOBBTree[nodeIndex];
            if (node.maxIndex >= node.minIndex + 2)
            {
                for (int32_t j = 0; j < 3; ++j)
                {
                    if (node.box.extent[j] <= mBoxExtentEpsilon)
                    {
                        mBoxes.push_back(node.box);
                        Plane3<T> plane(node.box.axis[j], node.box.center);
                        ProcessPlane(plane);
                        return;
                    }
                }
            }

            if (node.leftChild != std::numeric_limits<size_t>::max())
            {
                LocatePlanes(node.leftChild);
            }

            if (node.rightChild != std::numeric_limits<size_t>::max())
            {
                LocatePlanes(node.rightChild);
            }
        }

        void ProcessPlane(Plane3<T> const& plane)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const epsilon = mCosAngleEpsilon;
            T const oneMinusEpsilon = one - epsilon;

            for (size_t i = 0; i < mPlanes.size(); ++i)
            {
                T cosAngle = Dot(plane.normal, mPlanes[i].normal);
                T absDiff{};
                if (cosAngle > zero)
                {
                    absDiff = std::fabs(plane.constant - mPlanes[i].constant);
                    if (cosAngle >= oneMinusEpsilon && absDiff <= epsilon)
                    {
                        // The planes are effectively the same.
                        return;
                    }
                }
                else
                {
                    cosAngle = -cosAngle;
                    absDiff = std::fabs(plane.constant + mPlanes[i].constant);
                    if (cosAngle >= oneMinusEpsilon && absDiff <= epsilon)
                    {
                        // The planes are effectively the same.
                        return;
                    }
                }
            }

            mPlanes.push_back(plane);
        }

        void AssociatePointsWithPlanes(std::vector<Vector3<T>> const& points)
        {
            mIndices.resize(mPlanes.size());
            for (size_t i = 0; i < points.size(); ++i)
            {
                auto const& point = points[i];
                T minDistance = std::numeric_limits<T>::max();
                size_t minJ = std::numeric_limits<size_t>::max();
                for (size_t j = 0; j < mPlanes.size(); ++j)
                {
                    auto const& plane = mPlanes[j];
                    auto diff = point - plane.origin;
                    T distance = std::fabs(Dot(plane.normal, diff));
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        minJ = j;
                    }
                }

                mIndices[minJ].push_back(static_cast<int32_t>(i));
            }
        }

        Ellipse3<T> ComputeEllipse(std::vector<Vector3<T>> const& points,
            std::vector<int32_t> const& indices)
        {
            // Fit the points with a 3D Gaussian distribution. The eigenvalues
            // are computed in nondecreasing order, which means the smallest
            // eigenvalue corresponds to the normal vector gbox.axis[0] of the
            // plane of the points. Use gbox.axis[1] and gbox.axis[2] as the
            // spanners of the plane of the point.
            ApprGaussian3<T> gfitter{};
            gfitter.FitIndexed(points.size(), points.data(),
                indices.size(), indices.data());
            auto const& gbox = gfitter.GetParameters();

            // Project the points onto the plane as 2-tuples.
            std::vector<Vector2<T>> projections(indices.size());
            for (size_t i = 0; i < indices.size(); ++i)
            {
                Vector3<T> diff = points[indices[i]] - gbox.center;
                projections[i][0] = Dot(gbox.axis[1], diff);
                projections[i][1] = Dot(gbox.axis[2], diff);
            }

            // Fit the projected points with a 2D ellipse.
            ApprEllipse2<T> efitter{};
            size_t const numIterations = 1024;
            bool const useEllipseForInitialGuess = false;
            Ellipse2<T> ellipse2{};
            efitter(projections, numIterations, useEllipseForInitialGuess, ellipse2);

            // Lift the 2D ellipse to a 3D ellipse.
            Ellipse3<T> ellipse3{};
            ellipse3.center = gbox.center +
                ellipse2.center[0] * gbox.axis[1] +
                ellipse2.center[1] * gbox.axis[2];
            ellipse3.normal = gbox.axis[0];
            ellipse3.axis[0] =
                ellipse2.axis[0][0] * gbox.axis[1] +
                ellipse2.axis[0][1] * gbox.axis[2];
            ellipse3.axis[1] =
                ellipse2.axis[1][0] * gbox.axis[1] +
                ellipse2.axis[1][1] * gbox.axis[2];
            ellipse3.extent = ellipse2.extent;
            return ellipse3;
        }

        T mBoxExtentEpsilon, mCosAngleEpsilon;
        std::vector<OBBNode<T>> mOBBTree;
        std::vector<Plane3<T>> mPlanes;
        std::vector<std::vector<int32_t>> mIndices;
        std::vector<OrientedBox3<T>> mBoxes;
        std::vector<Ellipse3<T>> mEllipses;
    };
}
