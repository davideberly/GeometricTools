// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.22

#pragma once

// The surface is defined by F(x,y,z) = 0. In all member functions it is
// the application's responsibility to ensure that (x,y,z) is a solution
// to F = 0. The class is abstract, so you must derive from it and
// implement the function and derivative evaluations.

#include <Mathematics/Matrix3x3.h>

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
            // Principal curvatures and directions for implicitly defined
            // surfaces F(x,y,z) = 0.
            //
            // DF = (Fx,Fy,Fz), L = Length(DF)
            //
            // D^2 F = +-           -+
            //         | Fxx Fxy Fxz |
            //         | Fxy Fyy Fyz |
            //         | Fxz Fyz Fzz |
            //         +-           -+
            //
            // adj(D^2 F) =
            //   +-                                                 -+
            //   | Fyy*Fzz-Fyz*Fyz  Fyz*Fxz-Fxy*Fzz  Fxy*Fyz-Fxz*Fyy |
            //   | Fyz*Fxz-Fxy*Fzz  Fxx*Fzz-Fxz*Fxz  Fxy*Fxz-Fxx*Fyz |
            //   | Fxy*Fyz-Fxz*Fyy  Fxy*Fxz-Fxx*Fyz  Fxx*Fyy-Fxy*Fxy |
            //   +-                                                 -+
            //
            // Gaussian curvature = [DF^t adj(D^2 F) DF]/L^4
            // 
            // Mean curvature = 0.5*[trace(D^2 F)/L - (DF^t D^2 F DF)/L^3]

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);
            T const half = static_cast<T>(0.5);

            // Evaluate the first derivatives.
            T fx = FX(position);
            T fy = FY(position);
            T fz = FZ(position);
            T fLength = std::sqrt(fx * fx + fy * fy + fz * fz);
            if (fLength == zero)
            {
                return false;
            }

            T fxfx = fx * fx;
            T fxfy = fx * fy;
            T fxfz = fx * fz;
            T fyfy = fy * fy;
            T fyfz = fy * fz;
            T fzfz = fz * fz;

            T invLength = one / fLength;
            if (invLength == zero)
            {
                return false;
            }

            T invLength2 = invLength * invLength;
            if (invLength2 == zero)
            {
                return false;
            }

            T invLength3 = invLength * invLength2;
            if (invLength3 == zero)
            {
                return false;
            }

            T invLength4 = invLength2 * invLength2;
            if (invLength4 == zero)
            {
                return false;
            }

            // Evaluate the second derivatives.
            T fxx = FXX(position);
            T fxy = FXY(position);
            T fxz = FXZ(position);
            T fyy = FYY(position);
            T fyz = FYZ(position);
            T fzz = FZZ(position);

            // mean curvature
            T meanCurv = half * invLength3 * (fxx * (fyfy + fzfz) +
                fyy * (fxfx + fzfz) + fzz * (fxfx + fyfy) - two *
                (fxy * fxfy + fxz * fxfz + fyz * fyfz));

            // Gaussian curvature
            T gaussCurv = invLength4 * (fxfx * (fyy * fzz - fyz * fyz)
                + fyfy * (fxx * fzz - fxz * fxz) + fzfz * (fxx * fyy - fxy * fxy)
                + two * (fxfy * (fxz * fyz - fxy * fzz) +
                    fxfz * (fxy * fyz - fxz * fyy) +
                    fyfz * (fxy * fxz - fxx * fyz)));

            // Solve for the principal curvatures.
            T discr = std::sqrt(std::max(meanCurv * meanCurv - gaussCurv, zero));
            curvature0 = meanCurv - discr;
            curvature1 = meanCurv + discr;

            T m00 = ((-one + fxfx * invLength2) * fxx) * invLength +
                (fxfy * fxy) * invLength3 + (fxfz * fxz) * invLength3;
            T m01 = ((-one + fxfx * invLength2) * fxy) * invLength +
                (fxfy * fyy) * invLength3 + (fxfz * fyz) * invLength3;
            T m02 = ((-one + fxfx * invLength2) * fxz) * invLength +
                (fxfy * fyz) * invLength3 + (fxfz * fzz) * invLength3;
            T m10 = (fxfy * fxx) * invLength3 +
                ((-one + fyfy * invLength2) * fxy) * invLength +
                (fyfz * fxz) * invLength3;
            T m11 = (fxfy * fxy) * invLength3 +
                ((-one + fyfy * invLength2) * fyy) * invLength +
                (fyfz * fyz) * invLength3;
            T m12 = (fxfy * fxz) * invLength3 +
                ((-one + fyfy * invLength2) * fyz) * invLength +
                (fyfz * fzz) * invLength3;
            T m20 = (fxfz * fxx) * invLength3 + (fyfz * fxy) * invLength3 +
                ((-one + fzfz * invLength2) * fxz) * invLength;
            T m21 = (fxfz * fxy) * invLength3 + (fyfz * fyy) * invLength3 +
                ((-one + fzfz * invLength2) * fyz) * invLength;
            T m22 = (fxfz * fxz) * invLength3 + (fyfz * fyz) * invLength3 +
                ((-one + fzfz * invLength2) * fzz) * invLength;

            // Solve for the principal directions.
            T tmp1 = m00 + curvature0;
            T tmp2 = m11 + curvature0;
            T tmp3 = m22 + curvature0;

            std::array<Vector3<T>, 3> U{};
            std::array<T, 3> lengths{};

            U[0][0] = m01 * m12 - m02 * tmp2;
            U[0][1] = m02 * m10 - m12 * tmp1;
            U[0][2] = tmp1 * tmp2 - m01 * m10;
            lengths[0] = Length(U[0]);

            U[1][0] = m01 * tmp3 - m02 * m21;
            U[1][1] = m02 * m20 - tmp1 * tmp3;
            U[1][2] = tmp1 * m21 - m01 * m20;
            lengths[1] = Length(U[1]);

            U[2][0] = tmp2 * tmp3 - m12 * m21;
            U[2][1] = m12 * m20 - m10 * tmp3;
            U[2][2] = m10 * m21 - m20 * tmp2;
            lengths[2] = Length(U[2]);

            size_t maxIndex = 0;
            T maxValue = lengths[0];
            if (lengths[1] > maxValue)
            {
                maxIndex = 1;
                maxValue = lengths[1];
            }
            if (lengths[2] > maxValue)
            {
                maxIndex = 2;
            }

            invLength = one / lengths[maxIndex];
            U[maxIndex] *= invLength;

            direction1 = U[maxIndex];
            direction0 = UnitCross(direction1, Vector3<T>{ fx, fy, fz });
            return true;
        }

    protected:
        ImplicitSurface3() = default;
    };
}
