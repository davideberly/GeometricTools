// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.12.30

#pragma once

// Given a set of samples (x_i,y_i,z_i) for 0 <= i < N, and assuming
// that the true values lie on a paraboloid
//   z-c = p0*(x-a)^2 + p1*(x-a)*(y-b) + p2*(y-b)^2 + p3*(x-a) + p4*(y-b) + p5
//       = Dot(P,Q(x-a,y-b))
// where (a,b,c) is the average of the (x_i,y_i,z_i),
// P = (p0,p1,p2,p3,p4,p5), and Q(u,v) = (u^2,u*v,v^2,u,v,1),
// select P to minimize the sum of squared errors
//     E(P) = sum_{i=0}^{N-1} [Dot(P,Q_i)-z_i]^2
// where Q_i = Q(x_i-a, y_i-b).
//
// The minimum occurs when the gradient of E is the zero vector,
//     grad(E) = 2 sum_{i=0}^{N-1} [Dot(P,Q_i)-z_i] Q_i = 0
// Some algebra converts this to a system of 6 equations in 6 unknowns:
//     [(sum_{i=0}^{N-1} Q_i Q_i^t] P = sum_{i=0}^{N-1} z_i Q_i
// The product Q_i Q_i^t is a product of the 6x1 matrix Q_i with the
// 1x6 matrix Q_i^t, the result being a 6x6 matrix.
//
// Define the 6x6 symmetric matrix A = sum_{i=0}^{N-1} Q_i Q_i^t and the 6x1
// vector B = sum_{i=0}^{N-1} z_i Q_i. The choice for P is the solution to
// the linear system of equations A*P = B. The entries of A and B indicate
// summations over the appropriate product of variables. For example,
// s(x^3 y) = sum_{i=0}^{N-1} x_i^3 y_i.
//
// +-                                                     -++  +   +-      -+
// | s(x^4) s(x^3 y)   s(x^2 y^2) s(x^3)   s(x^2 y) s(x^2) ||p0|   |s(z x^2)|
// |        s(x^2 y^2) s(x y^3)   s(x^2 y) s(x y^2) s(x y) ||p1|   |s(z x y)|
// |                   s(y^4)     s(x y^2) s(y^3)   s(y^2) ||p2| = |s(z y^2)|
// |                              s(x^2)   s(x y)   s(x)   ||p3|   |s(z x)  |
// |                                       s(y^2)   s(y)   ||p4|   |s(z y)  |
// |                                                s(1)   ||p5|   |s(z)    |
// +-                                                     -++  +   +-      -+

#include <Mathematics/LinearSystem.h>
#include <Mathematics/Matrix.h>
#include <Mathematics/Vector3.h>
#include <array>
#include <cstdint>
#include <vector>

namespace gte
{
    template <typename T>
    class ApprParaboloid3
    {
    public:
        // A successful fit is indicated by return value of 'true'. Set the
        // meanSquareError to a nonnull pointer if you want the least-squares
        // error.
        static bool Fit(std::vector<Vector3<T>> const& points,
            Vector3<T>& average, std::array<T, 6>& coefficients, T* meanSquareError = nullptr)
        {
            return Fit(static_cast<int32_t>(points.size()), points.data(),
                average, coefficients, meanSquareError);
        }

        // A successful fit is indicated by return value of 'true'. Set the
        // meanSquareError to a nonnull pointer if you want the least-squares
        // error.
        static bool Fit(int32_t numPoints, Vector3<T> const* points,
            Vector3<T>& average, std::array<T, 6>& coefficients, T* meanSquareError = nullptr)
        {
            LogAssert(numPoints >= 6, "Insufficient points to fit with a paraboloid.");

            Matrix<6, 6, T> A{};  // creates the zero matrix
            Vector<6, T> B{};
            B.MakeZero();

            // Compute the mean of the points.
            T tNumPoints = static_cast<T>(numPoints);
            average = Vector3<T>::Zero();
            for (size_t i = 0; i < numPoints; ++i)
            {
                average += points[i];
            }
            average /= tNumPoints;

            for (int32_t i = 0; i < numPoints; i++)
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
                A(3, 3) += x2;
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
            A(5, 5) = static_cast<T>(numPoints);

            for (int32_t i = 0; i < 35; ++i)
            {
                A[i] /= tNumPoints;
            }
            A(5, 5) = static_cast<T>(1);

            for (int32_t i = 0; i < 6; ++i)
            {
                B[i] /= tNumPoints;
            }

            bool success = LinearSystem<T>().Solve(6, &A[0], &B[0], coefficients.data());
            if (success && meanSquareError != nullptr)
            {
                T totalError = static_cast<T>(0);
                for (size_t i = 0; i < numPoints; ++i)
                {
                    Vector3<T> diff = points[i] - average;
                    T error = std::fabs(
                        coefficients[0] * diff[0] * diff[0] +
                        coefficients[1] * diff[0] * diff[1] +
                        coefficients[2] * diff[1] * diff[1] +
                        coefficients[3] * diff[0] +
                        coefficients[4] * diff[1] +
                        coefficients[5] - diff[2]);
                    totalError += error * error;
                }
                totalError = std::sqrt(totalError * totalError) / tNumPoints;
                *meanSquareError = totalError;
            }
            return success;
        }
    };
}
