// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.1.2024.02.25

#pragma once

// Compute the maximum-area, fixed-aspect-ratio, and axis-aligned rectangle
// inscribed in a convex quadrilateral. The algorithm is described in
// https://www.geometrictools.com/Documentation/MaximumAreaAspectRectangle.pdf

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <utility>

namespace gte
{
    template <typename T>
    class InscribedFixedAspectRectInQuad
    {
    public:
        // The returned 'bool' is 'true' when there is a unique solution or
        // 'false' when there are infinitely many solutions. The caller is
        // responsible for the 'quad' vertices occuring in counterclockwise
        // order. The output 'rectOrigin' is (u,v), the 'rectWidth' is w,
        // and the 'rectHeight' is h. The rectangle vertices are (u,v),
        // (u + w, v), (u + w, v + h), and (u, v + h) in counterclockwise
        // order.
        static bool Execute(std::array<Vector2<T>, 4> const& quad, T const& aspectRatio,
            Vector2<T>& rectOrigin, T& rectWidth, T& rectHeight)
        {
            bool isUnique = false;

            // The i-th edge lies on a line with origin quad[i] and non-unit
            // direction edges[i] = quad[(i + 1) % 3] - quad[i]. The lines
            // containing the edges have these inner-pointing normal vectors.
            std::array<Vector2<T>, 4> normals =
            {
                Perp(quad[0] - quad[1]),
                Perp(quad[1] - quad[2]),
                Perp(quad[2] - quad[3]),
                Perp(quad[3] - quad[0])
            };

            // Compute the 4 linear inequality constraints of the form
            // Dot(N[i], R[floor(2*angle[i]/pi)] - V[i]) >= 0, where
            // V[i] is a quad vertex and N[i] is a corresponding normal.
            // The angle[i] is the angle formed by N[i] with the positive
            // x-axis and is in [0,2*pi). Each constraint is written as
            // Dot((c0,c1,c2),(u,v,w)) + c3 >= 0. In the comments for rect[],
            // r is the aspect ratio w/h.
            T const zero = static_cast<T>(0);
            T const twoPi = static_cast<T>(GTE_C_TWO_PI);
            T const invTwoDivPi = static_cast<T>(GTE_C_INV_HALF_PI);

            // The planes are of the form Dot(N,X) = c. The 'first' value is
            // the normal N and the 'second' value is the constant c.
            std::array<std::pair<Vector3<T>, T>, 4> constraints{};
            for (size_t i = 0; i < 4; ++i)
            {
                T angle = std::atan2(normals[i][1], normals[i][0]);
                if (angle < zero)
                {
                    angle += twoPi;
                }

                size_t j = static_cast<size_t>(std::floor(invTwoDivPi * angle));
                constraints[i].first[0] = normals[i][0];
                constraints[i].first[1] = normals[i][1];
                constraints[i].second = Dot(normals[i], quad[i]);
                if (j == 0)
                {
                    // rect[0] = (u, v)
                    constraints[i].first[2] = zero;
                }
                else if (j == 1)
                {
                    // rect[1] = (u, v) + (w, 0)
                    constraints[i].first[2] = normals[i][0];
                }
                else if (j == 2)
                {
                    // rect[2] = (u, v) + (w, w / r)
                    constraints[i].first[2] = normals[i][0] + normals[i][1] / aspectRatio;
                }
                else // j == 3
                {
                    // rect[3] = (u, v) + (0, w / r)
                    constraints[i].first[2] = normals[i][1] / aspectRatio;
                }
            }

            // Solve the linear programming problem.
            Vector3<T> origin{}, direction{};
            bool isLine = FindIntersection(constraints[0].first, constraints[0].second,
                constraints[2].first, constraints[2].second, origin, direction);
            LogAssert(
                isLine,
                "Unexpected condition.");

            T alpha1 = Dot(constraints[1].first, direction);
            T beta1 = Dot(constraints[1].first, origin) - constraints[1].second;
            T alpha3 = Dot(constraints[3].first, direction);
            T beta3 = Dot(constraints[3].first, origin) - constraints[3].second;
            LogAssert(
                alpha1 != zero && alpha3 != zero,
                "Unexpected condition.");

            T end1 = -beta1 / alpha1;
            bool isPositiveInfinite1 = (alpha1 > zero);
            T end3 = -beta3 / alpha3;
            bool isPositiveInfinite3 = (alpha3 > zero);
            IIResult iiResult = IIQuery{}(end1, isPositiveInfinite1, end3, isPositiveInfinite3);
            if (iiResult.type == IIResult::isFinite)
            {
                Vector3<T> solution0 = iiResult.overlap[0] * direction + origin;
                Vector3<T> solution1 = iiResult.overlap[1] * direction + origin;
                if (solution0[2] > solution1[2])
                {
                    rectOrigin[0] = solution0[0];
                    rectOrigin[1] = solution0[1];
                    rectWidth = solution0[2];
                    rectHeight = rectWidth / aspectRatio;
                }
                else
                {
                    rectOrigin[0] = solution1[0];
                    rectOrigin[1] = solution1[1];
                    rectWidth = solution1[2];
                    rectHeight = rectWidth / aspectRatio;
                }

                isUnique = (solution0[2] != solution1[2]);
            }
            else if (iiResult.type == IIResult::isPoint)
            {
                Vector3<T> solution = iiResult.overlap[0] * direction + origin;
                rectOrigin[0] = solution[0];
                rectOrigin[1] = solution[1];
                rectWidth = solution[2];
                rectHeight = rectWidth / aspectRatio;

                isUnique = true;
            }
            else if (iiResult.type == IIResult::isEmpty)
            {
                isLine = FindIntersection(constraints[1].first, constraints[1].second,
                    constraints[3].first, constraints[3].second, origin, direction);
                LogAssert(
                    isLine,
                    "Unexpected condition.");

                T alpha0 = Dot(constraints[0].first, direction);
                T beta0 = Dot(constraints[0].first, origin) - constraints[0].second;
                T alpha2 = Dot(constraints[2].first, direction);
                T beta2 = Dot(constraints[2].first, origin) - constraints[2].second;
                LogAssert(
                    alpha0 != zero && alpha2 != zero,
                    "Unexpected condition.");

                T end0 = -beta0 / alpha0;
                bool isPositiveInfinite0 = (alpha0 > zero);
                T end2 = -beta2 / alpha2;
                bool isPositiveInfinite2 = (alpha2 > zero);
                iiResult = IIQuery{}(end0, isPositiveInfinite0, end2, isPositiveInfinite2);
                if (iiResult.type == IIResult::isFinite)
                {
                    Vector3<T> solution0 = iiResult.overlap[0] * direction + origin;
                    Vector3<T> solution1 = iiResult.overlap[1] * direction + origin;
                    if (solution0[2] > solution1[2])
                    {
                        rectOrigin[0] = solution0[0];
                        rectOrigin[1] = solution0[1];
                        rectWidth = solution0[2];
                        rectHeight = rectWidth / aspectRatio;
                    }
                    else
                    {
                        rectOrigin[0] = solution1[0];
                        rectOrigin[1] = solution1[1];
                        rectWidth = solution1[2];
                        rectHeight = rectWidth / aspectRatio;
                    }

                    isUnique = (solution0[2] != solution1[2]);
                }
                else if (iiResult.type == IIResult::isPoint)
                {
                    Vector3<T> solution = iiResult.overlap[0] * direction + origin;
                    rectOrigin[0] = solution[0];
                    rectOrigin[1] = solution[1];
                    rectWidth = solution[2];
                    rectHeight = rectWidth / aspectRatio;

                    isUnique = true;
                }
                else
                {
                    LogError(
                        "Unexpected interval intersection type.");
                }
            }
            else
            {
                LogError(
                    "Unexpected interval intersection type.");
            }

            return isUnique;
        }

    private:
        using IIQuery = FIQuery<T, std::array<T, 2>, std::array<T, 2>>;
        using IIResult = typename IIQuery::Result;

        static bool FindIntersection(
            Vector3<T> const& normal0, T const& constant0,
            Vector3<T> const& normal1, T const& constant1,
            Vector3<T>& origin, Vector3<T>& direction)
        {
            // The intersection line is of the form
            // t * Cross(normal0, normal1) + a0 * normal0 + a1 * normal1

            direction = Cross(normal0, normal1);
            if (direction != Vector3<T>::Zero())
            {
                T dotN0N0 = Dot(normal0, normal0);
                T dotN0N1 = Dot(normal0, normal1);
                T dotN1N1 = Dot(normal1, normal1);
                T det = Dot(direction, direction);
                T a0 = (dotN1N1 * constant0 - dotN0N1 * constant1) / det;
                T a1 = (dotN0N0 * constant1 - dotN0N1 * constant0) / det;
                origin = a0 * normal0 + a1 * normal1;
                return true;
            }
            else
            {
                origin = Vector3<T>::Zero();
                direction = Vector3<T>::Zero();
                return false;
            }
        }
    };
}
