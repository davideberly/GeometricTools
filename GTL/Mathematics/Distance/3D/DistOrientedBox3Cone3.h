// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between an oriented box and a cone frustum. The
// frustum is part of a single-sided cone with heights measured along the
// axis direction. The single-sided cone heights h satisfy
// 0 <= h <= infinity. The cone frustum has heights that satisfy
// 0 <= hmin < h <= hmax < infinity. The algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceBox3Cone3.pdf

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Cone.h>
#include <GTL/Mathematics/Primitives/ND/OrientedBox.h>
#include <GTL/Mathematics/Minimizers/BrentsMinimizer.h>
#include <GTL/Mathematics/Minimizers/LCPSolver.h>
#include <GTL/Mathematics/Algebra/Matrix.h>
#include <array>
#include <cmath>
#include <limits>
#include <type_traits>

// TODO: Remove the dependence on the LCP solver.

namespace gtl
{
    template <typename T>
    class DCPQuery<T, OrientedBox3<T>, Cone3<T>>
    {
    public:
        DCPQuery()
            :
            mLCP{}
        {
            static_assert(
                std::is_floating_point<T>::value,
                "The input type must be a floating-point type.");
        }

        // Parameters used internally for controlling the minimizer.
        struct Control
        {
            Control(
                int32_t inMaxSubdivisions = 8,
                int32_t inMaxBisections = 128,
                T inEpsilon = static_cast<T>(1e-08),
                T inTolerance = static_cast<T>(1e-04))
                :
                maxSubdivisions(inMaxSubdivisions),
                maxBisections(inMaxBisections),
                epsilon(inEpsilon),
                tolerance(inTolerance)
            {
            }

            int32_t maxSubdivisions;
            int32_t maxBisections;
            T epsilon;
            T tolerance;
        };

        // The output of the query, which is the distance between the
        // objects and a pair of closest points, one from each object. The
        // point closest[0] is on the box and the point closest[1] is on the
        // cone.
        struct Output
        {
            Output()
                :
                distance(std::numeric_limits<T>::max()),
                closest{}
            {
            }

            T distance;
            std::array<Vector3<T>, 2> closest;
        };

        // The default minimizer controls are reasonable choices generally,
        // in which case you can use
        //   using BCQuery = DCPQuery<T, OrientedBox3<T>, Cone3<T>>;
        //   BCQuery bcQuery{};
        //   BCQuery::Output bcOutput = bcQuery(box, cone);
        // If your application requires specialized controls,
        //   using BCQuery = DCPQuery<T, OrientedBox3<T>, Cone3<T>>;
        //   BCQuery bcQuery{};
        //   BCQuery::Control bcControl(your_parameters);
        //   BCQuery::Output bcOutput = bcQuery(box, cone, &bcControl);
        Output operator()(OrientedBox3<T> const& box, Cone3<T> const& cone,
            Control const* inControl = nullptr)
        {
            Control control{};
            if (inControl != nullptr)
            {
                control = *inControl;
            }

            // Compute a basis for the cone coordinate system.
            Vector3<T> coneW0{}, coneW1{}, direction = cone.direction;
            ComputeOrthonormalBasis(1, direction, coneW0, coneW1);

            Output output{};
            output.distance = -C_<T>(1);

            auto F = [this, &box, &cone, &coneW0, &coneW1, &output](T const& angle)
            {
                T distance = -C_<T>(1);
                Vector3<T> boxClosestPoint{}, quadClosestPoint{};
                DoBoxQuadQuery(box, cone, coneW0, coneW1, angle,
                    distance, boxClosestPoint, quadClosestPoint);

                if (output.distance == -C_<T>(1) || distance < output.distance)
                {
                    output.distance = distance;
                    output.closest[0] = boxClosestPoint;
                    output.closest[1] = quadClosestPoint;
                }

                return distance;
            };

            BrentsMinimizer<T> minimizer(control.maxSubdivisions, control.maxBisections,
                control.epsilon, control.tolerance);
            T angle0 = -C_PI_DIV_2<T>;
            T angle1 = C_PI_DIV_2<T>;
            T angleMin = C_<T>(0);
            T distanceMin = -C_<T>(1);
            minimizer(F, angle0, angle1, angleMin, distanceMin);
            GTL_RUNTIME_ASSERT(
                distanceMin == output.distance,
                "Unexpected mismatch in minimum distance.");

            return output;
        }

