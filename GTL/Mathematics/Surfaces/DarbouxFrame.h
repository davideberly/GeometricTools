// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

#include <GTL/Mathematics/Algebra/Matrix.h>
#include <GTL/Mathematics/Surfaces/ParametricSurface.h>
#include <memory>

namespace gtl
{
    template <typename T>
    class DarbouxFrame3
    {
    public:
        // Get a coordinate frame, {T0, T1, N}. At a nondegenerate surface
        // points, dX/du and dX/dv are linearly independent tangent vectors.
        // The frame is constructed as
        //   T0 = (dX/du)/|dX/du|
        //   N  = Cross(dX/du,dX/dv)/|Cross(dX/du,dX/dv)|
        //   T1 = Cross(N, T0)
        // so that {T0, T1, N} is a right-handed orthonormal set.
        static void GetFrame(ParametricSurface<T, 3> const& surface, T const& u,
            T const& v, Vector3<T>& position, Vector3<T>& tangent0,
            Vector3<T>& tangent1, Vector3<T>& normal)
        {
            std::array<Vector3<T>, 3> jet;
            surface.Evaluate(u, v, 1, jet.data());
            position = jet[0];
            tangent0 = jet[1];
            Normalize(tangent0);
            tangent1 = jet[2];
            Normalize(tangent1);
            normal = UnitCross(tangent0, tangent1);
            tangent1 = Cross(normal, tangent0);
        }

        // Compute the principal curvatures and principal directions.
        static void GetPrincipalInformation(ParametricSurface<T, 3> const& surface,
            T const& u, T const& v, T& curvature0, T& curvature1,
            Vector3<T>& direction0, Vector3<T>& direction1)
        {
            // Tangents:  T0 = (x_u,y_u,z_u), T1 = (x_v,y_v,z_v)
            // Normal:    N = Cross(T0,T1)/Length(Cross(T0,T1))
            // Metric Tensor:    G = +-                      -+
            //                       | Dot(T0,T0)  Dot(T0,T1) |
            //                       | Dot(T1,T0)  Dot(T1,T1) |
            //                       +-                      -+
            //
            // Curvature Tensor:  B = +-                          -+
            //                        | -Dot(N,T0_u)  -Dot(N,T0_v) |
            //                        | -Dot(N,T1_u)  -Dot(N,T1_v) |
            //                        +-                          -+
            //
            // Principal curvatures k are the generalized eigenvalues of
            // Bw = kGw. If k is a curvature and w = (a,b) is the
            // corresponding solution to Bw = kGw, then the principal
            // direction as a 3D vector is d = a * U + b * V.
            //
            // Let k1 and k2 be the principal curvatures. The mean curvature
            // is (k1+k2)/2 and the Gaussian curvature is k1*k2.

            // Compute derivatives.
            std::array<Vector3<T>, 6> jet;
            surface.Evaluate(u, v, 2, jet.data());
            Vector3<T> derU = jet[1];
            Vector3<T> derV = jet[2];
            Vector3<T> derUU = jet[3];
            Vector3<T> derUV = jet[4];
            Vector3<T> derVV = jet[5];

            // Compute the metric tensor.
            Matrix2x2<T> metricTensor;
            metricTensor(0, 0) = Dot(jet[1], jet[1]);
            metricTensor(0, 1) = Dot(jet[1], jet[2]);
            metricTensor(1, 0) = metricTensor(0, 1);
            metricTensor(1, 1) = Dot(jet[2], jet[2]);

            // Compute the curvature tensor.
            Vector3<T> normal = UnitCross(jet[1], jet[2]);
            Matrix2x2<T> curvatureTensor;
            curvatureTensor(0, 0) = -Dot(normal, derUU);
            curvatureTensor(0, 1) = -Dot(normal, derUV);
            curvatureTensor(1, 0) = curvatureTensor(0, 1);
            curvatureTensor(1, 1) = -Dot(normal, derVV);

            // The characteristic polynomial is
            // 0 = det(B-kG) = c2 * k^2 + c1 * k + c0.
            T c0 = GetDeterminant(curvatureTensor);
            T c1 = C_<T>(2) * curvatureTensor(0, 1) * metricTensor(0, 1) -
                curvatureTensor(0, 0) * metricTensor(1, 1) -
                curvatureTensor(1, 1) * metricTensor(0, 0);
            T c2 = GetDeterminant(metricTensor);

            // The principal curvatures are the roots of the characteristic
            // polynomial.
            T temp = std::sqrt(std::max(c1 * c1 - C_<T>(4) * c0 * c2, C_<T>(0)));
            T mult = C_<T>(1, 2) / c2;
            curvature0 = -mult * (c1 + temp);  // k0
            curvature1 = -mult * (c1 - temp);  // k1

            // The principal directions are generalized eigenvectors for
            // (B - k * G) * w = 0. For principal curvature k0, the
            // generalized eigenvector is
            // (a00, a01) = (b01 - k0 * g01, -(b00 - k0 * g00))
            // or
            // (a10, a11) = (b11 - k0 * g11, -(b01 - k0 * g01)),
            // whichever has the largest length (for numerical robustness).
            T a00 = +(curvatureTensor(0, 1) - curvature0 * metricTensor(0, 1));
            T a01 = -(curvatureTensor(0, 0) - curvature0 * metricTensor(0, 0));
            T length0 = std::sqrt(a00 * a00 + a01 * a01);
            T a10 = +(curvatureTensor(1, 1) - curvature0 * metricTensor(1, 1));
            T a11 = -(curvatureTensor(0, 1) - curvature0 * metricTensor(0, 1));
            T length1 = std::sqrt(a10 * a10 + a11 * a11);
            if (length0 > C_<T>(0) || length1 > C_<T>(0))
            {
                if (length0 >= length1)
                {
                    direction0 = a00 * derU + a01 * derV;
                }
                else
                {
                    direction0 = a10 * derU + a11 * derV;
                }
            }
            else
            {
                // The point is an umbilic. Any direction is principal.
                direction0 = derU;
            }
            Normalize(direction0);

            // The second tangent is the cross product of the normal with
            // the first tangent.
            direction1 = Cross(normal, direction0);
        }
    };
}
