// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// The 3D point-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document.

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/3D/Circle3.h>
#include <array>
#include <cmath>

namespace gtl
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
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                closest{},
                equidistant(false)
            {
            }

            T distance, sqrDistance;
            std::array<Vector3<T>, 2> closest;
            bool equidistant;
        };

        Output operator()(Vector3<T> const& point, Circle3<T> const& circle)
        {
            Output output{};

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
            if (lengthScaledQmC > C_<T>(0))
            {
                output.closest[0] = point;
                output.closest[1] = circle.center + circle.radius * (scaledQmC / lengthScaledQmC);
                T height = Dot(N, PmC);
                T radial = Length(Cross(N, PmC)) - circle.radius;
                output.sqrDistance = height * height + radial * radial;
                output.distance = std::sqrt(output.sqrDistance);
                output.equidistant = false;
            }
            else
            {
                // All circle points are equidistant from P. Return one of
                // them.
                output.closest[0] = point;
                output.closest[1] = circle.center + circle.radius * GetOrthogonal(N, true);
                output.sqrDistance = Dot(PmC, PmC) + circle.radius * circle.radius;
                output.distance = std::sqrt(output.sqrDistance);
                output.equidistant = true;
            }

            return output;
        }
    };
}
