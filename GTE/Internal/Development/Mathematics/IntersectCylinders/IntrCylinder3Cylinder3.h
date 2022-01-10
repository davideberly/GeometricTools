// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#pragma once

#include <Mathematics/TIQuery.h>
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Vector3.h>

// The queries consider the cylinders to be solid.

namespace gte
{
    template <typename T>
    class TIQuery<T, Cylinder3<T>, Cylinder3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Cylinder3<T> const& cylinder0, Cylinder3<T> const& cylinder1)
        {
            T const zero = static_cast<T>(0);
            T const half = static_cast<T>(0.5);
            Result result{};  // result.intersect = false;

            // Convenient renaming for readability of the code.
            Vector3<T> const& C0 = cylinder0.axis.origin;
            Vector3<T> const& W0 = cylinder0.axis.direction;
            T const& h0 = cylinder0.height;
            T const& r0 = cylinder0.radius;
            Vector3<T> const& C1 = cylinder1.axis.origin;
            Vector3<T> const& W1 = cylinder1.axis.direction;
            T const& h1 = cylinder1.height;
            T const& r1 = cylinder1.radius;

            T h0Div2 = half * h0;
            T h1Div2 = half * h1;
            T rSum = cylinder0.radius + cylinder1.radius;
            Vector3<T> delta = C1 - C0;
            Vector3<T> W0xW1 = Cross(W0, W1);
            T lenW0xW1 = Length(W0xW1);
            T test = zero;

            if (lenW0xW1 > zero)
            {
                T absDotW0W1 = std::fabs(Dot(W0, W1));

                // Test for separation by W0.
                T absDotW0Delta = std::fabs(Dot(W0, delta));
                test = r1 * lenW0xW1 + h0Div2 + h1Div2 * absDotW0W1 - absDotW0Delta;
                if (test < zero)
                {
                    return result;
                }

                // Test for separation by W1.
                T absDotW1Delta = std::fabs(Dot(W1, delta));
                test = r0 * lenW0xW1 + h1Div2 + h0Div2 * absDotW0W1 - absDotW1Delta;
                if (test < zero)
                {
                    return result;
                }

                // Test for separation by W0xW1.
                T absDotW0xW1Delta = std::fabs(Dot(W0xW1, delta));
                test = rSum * lenW0xW1 - absDotW0xW1Delta;
                if (test < zero)
                {
                    return result;
                }

                // Test for separation by directions perpendicular to W0.
                if (SeparatedByCylinderPerpendiculars(C0, W0, r0, h0, C1, W1, r1, h1))
                {
                    return result;
                }

                // Test for separation by directions perpendicular to W1.
                if (SeparatedByCylinderPerpendiculars(C1, W1, r1, h1, C0, W0, r0, h0))
                {
                    return result;
                }

                // Test for separation by other directions.
                if (SeparatedByOtherDirections(W0, r0, h0, W1, r1, h1, delta))
                {
                    return result;
                }
            }
            else
            {
                // Test for separation by height.
                T dotW0Delta = Dot(W0, delta);
                test = h0Div2 + h1Div2 - std::fabs(dotW0Delta);
                if (test < zero)
                {
                    return result;
                }

                // Test for separation radially.
                test = rSum - Length(delta - dotW0Delta * W0);
                if (test < zero)
                {
                    return result;
                }

                // If parallel cylinders are not separated by height or radial
                // distance, then the cylinders must overlap.
            }

            result.intersect = true;
            return result;
        }

    protected:
        bool SeparatedByCylinderPerpendiculars(
            Vector3<T> const& C0, Vector3<T> const& W0, T const& r0, T const& h0,
            Vector3<T> const& C1, Vector3<T> const& W1, T const& r1, T const& h1)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const half = static_cast<T>(0.5);

            Vector3<T> delta = C1 - C0;
            T c1 = Dot(W0, W1);
            T b1 = std::sqrt(std::max(one - c1 * c1, zero));
            Vector3<T> V0 = (W1 - c1 * W0) / b1;
            Vector3<T> U0 = Cross(V0, W0);
            T h1b1Div2 = half * h1 * b1;
            T c1Sqr = c1 * c1;
            T a2 = Dot(Delta, U0);
            T b2 = Dot(Delta, V0);

            // Test directions (1-t)*U0 + t*V0.
            if (F(0, r0, r1, h1b1Div2, c1sqr, a2, b2) <= 0)
            {
                // U0 is a separating direction
                return true;
            }

            if (F(1, r0, r1, h1b1Div2, c1sqr, a2, b2) <= 0)
            {
                // V0 is a separating direction
                return true;
            }

            if (FDer(0, r0, r1, h1b1Div2, c1sqr, a2, b2) >= 0)
            {
                // no separation by perpendicular directions
                return false;
            }

            if (FDer(1, r0, r1, h1b1Div2, c1sqr, a2, b2) <= 0)
            {
                // no separation by perpendicular directions
                return false;
            }

            // Use bisection to locate t-bar for which F(t-bar) is a minimum.  The upper
            // bound maxIterations may be chosen to guarantee a specified number of digits
            // of precision in the t-variable.
            T t0, t1, fd0, fd1, tmid, fdmid;
            int i;
            t0 = 0;
            t1 = 1;
            for (i = 0; i < maxIterations; ++i)
            {
                tmid = 0.5 * (t0 + t1);
                if (F(tmid, r0, r1, h1b1Div2, c1sqr, a2, b2) <= 0)
                {
                    // (1-t)*U0 + t*V0 is a separating direction
                    return true;
                }

                fdmid = FDer(tmid, r0, r1, h1b1Div2, c1sqr, a2, b2);
                if (fdmid > 0)
                {
                    t1 = tmid;
                }
                else if (fdmid < 0)
                {
                    t0 = tmid;
                }
                else
                {
                    break;
                }
            }

            // Test directions (1-t)*(-U0) + t*V0.
            a2 = -a2;
            if (F(0, r0, r1, h1b1Div2, c1sqr, a2, b2) <= 0)
            {
                // U0 is a separating direction
                return true;
            }

            if (F(1, r0, r1, h1b1Div2, c1sqr, a2, b2) <= 0)
            {
                // V0 is a separating direction
                return true;
            }

            if (FDer(0, r0, r1, h1b1Div2, c1sqr, a2, b2) >= 0)
            {
                // no separation by perpendicular directions
                return false;
            }

            if (FDer(1, r0, r1, h1b1Div2, c1sqr, a2, b2) <= 0)
            {
                // no separation by perpendicular directions
                return false;
            }

            // Use bisection to locate t-bar for which F(t-bar) is a minimum.  The upper
            // bound maxIterations may be chosen to guarantee a specified number of digits
            // of precision in the t-variable.
            t0 = 0;
            t1 = 1;
            for (i = 0; i < maxIterations; ++i)
            {
                tmid = 0.5 * (t0 + t1);
                if (F(tmid, r0, r1, h1b1Div2, c1sqr, a2, b2) <= 0)
                {
                    // (1-t)*U0 + t*V0 is a separating direction
                    return true;
                }

                fdmid = FDer(tmid, r0, r1, h1b1Div2, c1sqr, a2, b2);
                if (fdmid > 0)
                {
                    t1 = tmid;
                }
                else if (fdmid < 0)
                {
                    t0 = tmid;
                }
                else
                {
                    break;
                }
            }
        }
    };
}
