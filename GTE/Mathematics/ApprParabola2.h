// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2025.03.12

#pragma once

// The algorithm for least-squares fitting of a point set by a parabola is
// described in
//   https://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf

#include <Mathematics/LinearSystem.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gte
{
    // The code works for T in {float, double, BSRational<*>}. Unit tests
    // using BSRational<*> show that the code produces the theoretically
    // correct fit. However, if you have a large number of points and want the
    // meanSquareError, the compute time for BSRational<*> is extremely large.
    template <typename T>
    class ApprParabola2
    {
    public:
        // A successful fit is indicated by return value of 'true'. Set the
        // meanSquareError to a nonnull pointer if you want the least-squares
        // error.

        // Fit with y = u0*x^2 + u1*x + u2. The code uses a specialized 3x3
        // linear system solver. This is faster than Gaussian elimination,
        // an LDL^T decomposition, or a 3x3 eigensystem solver.
        static bool Fit(std::vector<Vector2<T>> const& points,
            std::array<T, 3>& u, T* meanSquareError = nullptr)
        {
            return Fit(static_cast<std::int32_t>(points.size()),
                points.data(), u, meanSquareError);
        }

        static bool Fit(std::int32_t numPoints, Vector2<T> const* points,
            std::array<T, 3>& u, T* meanSquareError = nullptr)
        {
            LogAssert(
                numPoints >= 3,
                "Insufficient points to fit with a parabola.");

            Matrix3x3<T> A{};  // The constructor creates the zero matrix.
            Vector3<T> B{};
            B.MakeZero();

            for (std::int32_t i = 0; i < numPoints; i++)
            {
                auto const& point = points[i];
                T x2 = point[0] * point[0];
                T x3 = point[0] * x2;
                T x4 = x2 * x2;
                T x2y = x2 * point[1];
                T xy = point[0] * point[1];

                A(0, 0) += x4;
                A(0, 1) += x3;
                A(0, 2) += x2;
                A(1, 2) += point[0];

                B[0] += x2y;
                B[1] += xy;
                B[2] += point[1];
            }

            A(1, 0) = A(0, 1);
            A(1, 1) = A(0, 2);
            A(2, 0) = A(0, 2);
            A(2, 1) = A(1, 2);
            A(2, 2) = static_cast<T>(1);

            // Scale A and B by dividing by the number of points. This
            // reduces the magnitude of the numbers to help with numerical
            // conditioning. The value A(2,2) is already scaled to 1 (the
            // i-index into A(2,2) is 8).
            T tNumPoints = static_cast<T>(numPoints);
            for (std::int32_t i = 0; i < 8; ++i)
            {
                A[i] /= tNumPoints;
            }

            for (std::int32_t i = 0; i < 3; ++i)
            {
                B[i] /= tNumPoints;
            }

            bool success = LinearSystem<T>().Solve(A, B, reinterpret_cast<Vector3<T>&>(u));
            if (success && meanSquareError != nullptr)
            {
                T totalSqrError = static_cast<T>(0);
                for (std::int32_t i = 0; i < numPoints; ++i)
                {
                    Vector2<T> const& point = points[i];
                    T error =
                        u[0] * point[0] * point[0] +
                        u[1] * point[0] +
                        u[2] - point[1];
                    totalSqrError += error * error;
                }
                *meanSquareError = std::sqrt(totalSqrError) / static_cast<T>(numPoints);
            }
            return success;
        }

        // Fit with y-b = v0*(x-a)^2 + v1*(x-a) + v2, where the average of the
        // n samples is (a,b) = [sum_{i=0}{n-1} (x_i,y_i)]/n. To convert back
        // to the u-polynomial output by Fit(...): u0 = v0, u1 = v1 - 2*v0*a,
        // and u2 = v0*a^2 - v1*a + v2 + b. FitRobust is more expensive to
        // compute than Fit, but the effect of rounding errors is mitigated.
        static bool FitRobust(std::vector<Vector2<T>> const& points,
            Vector2<T>& average, std::array<T, 3>& v, T* meanSquareError = nullptr)
        {
            return FitRobust(static_cast<std::int32_t>(points.size()),
                points.data(), average, v, meanSquareError);
        }

        static bool FitRobust(std::int32_t numPoints, Vector2<T> const* points,
            Vector2<T>& average, std::array<T, 3>& v, T* meanSquareError = nullptr)
        {
            LogAssert(
                numPoints >= 3,
                "Insufficient points to fit with a parabola.");

            Matrix3x3<T> A{};  // The constructor creates the zero matrix.
            Vector3<T> B{};
            B.MakeZero();

            // Compute the mean of the points.
            T tNumPoints = static_cast<T>(numPoints);
            average = Vector2<T>::Zero();
            for (std::int32_t i = 0; i < numPoints; ++i)
            {
                average += points[i];
            }
            average /= tNumPoints;

            for (std::int32_t i = 0; i < numPoints; i++)
            {
                Vector2<T> diff = points[i] - average;
                T x2 = diff[0] * diff[0];
                T x3 = diff[0] * x2;
                T x4 = x2 * x2;
                T x2y = x2 * diff[1];
                T xy = diff[0] * diff[1];

                A(0, 0) += x4;
                A(0, 1) += x3;
                A(0, 2) += x2;
                A(1, 2) += diff[0];

                B[0] += x2y;
                B[1] += xy;
                B[2] += diff[1];
            }

            A(1, 0) = A(0, 1);
            A(1, 1) = A(0, 2);
            A(2, 0) = A(0, 2);
            A(2, 1) = A(1, 2);
            A(2, 2) = static_cast<T>(1);

            // Scale A and B by dividing by the number of points. This
            // reduces the magnitude of the numbers to help with numerical
            // conditioning. The value A(2,2) is already scaled to 1 (the
            // i-index into A(2,2) is 8).
            for (std::int32_t i = 0; i < 8; ++i)
            {
                A[i] /= tNumPoints;
            }

            for (std::int32_t i = 0; i < 3; ++i)
            {
                B[i] /= tNumPoints;
            }

            bool success = LinearSystem<T>().Solve(A, B, reinterpret_cast<Vector3<T>&>(v));
            if (success && meanSquareError != nullptr)
            {
                T totalSqrError = static_cast<T>(0);
                for (std::int32_t i = 0; i < numPoints; ++i)
                {
                    Vector2<T> diff = points[i] - average;
                    T error =
                        v[0] * diff[0] * diff[0] +
                        v[1] * diff[0] +
                        v[2] - diff[1];
                    totalSqrError += error * error;
                }
                *meanSquareError = std::sqrt(totalSqrError) / tNumPoints;
            }
            return success;
        }
    };
}
