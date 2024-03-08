// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.11.27

#pragma once

// The 3D circle-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document.

#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/DCPQuery.h>
#include <Mathematics/Circle3.h>
#include <Mathematics/Rotation.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/Polynomial1.h>
#include <Mathematics/RootsGeneralPolynomial.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <set>
#include <utility>
#include <type_traits>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Circle3<T>, Circle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                numClosestPairs(0),
                circle0Closest{ Vector3<T>::Zero(), Vector3<T>::Zero() },
                circle1Closest{ Vector3<T>::Zero(), Vector3<T>::Zero() },
                equidistant(false)
            {
                static_assert(
                    std::is_floating_point<T>::value,
                    "For now, only floating-point types are supported.");
            }

            T distance, sqrDistance;
            size_t numClosestPairs;
            std::array<Vector3<T>, 2> circle0Closest, circle1Closest;
            bool equidistant;
        };

        Result operator()(Circle3<T> const& inCircle0, Circle3<T> const& inCircle1)
        {
            Result result{};

            // Transform the circles by a translation, rotation, and uniform
            // scaling so that circle1.center = (0,0,0), circle1.normal =
            // (0,0,1), circle1.radius = 1, circle0.center[0] = 0, and
            // circle0.radius = min(inCircle0.radius, inCircle1.radius) /
            // max(inCircle0.radius, inCircle1.radius). The transformation
            // is Q = scale*rotate*(P+translate). The inverse transformation
            // is P = (1/scale)*Transpose(rot)*Q-translate.
            Circle3<T> circle0{}, circle1{};
            Matrix3x3<T> rotate{};
            Vector3<T> translate{};
            T scale{};
            PrepareCircles(inCircle0, inCircle1, circle0, circle1,
                rotate, translate, scale);

            if (circle0.normal[2] < static_cast<T>(1))
            {
                // Convert the circle members to rationals. This is required
                // to avoid significant floating-point rounding errors when
                // creating the polynomials. If this is not done, the
                // polynomial root finder produces inaccurate results.
                Rational const rZero(0), rOne(1), rTwo(2);
                Vector3<Rational> rC0{ rZero, circle0.center[1], circle0.center[2] };
                Vector3<Rational> rN0{ circle0.normal[0], circle0.normal[1], circle0.normal[2] };
                Rational rR0 = circle0.radius;
                // D = -C0, U1 = (1,0,0), V1 = (0,1,0)

                // Construct the polynomial phi(cos(theta)).
                Rational rR0sqr = rR0 * rR0;
                Vector3<Rational> rN0xD = Cross(rN0, -rC0);
                Vector3<Rational> rN0xU1 = { rZero, rN0[2], -rN0[1] }; // Cross(N0,U1)
                Vector3<Rational> rN0xV1 = { -rN0[2], rZero, rN0[0] }; // Cross(N0,V1)
                Rational rA0 = -rC0[0]; // r1 * Dot(D,U1)
                Rational rA1 = -rC0[1]; // r1 * Dot(D,V1)
                Rational rA2 = Dot(rN0xD, rN0xD);
                Rational rA3 = rN0xD[1] * rN0[2] - rN0xD[2] * rN0[1]; // r1*Dot(N0xD,rN0xU1)
                Rational rA4 = rN0xD[2] * rN0[0] - rN0xD[0] * rN0[2]; // r1*Dot(N0xD,rN0xV1)
                Rational rA5 = rN0[1] * rN0[1] + rN0[2] * rN0[2]; // r1^2*Dot(N0xU1,N0xU1)
                Rational rA6 = -rN0[0] * rN0[1]; // r1^2*Dot(N0xU1,N0xV1)
                Rational rA7 = rN0[0] * rN0[0] + rN0[2] * rN0[2]; // r1^2*Dot(N0xV1,N0xV1)
                Polynomial1<Rational> rP0{ rA2 + rA7, rTwo * rA3, rA5 - rA7 };
                Polynomial1<Rational> rP1{ rTwo * rA4, rTwo * rA6 };
                Polynomial1<Rational> rP2{ rZero, rA1 };
                Polynomial1<Rational> rP3{ -rA0 };
                Polynomial1<Rational> rP4{ -rA6, rA4, rTwo * rA6 };
                Polynomial1<Rational> rP5{ -rA3, rA7 - rA5 };
                Polynomial1<Rational> rTmp0{ rOne, rZero, -rOne };
                Polynomial1<Rational> rTmp1 = rP2 * rP2 + rTmp0 * rP3 * rP3;
                Polynomial1<Rational> rTmp2 = rTwo * rP2 * rP3;
                Polynomial1<Rational> rTmp3 = rP4 * rP4 + rTmp0 * rP5 * rP5;
                Polynomial1<Rational> rTmp4 = rTwo * rP4 * rP5;
                Polynomial1<Rational> rP6 = rP0 * rTmp1 + rTmp0 * rP1 * rTmp2 - rR0sqr * rTmp3;
                Polynomial1<Rational> rP7 = rP0 * rTmp2 + rP1 * rTmp1 - rR0sqr * rTmp4;

                // Parameters for polynomial root finding. The roots[] array
                // stores the roots. We need only the unique ones, which is
                // the responsibility of the set uniqueRoots. The pairs[]
                // array stores the (cosine,sine) information mentioned in the
                // PDF.
                std::vector<Rational> rRoots{};
                std::vector<std::pair<T, T>> pairs{};
                pairs.reserve(16);

                if (rP7.GetDegree() > 0 || rP7[0].GetSign() != 0)
                {
                    // H(cs,sn) = p6(cs) + sn * p7(cs)
                    Polynomial1<Rational> rPhi = rP6 * rP6 - rTmp0 * rP7 * rP7;
                    LogAssert(rPhi.GetDegree() > 0, "Unexpected degree for phi.");

                    RootsGeneralPolynomial<T>::Solve(rPhi.GetCoefficients(), true, rRoots);
                    std::set<Rational> rUniqueRoots{};
                    for (auto const& rRoot : rRoots)
                    {
                        rUniqueRoots.insert(rRoot);
                    }

                    for (auto const& rCos : rUniqueRoots)
                    {
                        if (std::fabs(rCos) <= rOne)
                        {
                            Rational rValue = rP7(rCos);
                            if (rValue.GetSign() != 0)
                            {
                                Rational rSin = -rP6(rCos) / rValue;
                                pairs.push_back(std::make_pair(rCos, rSin));
                            }
                            else
                            {
                                Rational rSin = std::sqrt(rOne - rCos * rCos);
                                pairs.push_back(std::make_pair(rCos, rSin));
                                if (rSin.GetSign() != 0)
                                {
                                    pairs.push_back(std::make_pair(rCos, -rSin));
                                }
                            }
                        }
                    }
                }
                else
                {
                    // H(cs,sn) = p6(cs)
                    LogAssert(rP6.GetDegree() > 0, "Unexpected degree for p6.");

                    RootsGeneralPolynomial<T>::Solve(rP6.GetCoefficients(), true, rRoots);
                    std::set<Rational> rUniqueRoots{};
                    for (auto const& rRoot : rRoots)
                    {
                        rUniqueRoots.insert(rRoot);
                    }

                    for (auto const& rCos : rUniqueRoots)
                    {
                        if (std::fabs(rCos) <= rOne)
                        {
                            Rational rSin = std::sqrt(rOne - rCos * rCos);
                            pairs.push_back(std::make_pair(rCos, rSin));
                            if (rSin.GetSign() != 0)
                            {
                                pairs.push_back(std::make_pair(rCos, -rSin));
                            }
                        }
                    }
                }

                // Convert the rational values to floating-point values for
                // fast computation of the closest-point candidates.
                T const zero = static_cast<T>(0);
                //Vector3<T> U1 = { rU1[0], rU1[1], rU1[2] };
                //Vector3<T> V1 = { rV1[0], rV1[1], rV1[2] };
                std::array<ClosestInfo, 16> candidates{};
                for (size_t i = 0; i < pairs.size(); ++i)
                {
                    ClosestInfo& info = candidates[i];
                    Vector3<T> delta = circle1.center - circle0.center +
                        circle1.radius * Vector3<T>{ pairs[i].first, pairs[i].second, zero };
                    info.circle1Closest = circle0.center + delta;

                    Vector3<T> N0xDelta = Cross(circle0.normal, delta);
                    T lenN0xDelta = Length(N0xDelta);
                    if (lenN0xDelta > zero)
                    {
                        T N0dDelta = Dot(circle0.normal, delta);
                        T diff = lenN0xDelta - circle0.radius;
                        info.sqrDistance = N0dDelta * N0dDelta + diff * diff;
                        delta -= N0dDelta * circle0.normal;
                        Normalize(delta);
                        info.circle0Closest = circle0.center + circle0.radius * delta;
                        info.equidistant = false;
                    }
                    else
                    {
                        Vector3<T> U0{};
                        if (std::fabs(circle0.normal[0]) > std::fabs(circle0.normal[1]))
                        {
                            U0 = { -circle0.normal[2], zero, circle0.normal[0] };
                        }
                        else
                        {
                            U0 = { zero, circle0.normal[2], -circle0.normal[1] };
                        }
                        Normalize(U0);

                        Vector3<T> r0U0 = circle0.radius * U0;
                        Vector3<T> diff = delta - r0U0;
                        info.sqrDistance = Dot(diff, diff);
                        info.circle0Closest = circle0.center + r0U0;
                        info.equidistant = true;
                    }
                }

                std::sort(candidates.begin(), candidates.begin() + pairs.size());

                result.numClosestPairs = 1;
                result.sqrDistance = candidates[0].sqrDistance;
                result.distance = std::sqrt(result.sqrDistance);
                result.circle0Closest[0] = candidates[0].circle0Closest;
                result.circle1Closest[0] = candidates[0].circle1Closest;
                result.equidistant = candidates[0].equidistant;
                if (rRoots.size() > 1 &&
                    candidates[1].sqrDistance == candidates[0].sqrDistance)
                {
                    result.numClosestPairs = 2;
                    result.circle0Closest[1] = candidates[1].circle0Closest;
                    result.circle1Closest[1] = candidates[1].circle1Closest;
                }
            }
            else
            {
                // The planes of the circles are parallel.  Whether the planes
                // are the same or different, the problem reduces to
                // determining how two circles in the same plane are
                // separated, tangent with one circle outside the other,
                // overlapping, or one circle contained inside the other
                // circle.
                Vector3<T> D = circle1.center - circle0.center;
                DoQueryParallelPlanes(circle0, circle1, D, result);
            }

            result.distance /= scale;
            result.sqrDistance = result.distance * result.distance;
            for (size_t i = 0; i < result.numClosestPairs; ++i)
            {
                auto& closest = result.circle0Closest[i];
                closest = (closest * rotate) / scale - translate;
            }
            return result;
        }

    private:
        using Rational = BSRational<UIntegerAP32>;

        class SCPolynomial
        {
        public:
            SCPolynomial()
                :
                mPoly{}
            {
            }

            SCPolynomial(T const& oneTerm, T const& cosTerm, T const& sinTerm)
                :
                mPoly{ Polynomial1<T>{ oneTerm, cosTerm }, Polynomial1<T>{ sinTerm } }
            {
            }

            inline Polynomial1<T> const& operator[] (uint32_t i) const
            {
                return mPoly[i];
            }

            inline Polynomial1<T>& operator[] (uint32_t i)
            {
                return mPoly[i];
            }

            SCPolynomial operator+(SCPolynomial const& object) const
            {
                SCPolynomial result{};
                result.mPoly[0] = mPoly[0] + object.mPoly[0];
                result.mPoly[1] = mPoly[1] + object.mPoly[1];
                return result;
            }

            SCPolynomial operator-(SCPolynomial const& object) const
            {
                SCPolynomial result{};
                result.mPoly[0] = mPoly[0] - object.mPoly[0];
                result.mPoly[1] = mPoly[1] - object.mPoly[1];
                return result;
            }

            SCPolynomial operator*(SCPolynomial const& object) const
            {
                // 1 - c^2
                Polynomial1<T> omcsqr{ (T)1, (T)0, (T)-1 };
                SCPolynomial result{};
                result.mPoly[0] = mPoly[0] * object.mPoly[0] + omcsqr * mPoly[1] * object.mPoly[1];
                result.mPoly[1] = mPoly[0] * object.mPoly[1] + mPoly[1] * object.mPoly[0];
                return result;
            }

            SCPolynomial operator*(T scalar) const
            {
                SCPolynomial result{};
                result.mPoly[0] = scalar * mPoly[0];
                result.mPoly[1] = scalar * mPoly[1];
                return result;
            }

        private:
            // poly0(c) + s * poly1(c)
            std::array<Polynomial1<T>, 2> mPoly;
        };

        struct ClosestInfo
        {
            ClosestInfo()
                :
                sqrDistance((T)0),
                circle0Closest(Vector3<T>::Zero()),
                circle1Closest(Vector3<T>::Zero()),
                equidistant(false)
            {
            }

            T sqrDistance;
            Vector3<T> circle0Closest, circle1Closest;
            bool equidistant;

            inline bool operator< (ClosestInfo const& info) const
            {
                return sqrDistance < info.sqrDistance;
            }
        };

        // TODO: Return the transformation so we can invert it for distances
        // and closest points.
        void PrepareCircles(
            Circle3<T> const& inCircle0, Circle3<T> const& inCircle1,
            Circle3<T>& circle0, Circle3<T>& circle1,
            Matrix3x3<T>& rotate, Vector3<T>& translate, T& scale)
        {
            // Order the circles so that circle1.radius has the larger radius
            // of the two circles.
            if (inCircle0.radius <= inCircle1.radius)
            {
                circle0 = inCircle0;
                circle1 = inCircle1;
            }
            else
            {
                circle0 = inCircle1;
                circle1 = inCircle0;
            }

            // Ensure both circles have normals with z-value in [0,1].
            T const zero = static_cast<T>(0);
            if (circle0.normal[2] < zero)
            {
                circle0.normal = -circle0.normal;
            }
            if (circle1.normal[2] < zero)
            {
                circle1.normal = -circle1.normal;
            }

            // Apply a translation, rotation, and uniform scaling so that
            // circle1.center = (0,0,0), circle1.normal (0,0,1), and
            // circle1.radius = 1. A consequence is that circle0.radius
            // <= 1.
            T const one = static_cast<T>(1);
            AxisAngle<3, T> aa{};
            aa.angle = std::acos(circle1.normal[2]);
            aa.axis = UnitCross(circle1.normal, Vector3<T>::Unit(2));
            rotate = Rotation<3, T>(aa);
            translate = -circle1.center;
            scale = one / circle1.radius;

            circle0.center += translate;
            circle0.center = scale * (rotate * circle0.center);
            circle0.normal = rotate * circle0.normal;
            circle0.radius *= scale;
            circle1.center = Vector3<T>::Zero();
            circle1.normal = Vector3<T>::Unit(2);
            circle1.radius = one;

            // TODO: Rotate about circle1.normal to transform circle0.center
            // to (0,k1,k2); that is, the x-component is 0.
            if (circle0.center[0] != zero)
            {
                T length = std::sqrt(circle0.center[0] * circle0.center[0] + circle0.center[1] * circle0.center[1]);
                T sn = circle0.center[0] / length;
                T cs = circle0.center[1] / length;
                Matrix3x3<T> rot1{};
                rot1.SetCol(0, { cs, sn, 0.0 });
                rot1.SetCol(1, { -sn, cs, 0.0 });
                rot1.SetCol(2, { 0.0, 0.0, 1.0 });
                circle0.center = rot1 * circle0.center;
                circle0.center[0] = zero;
                circle0.normal = rot1 * circle0.normal;
                rotate = rot1 * rotate;
            }
        }

        // The two circles are in parallel planes where D = C1 - C0, the
        // difference of circle centers.
        void DoQueryParallelPlanes(Circle3<T> const& circle0,
            Circle3<T> const& circle1, Vector3<T> const& D, Result& result)
        {
            T N0dD = Dot(circle0.normal, D);
            Vector3<T> normProj = N0dD * circle0.normal;
            Vector3<T> compProj = D - normProj;
            Vector3<T> U = compProj;
            T d = Normalize(U);

            // The configuration is determined by the relative location of the
            // intervals of projection of the circles on to the D-line.
            // Circle0 projects to [-r0,r0] and circle1 projects to
            // [d-r1,d+r1].
            T r0 = circle0.radius, r1 = circle1.radius;
            T dmr1 = d - r1;
            T distance;
            if (dmr1 >= r0)  // d >= r0 + r1
            {
                // The circles are separated (d > r0 + r1) or tangent with one
                // outside the other (d = r0 + r1).
                distance = dmr1 - r0;
                result.numClosestPairs = 1;
                result.circle0Closest[0] = circle0.center + r0 * U;
                result.circle1Closest[0] = circle1.center - r1 * U;
                result.equidistant = false;
            }
            else // d < r0 + r1
            {
                // The cases implicitly use the knowledge that d >= 0.
                T dpr1 = d + r1;
                if (dpr1 <= r0)
                {
                    // Circle1 is inside circle0.
                    distance = r0 - dpr1;
                    result.numClosestPairs = 1;
                    if (d > (T)0)
                    {
                        result.circle0Closest[0] = circle0.center + r0 * U;
                        result.circle1Closest[0] = circle1.center + r1 * U;
                        result.equidistant = false;
                    }
                    else
                    {
                        // The circles are concentric, so U = (0,0,0).
                        // Construct a vector perpendicular to N0 to use for
                        // closest points.
                        U = GetOrthogonal(circle0.normal, true);
                        result.circle0Closest[0] = circle0.center + r0 * U;
                        result.circle1Closest[0] = circle1.center + r1 * U;
                        result.equidistant = true;
                    }
                }
                else if (dmr1 <= -r0)
                {
                    // Circle0 is inside circle1.
                    distance = -r0 - dmr1;
                    result.numClosestPairs = 1;
                    if (d > (T)0)
                    {
                        result.circle0Closest[0] = circle0.center - r0 * U;
                        result.circle1Closest[0] = circle1.center - r1 * U;
                        result.equidistant = false;
                    }
                    else
                    {
                        // The circles are concentric, so U = (0,0,0).
                        // Construct a vector perpendicular to N0 to use for
                        // closest points.
                        U = GetOrthogonal(circle0.normal, true);
                        result.circle0Closest[0] = circle0.center + r0 * U;
                        result.circle1Closest[0] = circle1.center + r1 * U;
                        result.equidistant = true;
                    }
                }
                else
                {
                    // The circles are overlapping.  The two points of
                    // intersection are C0 + s*(C1-C0) +/- h*Cross(N,U), where
                    // s = (1 + (r0^2 - r1^2)/d^2)/2 and
                    // h = sqrt(r0^2 - s^2 * d^2).
                    T r0sqr = r0 * r0, r1sqr = r1 * r1, dsqr = d * d;
                    T s = ((T)1 + (r0sqr - r1sqr) / dsqr) / (T)2;
                    T arg = std::max(r0sqr - dsqr * s * s, (T)0);
                    T h = std::sqrt(arg);
                    Vector3<T> midpoint = circle0.center + s * compProj;
                    Vector3<T> hNxU = h * Cross(circle0.normal, U);
                    distance = (T)0;
                    result.numClosestPairs = 2;
                    result.circle0Closest[0] = midpoint + hNxU;
                    result.circle0Closest[1] = midpoint - hNxU;
                    result.circle1Closest[0] = result.circle0Closest[0] + normProj;
                    result.circle1Closest[1] = result.circle0Closest[1] + normProj;
                    result.equidistant = false;
                }
            }

            result.sqrDistance = distance * distance + N0dD * N0dD;
            result.distance = std::sqrt(result.sqrDistance);
        }
    };
}
