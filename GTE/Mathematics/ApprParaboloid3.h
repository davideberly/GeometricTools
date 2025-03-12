// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2025.03.12

#pragma once

// The algorithm for least-squares fitting of a point set by a paraboloid is
// described in
//   https://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf

#include <Mathematics/LinearSystem.h>
#include <Mathematics/LDLTDecomposition.h>
#include <Mathematics/TypeTraits.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace gte
{
    // The code works for T in {float, double, BSRational<*>}. Unit tests
    // using BSRational<*> show that the code produces the theoretically
    // correct fit. However, if you have a large number of points and want the
    // meanSquareError, the compute time for BSRational<*> is extremely large.
    template <typename T>
    class ApprParaboloid3
    {
    public:
        // A successful fit is indicated by return value of 'true'. Set the
        // meanSquareError to a nonnull pointer if you want the least-squares
        // error.

        // Fit with z = u0*x^2 + u1*x*y + u2*y^2 + u3*x + u4*y + u5. The code
        // uses Gaussian elimination to solve a linear system.
        static bool Fit(std::vector<Vector3<T>> const& points,
            std::array<T, 6>& u, T* meanSquareError = nullptr)
        {
            return Fit(static_cast<std::int32_t>(points.size()),
                points.data(), u, meanSquareError);
        }

        static bool Fit(std::int32_t numPoints, Vector3<T> const* points,
            std::array<T, 6>& u, T* meanSquareError = nullptr)
        {
            LogAssert(
                numPoints >= 6,
                "Insufficient points to fit with a paraboloid.");

            Matrix<6, 6, T> A{};  // The constructor creates the zero matrix.
            Vector<6, T> B{};
            B.MakeZero();

            for (std::int32_t i = 0; i < numPoints; i++)
            {
                auto const& point = points[i];
                T x2 = point[0] * point[0];
                T xy = point[0] * point[1];
                T y2 = point[1] * point[1];
                T zx = point[2] * point[0];
                T zy = point[2] * point[1];
                T x3 = point[0] * x2;
                T x2y = x2 * point[1];
                T xy2 = point[0] * y2;
                T y3 = point[1] * y2;
                T zx2 = point[2] * x2;
                T zxy = point[2] * xy;
                T zy2 = point[2] * y2;
                T x4 = x2 * x2;
                T x3y = x3 * point[1];
                T x2y2 = x2 * y2;
                T xy3 = point[0] * y3;
                T y4 = y2 * y2;

                A(0, 0) += x4;
                A(0, 1) += x3y;
                A(0, 2) += x2y2;
                A(0, 3) += x3;
                A(0, 4) += x2y;
                A(0, 5) += x2;
                A(1, 2) += xy3;
                A(1, 4) += xy2;
                A(1, 5) += xy;
                A(2, 2) += y4;
                A(2, 4) += y3;
                A(2, 5) += y2;
                A(3, 5) += points[i][0];
                A(4, 5) += points[i][1];

                B[0] += zx2;
                B[1] += zxy;
                B[2] += zy2;
                B[3] += zx;
                B[4] += zy;
                B[5] += points[i][2];
            }

            A(1, 0) = A(0, 1);
            A(1, 1) = A(0, 2);
            A(1, 3) = A(0, 4);
            A(2, 0) = A(0, 2);
            A(2, 1) = A(1, 2);
            A(2, 3) = A(1, 4);
            A(3, 0) = A(0, 3);
            A(3, 1) = A(1, 3);
            A(3, 2) = A(2, 3);
            A(3, 3) = A(0, 5);
            A(3, 4) = A(1, 5);
            A(4, 0) = A(0, 4);
            A(4, 1) = A(1, 4);
            A(4, 2) = A(2, 4);
            A(4, 3) = A(3, 4);
            A(4, 4) = A(2, 5);
            A(5, 0) = A(0, 5);
            A(5, 1) = A(1, 5);
            A(5, 2) = A(2, 5);
            A(5, 3) = A(3, 5);
            A(5, 4) = A(4, 5);
            A(5, 5) = static_cast<T>(1);

            // Scale A and B by dividing by the number of points. This
            // reduces the magnitude of the numbers to help with numerical
            // conditioning. The value A(5,5) is already scaled to 1 (the
            // i-index into A(2,2) is 35).
            T tNumPoints = static_cast<T>(numPoints);
            for (std::int32_t i = 0; i < 35; ++i)
            {
                A[i] /= tNumPoints;
            }

            for (std::int32_t i = 0; i < 6; ++i)
            {
                B[i] /= tNumPoints;
            }

            bool success = Solve(A, B, u);
            if (success && meanSquareError != nullptr)
            {
                T totalSqrError = static_cast<T>(0);
                for (std::int32_t i = 0; i < numPoints; ++i)
                {
                    auto const& point = points[i];
                    T error =
                        u[0] * point[0] * point[0] +
                        u[1] * point[0] * point[1] +
                        u[2] * point[1] * point[1] +
                        u[3] * point[0] +
                        u[4] * point[1] +
                        u[5] - point[2];
                    totalSqrError += error * error;
                }
                *meanSquareError = std::sqrt(totalSqrError) / tNumPoints;
            }
            return success;
        }

        // Fit with z-c = v0*(x-a)^2 + v1*(x-a)*(y-b) + v2*(y-b)^2 + v3*(x-a)
        // + v4*(y-b) + v5, where the average of the n samples is (a,b,c) =
        // [sum_{i=0}^{n-1} (x_i,y_i,z_i)]/n. To convert back to the
        // u-polynomial output by Fit(...): u0 = v0, u1 = v1, u2 = v2,
        // u3 = v3 - v0 * 2 * a - v1 * b, u4 = v4 - v1 * a - v2 * 2 * b, and
        // u5 = v0 * a^2 + v1 * a * b + v2 * b^2  - v3 * a - v4 * b + v5 + c.
        // FitRobust is more expensive to compute than Fit, but the effect of
        // rounding errors is mitigated.
        static bool FitRobust(std::vector<Vector3<T>> const& points,
            Vector3<T>& average, std::array<T, 6>& v, T* meanSquareError = nullptr)
        {
            return FitRobust(static_cast<std::int32_t>(points.size()),
                points.data(), average, v, meanSquareError);
        }

        static bool FitRobust(std::int32_t numPoints, Vector3<T> const* points,
            Vector3<T>& average, std::array<T, 6>& v, T* meanSquareError = nullptr)
        {
            LogAssert(
                numPoints >= 6,
                "Insufficient points to fit with a paraboloid.");

            Matrix<6, 6, T> A{};  // The constructor creates the zero matrix.
            Vector<6, T> B{};
            B.MakeZero();

            // Compute the mean of the points.
            T tNumPoints = static_cast<T>(numPoints);
            average = Vector3<T>::Zero();
            for (std::int32_t i = 0; i < numPoints; ++i)
            {
                average += points[i];
            }
            average /= tNumPoints;

            for (std::int32_t i = 0; i < numPoints; i++)
            {
                Vector3<T> diff = points[i] - average;
                T x2 = diff[0] * diff[0];
                T xy = diff[0] * diff[1];
                T y2 = diff[1] * diff[1];
                T zx = diff[2] * diff[0];
                T zy = diff[2] * diff[1];
                T x3 = diff[0] * x2;
                T x2y = x2 * diff[1];
                T xy2 = diff[0] * y2;
                T y3 = diff[1] * y2;
                T zx2 = diff[2] * x2;
                T zxy = diff[2] * xy;
                T zy2 = diff[2] * y2;
                T x4 = x2 * x2;
                T x3y = x3 * diff[1];
                T x2y2 = x2 * y2;
                T xy3 = diff[0] * y3;
                T y4 = y2 * y2;

                A(0, 0) += x4;
                A(0, 1) += x3y;
                A(0, 2) += x2y2;
                A(0, 3) += x3;
                A(0, 4) += x2y;
                A(0, 5) += x2;
                A(1, 2) += xy3;
                A(1, 4) += xy2;
                A(1, 5) += xy;
                A(2, 2) += y4;
                A(2, 4) += y3;
                A(2, 5) += y2;
                A(3, 5) += diff[0];
                A(4, 5) += diff[1];

                B[0] += zx2;
                B[1] += zxy;
                B[2] += zy2;
                B[3] += zx;
                B[4] += zy;
                B[5] += diff[2];
            }

            A(1, 0) = A(0, 1);
            A(1, 1) = A(0, 2);
            A(1, 3) = A(0, 4);
            A(2, 0) = A(0, 2);
            A(2, 1) = A(1, 2);
            A(2, 3) = A(1, 4);
            A(3, 0) = A(0, 3);
            A(3, 1) = A(1, 3);
            A(3, 2) = A(2, 3);
            A(3, 3) = A(0, 5);
            A(3, 4) = A(1, 5);
            A(4, 0) = A(0, 4);
            A(4, 1) = A(1, 4);
            A(4, 2) = A(2, 4);
            A(4, 3) = A(3, 4);
            A(4, 4) = A(2, 5);
            A(5, 0) = A(0, 5);
            A(5, 1) = A(1, 5);
            A(5, 2) = A(2, 5);
            A(5, 3) = A(3, 5);
            A(5, 4) = A(4, 5);
            A(5, 5) = static_cast<T>(1);

            // Scale A and B by dividing by the number of points. This
            // reduces the magnitude of the numbers to help with numerical
            // conditioning. The value A(5,5) is already scaled to 1 (the
            // i-index into A(2,2) is 35).
            for (std::int32_t i = 0; i < 35; ++i)
            {
                A[i] /= tNumPoints;
            }

            for (std::int32_t i = 0; i < 6; ++i)
            {
                B[i] /= tNumPoints;
            }

            bool success = Solve(A, B, v);
            if (success && meanSquareError != nullptr)
            {
                T totalSqrError = static_cast<T>(0);
                for (std::int32_t i = 0; i < numPoints; ++i)
                {
                    Vector3<T> diff = points[i] - average;
                    T error =
                        v[0] * diff[0] * diff[0] +
                        v[1] * diff[0] * diff[1] +
                        v[2] * diff[1] * diff[1] +
                        v[3] * diff[0] +
                        v[4] * diff[1] +
                        v[5] - diff[2];
                    totalSqrError += error * error;
                }
                *meanSquareError = std::sqrt(totalSqrError) / tNumPoints;
            }
            return success;
        }

    private:
        template <typename Dummy = T>
        static typename std::enable_if<!is_arbitrary_precision<Dummy>::value, bool>::type
        Solve(Matrix<6, 6, T> const& A, Vector<6, T> const& B, std::array<T, 6>& X)
        {
            // The GaussianElimination class has dynamic memory allocation and
            // deallocation, which adds a small amount of cost to the linear
            // system solver. The templated LDLTDecomposition class does not
            // dynamic allocate/deallocate memory.
            return LDLTDecomposition<T, 6>{}.Solve(A, B, reinterpret_cast<Vector<6, T>&>(X));
        }

        template <typename Dummy = T>
        static typename std::enable_if<is_arbitrary_precision<Dummy>::value, bool>::type
        Solve(Matrix<6, 6, T> const& A, Vector<6, T> const& B, std::array<T, 6>& X)
        {
            // The LDLTDecomposition with rational arithmetic has too many
            // arithmetic operations which lead to the solver not completing
            // in a reasonable amount of time. Gaussian elimination is
            // extremely fast compared to the LDLT approach.
            return LinearSystem<T>{}.Solve(6, &A[0], &B[0], X.data());
        }
    };
}
