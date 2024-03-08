// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2023.08.08

#pragma once

// The curve is defined by F(x,y) = 0. In all member functions it is
// the application's responsibility to ensure that (x,y) is a solution
// to F = 0. The class is abstract, so you must derive from it and
// implement the function and derivative evaluations.

#include <Mathematics/Matrix2x2.h>
#include <array>
#include <cmath>

namespace gte
{
    template <typename T>
    class ImplicitCurve2
    {
    public:
        // Abstract base class.
        virtual ~ImplicitCurve2() = default;

        // Evaluate the implicit function.
        virtual T F(Vector2<T> const& position) const = 0;

        // Evaluate the first-order partial derivatives.
        virtual T FX(Vector2<T> const& position) const = 0;
        virtual T FY(Vector2<T> const& position) const = 0;

        // Evaluate the second-order partial derivatives.
        virtual T FXX(Vector2<T> const& position) const = 0;
        virtual T FXY(Vector2<T> const& position) const = 0;
        virtual T FYY(Vector2<T> const& position) const = 0;

        // Verify the point is on the curve within the tolerance specified
        // by epsilon.
        bool IsOnCurve(Vector2<T> const& position, T const& epsilon) const
        {
            return std::fabs(F(position)) <= epsilon;
        }

        // Compute all first-order derivatives.
        Vector2<T> GetGradient(Vector2<T> const& position) const
        {
            T fx = FX(position);
            T fy = FY(position);
            return Vector2<T>{ fx, fy };
        }

        // Compute all second-order derivatives.
        Matrix2x2<T> GetHessian(Vector2<T> const& position) const
        {
            T fxx = FXX(position);
            T fxy = FXY(position);
            T fyy = FYY(position);
            return Matrix2x2<T>{ fxx, fxy, fxy, fyy };
        }

        // Compute a coordinate frame. The set {T, N} is a right-handed
        // orthonormal basis.
        void GetFrame(Vector2<T> const& position, Vector2<T>& tangent,
            Vector2<T>& normal) const
        {
            std::array<Vector2<T>, 2> basis{};
            basis[0] = GetGradient(position);
            ComputeOrthogonalComplement(1, basis.data());
            tangent = basis[1];
            normal = basis[0];
        }

        // Compute the curvature at a point on the curve.
        bool GetCurvature(Vector2<T> const& position,
            T& curvature) const
        {
            // The curvature is
            // (-Fy^2*Fxx + 2*Fx*Fy*Fxy - Fx^2*Fyy) / (Fx^2+Fy^2)^{3/2}

            T const zero = static_cast<T>(0);
            T const two = static_cast<T>(2);
            T const threeHalfs = static_cast<T>(1.5);

            // Evaluate the first derivatives.
            T fx = FX(position);
            T fy = FY(position);

            // Evaluate the denominator.
            T fxSqr = fx * fx;
            T fySqr = fy * fy;
            T denom = std::pow(fxSqr + fySqr, threeHalfs);
            if (denom == zero)
            {
                curvature = zero;
                return false;
            }

            // Evaluate the second derivatives.
            T fxx = FXX(position);
            T fxy = FXY(position);
            T fyy = FYY(position);

            // Evaluate the numerator.
            T numer = -fySqr * fxx + two * fx * fy * fxy - fxSqr * fyy;

            curvature = numer / denom;
            return true;
        }

    protected:
        ImplicitCurve2() = default;
    };
}
