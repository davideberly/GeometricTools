// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// The surface is defined by F(x,y,z) = 0. In all member functions it is
// the application's responsibility to ensure that (x,y,z) is a solution
// to F = 0. The class is abstract, so you must derive from it and
// implement the function and derivative evaluations.
//
// The computation of principal curvature and principal directions is based
// on the document
// https://www.geometrictools.com/Documentation/PrincipalCurvature.pdf

#include <GTL/Mathematics/Algebra/Matrix.h>
#include <GTL/Mathematics/MatrixAnalysis/SymmetricEigensolver.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class ImplicitSurface3
    {
    public:
        // Abstract base class.
        virtual ~ImplicitSurface3() = default;

        // Evaluate the implicit function.
        virtual T F(Vector3<T> const& position) const = 0;

        // Evaluate the first-order partial derivatives.
        virtual T FX(Vector3<T> const& position) const = 0;
        virtual T FY(Vector3<T> const& position) const = 0;
        virtual T FZ(Vector3<T> const& position) const = 0;

        // Evaluate the second-order partial derivatives.
        virtual T FXX(Vector3<T> const& position) const = 0;
        virtual T FXY(Vector3<T> const& position) const = 0;
        virtual T FXZ(Vector3<T> const& position) const = 0;
        virtual T FYY(Vector3<T> const& position) const = 0;
        virtual T FYZ(Vector3<T> const& position) const = 0;
        virtual T FZZ(Vector3<T> const& position) const = 0;

        // Verify the point is on the surface within the tolerance specified
        // by epsilon.
        bool IsOnSurface(Vector3<T> const& position, T const& epsilon) const
        {
            return std::fabs(F(position)) <= epsilon;
        }

        // Compute all first-order derivatives.
        Vector3<T> GetGradient(Vector3<T> const& position) const
        {
            T fx = FX(position);
            T fy = FY(position);
            T fz = FZ(position);
            return Vector3<T>{ fx, fy, fz };
        }

        // Compute all second-order derivatives.
        Matrix3x3<T> GetHessian(Vector3<T> const& position) const
        {
            T fxx = FXX(position);
            T fxy = FXY(position);
            T fxz = FXZ(position);
            T fyy = FYY(position);
            T fyz = FYZ(position);
            T fzz = FZZ(position);
            return Matrix3x3<T>
            {
                { fxx, fxy, fxz },
                { fxy, fyy, fyz },
                { fxz, fyz, fzz }
            };
        }

        // Compute a coordinate frame. The set {T0,T1,N} is a right-handed
        // orthonormal basis.
        void GetFrame(Vector3<T> const& position, Vector3<T>& tangent0,
            Vector3<T>& tangent1, Vector3<T>& normal) const
        {
            normal = GetGradient(position);
            ComputeOrthonormalBasis(1, normal, tangent0, tangent1);
        }

        // Differential geometric quantities. The returned scalars are the
        // principal curvatures and the returned vectors are the corresponding
        // principal directions.
        bool GetPrincipalInformation(Vector3<T> const& position,
            T& curvature0, T& curvature1, Vector3<T>& direction0,
            Vector3<T>& direction1) const
        {
            // Compute the normal N.
            Vector3<T> normal = GetGradient(position);
            T gradientLength = Normalize(normal);
            if (gradientLength == C_<T>(0))
            {
                curvature0 = C_<T>(0);
                curvature1 = C_<T>(0);
                MakeZero(direction0);
                MakeZero(direction1);
                return false;
            }

            // Compute the matrix A.
            Matrix3x3<T> A = GetHessian(position) / gradientLength;

            // Solve for the eigensystem of equation (8) of the PDF referenced
            // at the top of this file.
            Vector3<T> tangent0{}, tangent1{};
            ComputeOrthonormalBasis(1, normal, tangent0, tangent1);
            Matrix<T, 3, 2> J{};
            J.SetCol(0, tangent0);
            J.SetCol(1, tangent1);
            Matrix2x2<T> barA = MultiplyATB(J, A * J);

            SymmetricEigensolver<T, 2> eigensolver{};
            eigensolver(barA(0, 0), barA(0, 1), barA(1, 1));
            curvature0 = eigensolver.GetEigenvalue(0);
            curvature1 = eigensolver.GetEigenvalue(1);
            Vector2<T> v0 = eigensolver.GetEigenvector(0);
            Vector2<T> v1 = eigensolver.GetEigenvector(1);
            direction0 = J * v0;
            direction1 = J * v1;
            return true;
        }

    protected:
        ImplicitSurface3() = default;
    };
}
