// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The algorithm for least-squares fitting of a point set by a cylinder is
// described in
//   https://www.geometrictools.com/Documentation/CylinderFitting.pdf
// This document shows how to compute the cylinder radius r and the cylinder
// axis as a line C + h * W with origin C, unit-length direction W, and any
// real-valued h. The implementation here adds one addition step. It
// projects the point set onto the cylinder axis, computes the bounding
// h-interval [hmin, hmax] for the projections and sets the cylinder center
// to C + ((hmin + hmax) / 2) * W and the cylinder height to hmax - hmin.
//
// TODO: ApprCylinder3 needs to be redesigned in GTL to have at most a
// single constructor but multiple operator()(...) functions, one per
// ConstructorType.

#include <Mathematics/Constants.h>
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/SymmetricEigensolver3x3.h>
#include <Mathematics/ApprCircle2.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <thread>
#include <vector>

namespace gte
{
    template <typename T>
    class ApprCylinder3
    {
    public:
        // Search the hemisphere for a minimum, choose numThetaSamples and
        // numPhiSamples to be positive (and preferably large). These are
        // used to generate a hemispherical grid of samples to be evaluated
        // to find the cylinder axis-direction W. If the grid samples is
        // quite large and the number of points to be fitted is large, you
        // most likely will want to run multithreaded. Set numThreads to 0
        // to run single-threaded in the main process. Set numThreads > 0 to
        // run multithreaded.
        //
        // Set fitPoints to 'true' to use the algorithm described in the
        // aforementioned PDF file. Set fitPoints to 'false' if you want to
        // fit a cylinder to a triangle mesh. TODO: The PDF needs to be
        // updated to describe the new algorithm. The description is in this
        // file, before the second operator()(...) implementation.
        ApprCylinder3(size_t numThreads, size_t numThetaSamples,
            size_t numPhiSamples, bool fitPoints = true)
            :
            mConstructorType(fitPoints ?
                ConstructorType::FIT_BY_HEMISPHERE_SEARCH :
                ConstructorType::FIT_TO_MESH),
            mNumThreads(numThreads),
            mNumThetaSamples(numThetaSamples),
            mNumPhiSamples(numPhiSamples),
            mEigenIndex(0),
            mCylinderAxis{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
            mX{},
            mMu{},
            mF0{},
            mF1{},
            mF2{}
        {
        }

        // Choose one of the eigenvectors for the covariance matrix as the
        // cylinder axis direction. If eigenIndex is 0, the eigenvector
        // associated with the smallest eigenvalue is chosen. If eigenIndex
        // is 2, the eigenvector associated with the largest eigenvalue is
        // chosen. If eigenIndex is 1, the eigenvector associated with the
        // median eigenvalue is chosen; keep in mind that this could be the
        // minimum or maximum eigenvalue if the eigenspace has dimension 2
        // or 3.
        ApprCylinder3(size_t eigenIndex)
            :
            mConstructorType(ConstructorType::FIT_USING_COVARIANCE_EIGENVECTOR),
            mNumThreads(0),
            mNumThetaSamples(0),
            mNumPhiSamples(0),
            mEigenIndex(eigenIndex),
            mCylinderAxis{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
            mX{},
            mMu{},
            mF0{},
            mF1{},
            mF2{}
        {
        }

        // Choose the cylinder axis. If cylinderAxis is not the zero vector,
        // the constructor will normalize it.
        ApprCylinder3(Vector3<T> const& cylinderAxis)
            :
            mConstructorType(ConstructorType::FIT_USING_SPECIFIED_AXIS),
            mNumThreads(0),
            mNumThetaSamples(0),
            mNumPhiSamples(0),
            mEigenIndex(0),
            mCylinderAxis(cylinderAxis),
            mX{},
            mMu{},
            mF0{},
            mF1{},
            mF2{}
        {
            Normalize(mCylinderAxis, true);
        }

        // The algorithm must estimate 6 parameters, so the number of points
        // must be at least 6 but preferably larger. The returned value is
        // the root-mean-square of the least-squares error.
        T operator()(size_t numPoints, Vector3<T> const* points, Cylinder3<T>& cylinder)
        {
            LogAssert(
                mConstructorType != ConstructorType::FIT_TO_MESH,
                "Call operator()(numPoints, points, triangles, cylinder) for fitting to a mesh.");

            LogAssert(
                numPoints >= 6 && points != nullptr,
                "Fitting requires at least 6 points.");

            T const zero = static_cast<T>(0);
            mX.clear();
            cylinder.axis.origin = { zero, zero, zero };
            cylinder.axis.direction = { zero, zero, zero };
            cylinder.radius = zero;
            cylinder.height = zero;

            Vector3<T> average{ zero, zero, zero };
            Preprocess(numPoints, points, average);

            // Fit the points based on which constructor the caller used. The
            // direction is either estimated or selected directly or
            // indirectly by the caller. The center and squared radius are
            // estimated.
            Vector3<T> minPC{}, minW{};
            T minRSqr = zero, minError = zero;

            if (mConstructorType == ConstructorType::FIT_BY_HEMISPHERE_SEARCH)
            {
                LogAssert(
                    mNumThetaSamples > 0 && mNumPhiSamples > 0,
                    "The number of theta and psi samples must be positive.");

                // Search the hemisphere for the vector that leads to minimum
                // error and use it for the cylinder axis.
                if (mNumThreads == 0)
                {
                    // Execute the algorithm in the main process.
                    minError = ComputeSingleThreaded(minPC, minW, minRSqr);
                }
                else
                {
                    // Execute the algorithm in multiple threads.
                    minError = ComputeMultiThreaded(minPC, minW, minRSqr);
                }
            }
            else if (mConstructorType == ConstructorType::FIT_USING_COVARIANCE_EIGENVECTOR)
            {
                LogAssert(
                    mEigenIndex < 3,
                    "Eigenvector index is out of range.");

                // Use the eigenvector corresponding to the mEigenIndex of the
                // eigenvectors of the covariance matrix as the cylinder axis
                // direction. The eigenvectors are sorted from smallest
                // eigenvalue (mEigenIndex = 0) to largest eigenvalue
                // (mEigenIndex = 2).
                minError = ComputeUsingCovariance(minPC, minW, minRSqr);
            }
            else  // mConstructorType == ApprCylinder3<T>::FIT_USING_SPECIFIED_AXIS
            {
                LogAssert(
                    mCylinderAxis != Vector3<T>::Zero(),
                    "The cylinder axis must be nonzero.");

                minError = ComputeUsingDirection(minPC, minW, minRSqr);
            }

            // Translate back to the original space by the average of the
            // points.
            cylinder.axis.origin = minPC + average;
            cylinder.axis.direction = minW;

            // Compute the cylinder radius.
            cylinder.radius = std::sqrt(minRSqr);

            // Project the points onto the cylinder axis and choose the
            // cylinder center and cylinder height as described in the
            // comments at the top of this header file.
            T hmin = zero, hmax = zero;
            for (size_t i = 0; i < numPoints; ++i)
            {
                T h = Dot(cylinder.axis.direction, points[i] - cylinder.axis.origin);
                hmin = std::min(h, hmin);
                hmax = std::max(h, hmax);
            }

            T hmid = static_cast<T>(0.5) * (hmin + hmax);
            cylinder.axis.origin += hmid * cylinder.axis.direction;
            cylinder.height = hmax - hmin;
            return minError;
        }

        // Use this operator for fitting a cylinder to a mesh. For each
        // candidate cone axis direction on the hemisphere, project the
        // triangles onto a plane perpendicular to the axis. Compute the
        // sum of 2*area for the projected triangles. The direction that
        // minimizes this measurement is used as the cone axis direction.
        // The projected points for this direction are fit with a circle
        // whose center is used to generate the cylinder center and whose
        // radius is used for the cylinder radius. The projection of the
        // points onto the axis are used to determine the minimum and
        // maximum coordinates along the axis, which are then used to
        // compute the cone height. The cone center is adjusted to be
        // midway between the minimum and maximum coordinates along the axis.
        void operator()(size_t numPoints, Vector3<T> const* points,
            size_t numTriangles, int32_t const* indices, Cylinder3<T>& cylinder)
        {
            LogAssert(
                mConstructorType == ConstructorType::FIT_TO_MESH,
                "Call operator()(numPoints, points, cylinder) for fitting to points.");

            LogAssert(
                numPoints >= 6 &&
                points != nullptr &&
                numTriangles >= 2 &&
                indices != nullptr,
                "Fitting requires at least 6 points and 2 triangles.");

            T const zero = static_cast<T>(0);
            mX.clear();
            cylinder.axis.origin = { zero, zero, zero };
            cylinder.axis.direction = { zero, zero, zero };
            cylinder.radius = zero;
            cylinder.height = zero;

            // Translate the points by translating their average to the
            // origin. This helps with numerical stability.
            mX.resize(numPoints);
            Vector3<T> average{ zero, zero, zero };
            for (size_t i = 0; i < numPoints; ++i)
            {
                average += points[i];
            }
            average /= static_cast<T>(numPoints);
            for (size_t i = 0; i < numPoints; ++i)
            {
                mX[i] = points[i] - average;
            }

            // Search the hemisphere for the vector that leads to minimum
            // error and use it for the cylinder axis.
            if (mNumThreads == 0)
            {
                // Execute the algorithm in the main process.
                FitToMeshSingleThreaded(numPoints, mX.data(), numTriangles, indices,
                    cylinder);
            }
            else
            {
                // Execute the algorithm in multiple threads.
                FitToMeshMultiThreaded(numPoints, mX.data(), numTriangles, indices,
                    cylinder);
            }

            // Translate to the original coordinate system.
            cylinder.axis.origin += average;
        }

    private:
        enum class ConstructorType
        {
            FIT_BY_HEMISPHERE_SEARCH,
            FIT_USING_COVARIANCE_EIGENVECTOR,
            FIT_USING_SPECIFIED_AXIS,
            FIT_TO_MESH
        };

        void Preprocess(size_t numPoints, Vector3<T> const* points, Vector3<T>& average)
        {
            // Copy the points and translate by the average for numerical
            // robustness.
            mX.resize(numPoints);
            T rNumPoints = static_cast<T>(numPoints);
            average.MakeZero();
            for (size_t i = 0; i < numPoints; ++i)
            {
                average += points[i];
            }
            average /= rNumPoints;
            for (size_t i = 0; i < numPoints; ++i)
            {
                mX[i] = points[i] - average;
            }

            T const zero = static_cast<T>(0);
            T const two = static_cast<T>(2);
            Vector<6, T> vzero{ zero, zero, zero, zero, zero, zero };
            std::vector<Vector<6, T>> products(mX.size(), vzero);
            mMu = vzero;
            for (size_t i = 0; i < mX.size(); ++i)
            {
                products[i][0] = mX[i][0] * mX[i][0];
                products[i][1] = mX[i][0] * mX[i][1];
                products[i][2] = mX[i][0] * mX[i][2];
                products[i][3] = mX[i][1] * mX[i][1];
                products[i][4] = mX[i][1] * mX[i][2];
                products[i][5] = mX[i][2] * mX[i][2];
                mMu[0] += products[i][0];
                mMu[1] += two * products[i][1];
                mMu[2] += two * products[i][2];
                mMu[3] += products[i][3];
                mMu[4] += two * products[i][4];
                mMu[5] += products[i][5];
            }
            mMu /= rNumPoints;

            mF0.MakeZero();
            mF1.MakeZero();
            mF2.MakeZero();
            for (size_t i = 0; i < mX.size(); ++i)
            {
                Vector<6, T> delta{};
                delta[0] = products[i][0] - mMu[0];
                delta[1] = two * products[i][1] - mMu[1];
                delta[2] = two * products[i][2] - mMu[2];
                delta[3] = products[i][3] - mMu[3];
                delta[4] = two * products[i][4] - mMu[4];
                delta[5] = products[i][5] - mMu[5];
                mF0(0, 0) += products[i][0];
                mF0(0, 1) += products[i][1];
                mF0(0, 2) += products[i][2];
                mF0(1, 1) += products[i][3];
                mF0(1, 2) += products[i][4];
                mF0(2, 2) += products[i][5];
                mF1 += OuterProduct(mX[i], delta);
                mF2 += OuterProduct(delta, delta);
            }
            mF0 /= rNumPoints;
            mF0(1, 0) = mF0(0, 1);
            mF0(2, 0) = mF0(0, 2);
            mF0(2, 1) = mF0(1, 2);
            mF1 /= rNumPoints;
            mF2 /= rNumPoints;
        }

        T ComputeUsingDirection(Vector3<T>& minPC, Vector3<T>& minW, T& minRSqr)
        {
            minW = mCylinderAxis;
            return G(minW, minPC, minRSqr);
        }

        T ComputeUsingCovariance(Vector3<T>& minPC, Vector3<T>& minW, T& minRSqr)
        {
            Matrix3x3<T> covar{};  // zero matrix
            for (auto const& X : mX)
            {
                covar += OuterProduct(X, X);
            }
            covar /= static_cast<T>(mX.size());
            std::array<T, 3> eval{};
            std::array<std::array<T, 3>, 3> evec{};
            SymmetricEigensolver3x3<T>()(
                covar(0, 0), covar(0, 1), covar(0, 2), covar(1, 1), covar(1, 2), covar(2, 2),
                true, +1, eval, evec);
            minW = evec[mEigenIndex];
            return G(minW, minPC, minRSqr);
        }

        T ComputeSingleThreaded(Vector3<T>& minPC, Vector3<T>& minW, T& minRSqr)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const iMultiplier = static_cast<T>(GTE_C_TWO_PI) / static_cast<T>(mNumThetaSamples);
            T const jMultiplier = static_cast<T>(GTE_C_HALF_PI) / static_cast<T>(mNumPhiSamples);

            // Handle the north pole (0,0,1) separately.
            minW = { zero, zero, one };
            T minError = G(minW, minPC, minRSqr);

            for (size_t j = 1; j <= mNumPhiSamples; ++j)
            {
                T phi = jMultiplier * static_cast<T>(j);  // in [0,pi/2]
                T csphi = std::cos(phi);
                T snphi = std::sin(phi);
                for (size_t i = 0; i < mNumThetaSamples; ++i)
                {
                    T theta = iMultiplier * static_cast<T>(i);  // in [0,2*pi)
                    T cstheta = std::cos(theta);
                    T sntheta = std::sin(theta);
                    Vector3<T> W{ cstheta * snphi, sntheta * snphi, csphi };
                    Vector3<T> PC{};
                    T rsqr;
                    T error = G(W, PC, rsqr);
                    if (error < minError)
                    {
                        minError = error;
                        minRSqr = rsqr;
                        minW = W;
                        minPC = PC;
                    }
                }
            }

            return minError;
        }

        T ComputeMultiThreaded(Vector3<T>& minPC, Vector3<T>& minW, T& minRSqr)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const iMultiplier = static_cast<T>(GTE_C_TWO_PI) / static_cast<T>(mNumThetaSamples);
            T const jMultiplier = static_cast<T>(GTE_C_HALF_PI) / static_cast<T>(mNumPhiSamples);

            // Handle the north pole (0,0,1) separately.
            minW = { zero, zero, one };
            T minError = G(minW, minPC, minRSqr);

            struct Local
            {
                Local()
                    :
                    error(std::numeric_limits<T>::max()),
                    rsqr(static_cast<T>(0)),
                    W{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                    PC{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                    jmin(0),
                    jmax(0)
                {
                }

                T error;
                T rsqr;
                Vector3<T> W;
                Vector3<T> PC;
                size_t jmin;
                size_t jmax;
            };

            std::vector<Local> local(mNumThreads);
            size_t numPhiSamplesPerThread = mNumPhiSamples / mNumThreads;
            for (size_t t = 0; t < mNumThreads; ++t)
            {
                local[t].jmin = numPhiSamplesPerThread * t;
                local[t].jmax = numPhiSamplesPerThread * (t + 1);
            }
            local[mNumThreads - 1].jmax = mNumPhiSamples + 1;

            std::vector<std::thread> process(mNumThreads);
            for (size_t t = 0; t < mNumThreads; ++t)
            {
                process[t] = std::thread
                (
                    [this, t, iMultiplier, jMultiplier, &local]()
                {
                    for (size_t j = local[t].jmin; j < local[t].jmax; ++j)
                    {
                        // phi in [0,pi/2]
                        T phi = jMultiplier * static_cast<T>(j);
                        T csphi = std::cos(phi);
                        T snphi = std::sin(phi);
                        for (size_t i = 0; i < mNumThetaSamples; ++i)
                        {
                            // theta in [0,2*pi)
                            T theta = iMultiplier * static_cast<T>(i);
                            T cstheta = std::cos(theta);
                            T sntheta = std::sin(theta);
                            Vector3<T> W{ cstheta * snphi, sntheta * snphi, csphi };
                            Vector3<T> PC{};
                            T rsqr;
                            T error = G(W, PC, rsqr);
                            if (error < local[t].error)
                            {
                                local[t].error = error;
                                local[t].rsqr = rsqr;
                                local[t].W = W;
                                local[t].PC = PC;
                            }
                        }
                    }
                }
                );
            }

            for (size_t t = 0; t < mNumThreads; ++t)
            {
                process[t].join();

                if (local[t].error < minError)
                {
                    minError = local[t].error;
                    minRSqr = local[t].rsqr;
                    minW = local[t].W;
                    minPC = local[t].PC;
                }
            }

            return minError;
        }

        T G(Vector3<T> const& W, Vector3<T>& PC, T& rsqr)
        {
            T const zero = static_cast<T>(0);
            T const four = static_cast<T>(4);

            Matrix3x3<T> P = Matrix3x3<T>::Identity() - OuterProduct(W, W);
            Matrix3x3<T> S
            {
                zero, -W[2], W[1],
                W[2], zero, -W[0],
                -W[1], W[0], zero
            };

            Matrix<3, 3, T> A = P * mF0 * P;
            Matrix<3, 3, T> hatA = -(S * A * S);
            Matrix<3, 3, T> hatAA = hatA * A;
            T trace = Trace(hatAA);
            Matrix<3, 3, T> Q = hatA / trace;
            Vector<6, T> pVec{ P(0, 0), P(0, 1), P(0, 2), P(1, 1), P(1, 2), P(2, 2) };
            Vector<3, T> alpha = mF1 * pVec;
            Vector<3, T> beta = Q * alpha;
            T term0 = Dot(pVec, mF2 * pVec);
            T term1 = four * Dot(alpha, beta);
            T term2 = four * Dot(beta, mF0 * beta);
            T gValue = (term0 - term1 + term2) / static_cast<T>(mX.size());

            PC = beta;
            rsqr = Dot(pVec, mMu) + Dot(PC, PC);
            return gValue;
        }

        void FitToMeshSingleThreaded(size_t numPoints, Vector3<T> const* points,
            size_t numTriangles, int32_t const* indices, Cylinder3<T>& cylinder)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const iMultiplier = static_cast<T>(GTE_C_TWO_PI) / static_cast<T>(mNumThetaSamples);
            T const jMultiplier = static_cast<T>(GTE_C_HALF_PI) / static_cast<T>(mNumPhiSamples);

            // Handle the north pole (0,0,1) separately.
            Vector3<T> minDirection{ zero, zero, one };
            T minMeasure = GetProjectionMeasure(minDirection,
                numPoints, points, numTriangles, indices);

            // Process a regular grid of (theta,phi) angles.
            for (size_t j = 1; j <= mNumPhiSamples; ++j)
            {
                T phi = jMultiplier * static_cast<T>(j);  // in [0,pi/2]
                T csphi = std::cos(phi);
                T snphi = std::sin(phi);
                for (size_t i = 0; i < mNumThetaSamples; ++i)
                {
                    T theta = iMultiplier * static_cast<T>(i);  // in [0,2*pi)
                    T cstheta = std::cos(theta);
                    T sntheta = std::sin(theta);
                    Vector3<T> direction
                    {
                        cstheta * snphi,
                        sntheta * snphi,
                        csphi
                    };

                    T measure = GetProjectionMeasure(direction, numPoints, points,
                        numTriangles, indices);
                    if (measure < minMeasure)
                    {
                        minDirection = direction;
                        minMeasure = measure;
                    }
                }
            }

            FinishCylinder(minDirection, numPoints, points, cylinder);
        }

        void FitToMeshMultiThreaded(size_t numPoints, Vector3<T> const* points,
            size_t numTriangles, int32_t const* indices, Cylinder3<T>& cylinder)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const iMultiplier = static_cast<T>(GTE_C_TWO_PI) / static_cast<T>(mNumThetaSamples);
            T const jMultiplier = static_cast<T>(GTE_C_HALF_PI) / static_cast<T>(mNumPhiSamples);

            // Handle the north pole (0,0,1) separately.
            Vector3<T> minDirection{ zero, zero, one };
            T minMeasure = GetProjectionMeasure(minDirection, numPoints, points,
                numTriangles, indices);

            struct Local
            {
                Local()
                    :
                    direction{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                    measure{ std::numeric_limits<T>::max() },
                    jmin(0),
                    jmax(0)
                {
                }

                Vector3<T> direction;
                T measure;
                size_t jmin;
                size_t jmax;
            };

            std::vector<Local> local(mNumThreads);
            size_t numPhiSamplesPerThread = mNumPhiSamples / mNumThreads;
            for (size_t t = 0; t < mNumThreads; ++t)
            {
                local[t].jmin = numPhiSamplesPerThread * t;
                local[t].jmax = numPhiSamplesPerThread * (t + 1);
            }
            local[mNumThreads - 1].jmax = mNumPhiSamples + 1;

            std::vector<std::thread> process(mNumThreads);
            for (size_t t = 0; t < mNumThreads; ++t)
            {
                process[t] = std::thread
                (
                    [this, t, iMultiplier, jMultiplier, &local,
                        numPoints, points, numTriangles, indices]()
                    {
                        for (size_t j = local[t].jmin; j < local[t].jmax; ++j)
                        {
                            // phi in [0,pi/2]
                            T phi = jMultiplier * static_cast<T>(j);
                            T csphi = std::cos(phi);
                            T snphi = std::sin(phi);
                            for (size_t i = 0; i < mNumThetaSamples; ++i)
                            {
                                // theta in [0,2*pi)
                                T theta = iMultiplier * static_cast<T>(i);
                                T cstheta = std::cos(theta);
                                T sntheta = std::sin(theta);
                                Vector3<T> direction
                                {
                                    cstheta * snphi,
                                    sntheta * snphi,
                                    csphi
                                };

                                T measure = GetProjectionMeasure(direction,
                                    numPoints, points, numTriangles, indices);
                                if (measure < local[t].measure)
                                {
                                    local[t].direction = direction;
                                    local[t].measure = measure;
                                }
                            }
                        }
                    }
                );
            }

            for (size_t t = 0; t < mNumThreads; ++t)
            {
                process[t].join();

                if (local[t].measure < minMeasure)
                {
                    minMeasure = local[t].measure;
                    minDirection = local[t].direction;
                }
            }

            FinishCylinder(minDirection, numPoints, points, cylinder);
        }

        T GetProjectionMeasure(Vector3<T> const& direction, size_t numPoints,
            Vector3<T> const* points, size_t numTriangles, int32_t const* indices)
        {
            std::array<Vector3<T>, 3> basis{};
            basis[0] = direction;
            ComputeOrthogonalComplement(1, basis.data());
            std::vector<Vector2<T>> projections(numPoints);
            for (size_t i = 0; i < numPoints; ++i)
            {
                projections[i][0] = Dot(basis[1], points[i]);
                projections[i][1] = Dot(basis[2], points[i]);
            }

            // Add up 2*area of the triangles.
            T measure = static_cast<T>(0);
            for (size_t t = 0; t < numTriangles; ++t)
            {
                auto const& V0 = projections[*indices++];
                auto const& V1 = projections[*indices++];
                auto const& V2 = projections[*indices++];
                auto edge10 = V1 - V0;
                auto edge20 = V2 - V0;
                measure += std::fabs(DotPerp(edge10, edge20));
            }
            return measure;
        }

        void FinishCylinder(Vector3<T> const& minDirection, size_t numPoints,
            Vector3<T> const* points, Cylinder3<T>& cylinder)
        {
            std::array<Vector3<T>, 3> basis{};
            basis[0] = minDirection;
            ComputeOrthogonalComplement(1, basis.data());
            std::vector<Vector2<T>> projections(numPoints);
            T hmin = std::numeric_limits<T>::max();
            T hmax = static_cast<T>(0);
            for (size_t i = 0; i < numPoints; ++i)
            {
                T h = Dot(basis[0], points[i]);
                if (h < hmin)
                {
                    hmin = h;
                }
                if (h > hmax)
                {
                    hmax = h;
                }

                projections[i][0] = Dot(basis[1], points[i]);
                projections[i][1] = Dot(basis[2], points[i]);
            }
            ApprCircle2<T> fitter{};
            Circle2<T> circle{};
            fitter.FitUsingSquaredLengths(static_cast<int32_t>(projections.size()),
                projections.data(), circle);

            Vector3<T> minCenter = circle.center[0] * basis[1] + circle.center[1] * basis[2];
            cylinder.axis.origin = minCenter + (static_cast<T>(0.5) * (hmax + hmin)) * minDirection;
            cylinder.axis.direction = minDirection;
            cylinder.radius = circle.radius;
            cylinder.height = hmax - hmin;
        }

        ConstructorType mConstructorType;

        // Parameters for the hemisphere-search constructor.
        size_t mNumThreads;
        size_t mNumThetaSamples;
        size_t mNumPhiSamples;

        // Parameters for the eigenvector-index constructor.
        size_t mEigenIndex;

        // Parameters for the specified-axis constructor.
        Vector3<T> mCylinderAxis;

        // A copy of the input points but translated by their average for
        // numerical robustness.
        std::vector<Vector3<T>> mX;

        // Preprocessed information that depends only on the sample points.
        // This allows precomputed summations so that G(...) can be evaluated
        // extremely fast.
        Vector<6, T> mMu;
        Matrix<3, 3, T> mF0;
        Matrix<3, 6, T> mF1;
        Matrix<6, 6, T> mF2;
    };
}
