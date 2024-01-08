// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2023.08.08

#pragma once

// The surface is defined by F(x,y,z) = 0. In all member functions it is
// the application's responsibility to ensure that (x,y,z) is a solution
// to F = 0. The class is abstract, so you must derive from it and
// implement the function and derivative evaluations.
//
// The computation of principal curvature and principal directions is based
// on the document
// https://www.geometrictools.com/Documentation/PrincipalCurvature.pdf

#include <Mathematics/Matrix2x2.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/SymmetricEigensolver2x2.h>
#include <array>
#include <cmath>

namespace gte
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
            return Matrix3x3<T>{ fxx, fxy, fxz, fxy, fyy, fyz, fxz, fyz, fzz };
        }

        // Compute a coordinate frame. The set {T0,T1,N} is a right-handed
        // orthonormal basis.
        void GetFrame(Vector3<T> const& position, Vector3<T>& tangent0,
            Vector3<T>& tangent1, Vector3<T>& normal) const
        {
            std::array<Vector3<T>, 3> basis{};
            basis[0] = GetGradient(position);
            ComputeOrthogonalComplement(1, basis.data());
            tangent0 = basis[1];
            tangent1 = basis[2];
            normal = basis[0];
        }

        // Differential geometric quantities. The returned scalars are the
        // principal curvatures and the returned vectors are the corresponding
        // principal directions.
        bool GetPrincipalInformation(Vector3<T> const& position,
            T& curvature0, T& curvature1, Vector3<T>& direction0,
            Vector3<T>& direction1) const
        {
            // Compute the normal N.
            T const zero = static_cast<T>(0);
            Vector3<T> normal = GetGradient(position);
            T gradientLength = Normalize(normal);
            if (gradientLength == zero)
            {
                curvature0 = zero;
                curvature1 = zero;
                direction0.MakeZero();
                direction1.MakeZero();
                return false;
            }

            // Compute the matrix A.
            Matrix3x3<T> A = GetHessian(position) / gradientLength;

            // Solve for the eigensystem of equation (8) of the PDF referenced
            // at the top of this file.
            std::array<Vector3<T>, 3> basis{};
            basis[0] = normal;
            ComputeOrthogonalComplement(1, basis.data());
            // basis[1] = tangent0
            // basis[2] = tangent1
            Matrix<3, 2, T> J{};
            J.SetCol(0, basis[1]);
            J.SetCol(1, basis[2]);
            Matrix2x2<T> barA = MultiplyATB(J, A * J);

            SymmetricEigensolver2x2<T> eigensolver{};
            std::array<T, 2> eval{};
            std::array<std::array<T, 2>, 2> evec{};
            eigensolver(barA(0, 0), barA(0, 1), barA(1, 1), +1, eval, evec);
            curvature0 = eval[0];
            curvature1 = eval[1];
            Vector2<T> v0 = { evec[0][0], evec[0][1] };
            Vector2<T> v1 = { evec[1][0], evec[1][1] };
            direction0 = J * v0;
            direction1 = J * v1;
            return true;
        }

    protected:
        ImplicitSurface3() = default;
    };
}