    private:
        void DoBoxQuadQuery(OrientedBox3<T> const& box, Cone3<T> const& cone,
            Vector3<T> const& coneW0, Vector3<T> const& coneW1,
            T const& quadAngle, T& distance, Vector3<T>& boxClosestPoint,
            Vector3<T>& quadClosestPoint)
        {
            Vector3<T> K = box.center, ell{};
            for (size_t i = 0; i < 3; ++i)
            {
                K -= box.extent[i] * box.axis[i];
                ell[i] = C_<T>(2) * box.extent[i];
            }

            T cs = std::cos(quadAngle), sn = std::sin(quadAngle);
            Vector3<T> term = cone.tanAngle * (cs * coneW0 + sn * coneW1);
            std::array<Vector3<T>, 2> G{};
            G[0] = cone.direction - term;
            G[1] = cone.direction + term;

            Matrix<T, 5, 5> A{};  // zero matrix
            A(0, 0) = C_<T>(1);
            A(0, 1) = C_<T>(0);
            A(0, 2) = C_<T>(0);
            A(0, 3) = -Dot(box.axis[0], G[0]);
            A(0, 4) = -Dot(box.axis[0], G[1]);
            A(1, 0) = A(0, 1);
            A(1, 1) = C_<T>(1);
            A(1, 2) = C_<T>(0);
            A(1, 3) = -Dot(box.axis[1], G[0]);
            A(1, 4) = -Dot(box.axis[1], G[1]);
            A(2, 0) = A(0, 2);
            A(2, 1) = A(1, 2);
            A(2, 2) = C_<T>(1);
            A(2, 3) = -Dot(box.axis[2], G[0]);
            A(2, 4) = -Dot(box.axis[2], G[1]);
            A(3, 0) = A(0, 3);
            A(3, 1) = A(1, 3);
            A(3, 2) = A(2, 3);
            A(3, 3) = Dot(G[0], G[0]);
            A(3, 4) = Dot(G[0], G[1]);
            A(4, 0) = A(0, 4);
            A(4, 1) = A(1, 4);
            A(4, 2) = A(2, 4);
            A(4, 3) = A(3, 4);
            A(4, 4) = Dot(G[1], G[1]);

            Vector3<T> KmV = K - cone.vertex;
            Vector<T, 5> b{};
            b[0] = Dot(box.axis[0], KmV);
            b[1] = Dot(box.axis[1], KmV);
            b[2] = Dot(box.axis[2], KmV);
            b[3] = -Dot(G[0], KmV);
            b[4] = -Dot(G[1], KmV);

            Matrix<T, 5, 5> D{};  // zero matrix
            D(0, 0) = -C_<T>(1);
            D(1, 1) = -C_<T>(1);
            D(2, 2) = -C_<T>(1);
            D(3, 3) = C_<T>(1);
            D(3, 4) = C_<T>(1);
            D(4, 3) = -C_<T>(1);
            D(4, 4) = -C_<T>(1);

            Vector<T, 5> e{};  // zero vector
            e[0] = -ell[0];
            e[1] = -ell[1];
            e[2] = -ell[2];
            e[3] = cone.GetMinHeight();
            e[4] = -cone.GetMaxHeight();

            std::array<T, 10> q{};
            for (size_t i = 0; i < 5; ++i)
            {
                q[i] = b[i];
                q[i + 5] = -e[i];
            }

            std::array<std::array<T, 10>, 10> M{};
            for (size_t r = 0; r < 5; ++r)
            {
                for (size_t c = 0; c < 5; ++c)
                {
                    M[r][c] = A(r, c);
                    M[r + 5][c] = D(r, c);
                    M[r][c + 5] = -D(c, r);
                    M[r + 5][c + 5] = C_<T>(0);
                }
            }

            std::array<T, 10> w{}, z{};
            if (mLCP(q, M, w, z))
            {
                boxClosestPoint = K;
                for (size_t i = 0; i < 3; ++i)
                {
                    boxClosestPoint += z[i] * box.axis[i];
                }

                quadClosestPoint = cone.vertex;
                for (size_t i = 0; i < 2; ++i)
                {
                    quadClosestPoint += z[i + 3] * G[i];
                }

                distance = Length(boxClosestPoint - quadClosestPoint);
            }
            else
            {
                MakeZero(boxClosestPoint);
                MakeZero(quadClosestPoint);
                distance = -C_<T>(1);
            }
        }

        LCPSolver<T, 10> mLCP;
    };
}
