// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

#include <GTL/Mathematics/Curves/ParametricCurve.h>
#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cmath>
#include <memory>

namespace gtl
{
    template <typename T>
    class FrenetFrame2
    {
    public:
        // The normal is perpendicular to the tangent, rotated clockwise by
        // pi/2 radians.
        static void GetFrame(ParametricCurve<T, 2> const& curve, T const& t,
            Vector2<T>& position, Vector2<T>& tangent, Vector2<T>& normal)
        {
            std::array<Vector2<T>, 2> jet{};
            curve.Evaluate(t, 1, jet.data());
            position = jet[0];
            tangent = jet[1];
            Normalize(tangent);
            normal = -Perp(tangent);
        }

        static T GetCurvature(ParametricCurve<T, 2> const& curve, T const& t)
        {
            std::array<Vector2<T>, 3> jet{};
            curve.Evaluate(t, 2, jet.data());
            T speedSqr = Dot(jet[1], jet[1]);
            if (speedSqr > C_<T>(0))
            {
                T numer = DotPerp(jet[1], jet[2]);
                T denom = std::pow(speedSqr, C_<T>(3, 2));
                return numer / denom;
            }
            else
            {
                // Curvature is indeterminate, just return 0.
                return C_<T>(0);
            }
        }
    };


    template <typename T>
    class FrenetFrame3
    {
    public:
        // The binormal is Cross(tangent, normal).
        static void GetFrame(ParametricCurve<T, 3> const& curve, T const& t,
            Vector3<T>& position, Vector3<T>& tangent, Vector3<T>& normal,
            Vector3<T>& binormal)
        {
            std::array<Vector3<T>, 3> jet{};
            curve.Evaluate(t, 2, jet.data());
            position = jet[0];
            T VDotV = Dot(jet[1], jet[1]);
            T VDotA = Dot(jet[1], jet[2]);
            normal = VDotV * jet[2] - VDotA * jet[1];
            Normalize(normal);
            tangent = jet[1];
            Normalize(tangent);
            binormal = Cross(tangent, normal);
        }

        static T GetCurvature(ParametricCurve<T, 3> const& curve, T const& t)
        {
            std::array<Vector3<T>, 3> jet{};
            curve.Evaluate(t, 2, jet.data());
            T speedSqr = Dot(jet[1], jet[1]);
            if (speedSqr > C_<T>(0))
            {
                T numer = Length(Cross(jet[1], jet[2]));
                T denom = std::pow(speedSqr, C_<T>(3, 2));
                return numer / denom;
            }
            else
            {
                // Curvature is indeterminate, just return 0.
                return C_<T>(0);
            }
        }

        static T GetTorsion(ParametricCurve<T, 3> const& curve, T const& t)
        {
            std::array<Vector3<T>, 4> jet{};
            curve.Evaluate(t, 3, jet.data());
            Vector3<T> cross = Cross(jet[1], jet[2]);
            T denom = Dot(cross, cross);
            if (denom > C_<T>(0))
            {
                T numer = Dot(cross, jet[3]);
                return numer / denom;
            }
            else
            {
                // Torsion is indeterminate, just return 0.
                return C_<T>(0);
            }
        }
    };
}
