// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The 3D point-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document.

#include <Mathematics/Logger.h>
#include <Mathematics/DCPQuery.h>
#include <Mathematics/Circle3.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Vector3<T>, Circle3<T>>
    {
    public:
        // The input point is stored in the member closest[0]. If a single
        // point on the circle is closest to the input point, the member
        // closest[1] is set to that point and the equidistant member is set
        // to false. If the entire circle is equidistant to the point, the
        // member closest[1] is set to C+r*U, where C is the circle center,
        // r is the circle radius and U is a vector perpendicular to the
        // normal N for the plane of the circles. Moreover, the equidistant
        // member is set to true.
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() },
                equidistant(false)
            {
            }

            T distance, sqrDistance;
            std::array<Vector3<T>, 2> closest;
            bool equidistant;
        };

        Result operator()(Vector3<T> const& point, Circle3<T> const& circle)
        {
            Result result{};

            // The projection of P-C onto the plane of the circle is
            // Q - C = (P - C) - Dot(N, P - C) * N. When P is nearly on the
            // normal line C + t * N, Q - C is nearly the zero vector. In this
            // case, floating-point rounding errors are a problem when the
            // closest point is computed as C + r * (Q - C) / Length(Q - C).
            // The rounding errors in Q - C are magnified by the division by
            // length, leading to an inaccurate result. Experiments indicate
            // it is better to compute an orthonormal basis {U, V, N}, where
            // the vectors are unit length and mutually perpendicular. The
            // point is P = C + x * U + y * V + z * N, with x = Dot(U, P - C),
            // y = Dot(V, Q - C) and z = Dot(N, Q - C). The projection is
            // Q = C + x * U + y * V. The computation of U and V involves
            // normalizations (divisions by square roots) which can be
            // avoided by instead computing an orthogonal basis {U, V, N},
            // where the vectors are mutually perpendicular but not required
            // to be unit length. U is computed by swapping two components
            // of N with at least one component nonzero and then negating a
            // component. V is computed as Cross(N, U). For example, if
            // N = (n0, n1, n2) with n0 != 0 or n1 != 0, then U = (-n1, n0, 0)
            // and V = (-n0*n2, -n1*n2, n0^2 + n1^2). Observe that the length
            // of V is |V| = |N|*|U|. In this case the projection is
            //   Q - C = x * U + y * V,
            //   x = Dot(U, Q - C) / Dot(U, U)
            //   y = Dot(V, Q - C) / (Dot(U, U) * Dot(N, N))
            // It is sufficient to process the scaled
            //   Dot(N, N) * Dot(U, U) * (Q - C)
            // to avoid the divisions before normalization.

            Vector3<T> PmC = point - circle.center;
            Vector3<T> U{}, V{}, N = circle.normal;
            ComputeOrthogonalBasis(1, N, U, V);
            Vector3<T> scaledQmC = (Dot(N, N) * Dot(U, PmC)) * U + Dot(V, PmC) * V;
            T lengthScaledQmC = Length(scaledQmC);
            if (lengthScaledQmC > static_cast<T>(0))
            {
                result.closest[0] = point;
                result.closest[1] = circle.center + circle.radius * (scaledQmC / lengthScaledQmC);
                T height = Dot(N, PmC);
                T radial = Length(Cross(N, PmC)) - circle.radius;
                result.sqrDistance = height * height + radial * radial;
                result.distance = std::sqrt(result.sqrDistance);
                result.equidistant = false;
            }
            else
            {
                // All circle points are equidistant from P. Return one of
                // them.
                result.closest[0] = point;
                result.closest[1] = circle.center + circle.radius * GetOrthogonal(N, true);
                result.sqrDistance = Dot(PmC, PmC) + circle.radius * circle.radius;
                result.distance = std::sqrt(result.sqrDistance);
                result.equidistant = true;
            }

            return result;
        }

    private:
        bool ComputeOrthogonalBasis(size_t numInputs, Vector3<T>& v0,
            Vector3<T>& v1, Vector3<T>& v2)
        {
            LogAssert(
                1 <= numInputs && numInputs <= 3,
                "Invalid number of inputs.");

            if (numInputs == 1)
            {
                T const zero = static_cast<T>(0);
                if (std::fabs(v0[0]) > std::fabs(v0[1]))
                {
                    v1 = { -v0[2], zero, v0[0] };
                }
                else
                {
                    v1 = { zero, v0[2], -v0[1] };
                }
            }
            else // numInputs == 2 || numInputs == 3
            {
                v1 = Dot(v0, v0) * v1 - Dot(v1, v0) * v0;
            }

            if (v1 == Vector3<T>::Zero())
            {
                v2.MakeZero();
                return false;
            }

            v2 = Cross(v0, v1);
            return v2 != Vector3<T>::Zero();
        }
    };
}
