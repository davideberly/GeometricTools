// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// The 3D circle-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document.

#include <GTL/Mathematics/Algebra/Polynomial.h>
#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/3D/Circle3.h>
#include <GTL/Mathematics/RootFinders/RootsPolynomial.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Circle3<T>, Circle3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                numClosestPairs(0),
                circle0Closest{},
                circle1Closest{},
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

        Output operator()(Circle3<T> const& circle0, Circle3<T> const& circle1)
        {
            Output output{};

            Vector3<T> N0 = circle0.normal, N1 = circle1.normal;
            T r0 = circle0.radius, r1 = circle1.radius;
            Vector3<T> D = circle1.center - circle0.center;
            Vector3<T> N0xN1 = Cross(N0, N1);

            if (N0xN1 != Vector3<T>{})  // comparison to the zero vector
            {
                // Get parameters for constructing the degree-8 polynomial phi.
                T r0sqr = r0 * r0, r1sqr = r1 * r1;

                // Compute U1 and V1 for the plane of circle1.
                Vector3<T> U1{}, V1{};
                ComputeOrthonormalBasis(1, N1, U1, V1);

                // Construct the polynomial phi(cos(theta)).
                Vector3<T> N0xD = Cross(N0, D);
                Vector3<T> N0xU1 = Cross(N0, U1), N0xV1 = Cross(N0, V1);
                T a0 = r1 * Dot(D, U1), a1 = r1 * Dot(D, V1);
                T a2 = Dot(N0xD, N0xD), a3 = r1 * Dot(N0xD, N0xU1);
                T a4 = r1 * Dot(N0xD, N0xV1), a5 = r1sqr * Dot(N0xU1, N0xU1);
                T a6 = r1sqr * Dot(N0xU1, N0xV1), a7 = r1sqr * Dot(N0xV1, N0xV1);
                Polynomial1<T> p0{ a2 + a7, C_<T>(2) * a3, a5 - a7 };
                Polynomial1<T> p1{ C_<T>(2) * a4, C_<T>(2) * a6 };
                Polynomial1<T> p2{ C_<T>(0), a1 };
                Polynomial1<T> p3{ -a0 };
                Polynomial1<T> p4{ -a6, a4, C_<T>(2) * a6 };
                Polynomial1<T> p5{ -a3, a7 - a5 };
                Polynomial1<T> tmp0{ C_<T>(1), C_<T>(0), -C_<T>(1) };
                Polynomial1<T> tmp1 = p2 * p2 + tmp0 * p3 * p3;
                Polynomial1<T> tmp2 = C_<T>(2) * p2 * p3;
                Polynomial1<T> tmp3 = p4 * p4 + tmp0 * p5 * p5;
                Polynomial1<T> tmp4 = C_<T>(2) * p4 * p5;
                Polynomial1<T> p6 = p0 * tmp1 + tmp0 * p1 * tmp2 - r0sqr * tmp3;
                Polynomial1<T> p7 = p0 * tmp2 + p1 * tmp1 - r0sqr * tmp4;

                // The use of 'double' is intentional in case T is a BSNumber
                // or BSRational type. We want the bisections to terminate in
                // a reasonable amount of time.
                size_t constexpr maxBisections = 2048;
                size_t constexpr precision = std::numeric_limits<double>::digits;
                RootsPolynomial finder(maxBisections, precision);
                std::vector<RootsPolynomial::BSN> roots(8);

                // We need only the unique roots. The multiplicities are
                // irrelevant.
                std::set<T> uniqueRoots{};

                std::array<std::pair<T, T>, 16> pairs{};
                size_t numPairs = 0;
                if (p7.GetDegree() > 0 || p7[0] != C_<T>(0))
                {
                    // H(cs,sn) = p6(cs) + sn * p7(cs)
                    Polynomial1<T> phi = p6 * p6 - tmp0 * p7 * p7;
                    GTL_RUNTIME_ASSERT(
                        phi.GetDegree() > 0,
                        "Unexpected degree for phi.");

                    finder(phi, roots);
                    for (size_t i = 0; i < roots.size(); ++i)
                    {
                        uniqueRoots.insert(roots[i]);
                    }

                    for (auto const& cs : uniqueRoots)
                    {
                        if (std::fabs(cs) <= C_<T>(1))
                        {
                            T temp = p7(cs);
                            if (temp != C_<T>(0))
                            {
                                T sn = -p6(cs) / temp;
                                pairs[numPairs++] = std::make_pair(cs, sn);
                            }
                            else
                            {
                                temp = std::max(C_<T>(1) - cs * cs, C_<T>(0));
                                T sn = std::sqrt(temp);
                                pairs[numPairs++] = std::make_pair(cs, sn);
                                if (sn != C_<T>(0))
                                {
                                    pairs[numPairs++] = std::make_pair(cs, -sn);
                                }
                            }
                        }
                    }
                }
                else
                {
                    // H(cs,sn) = p6(cs)
                    GTL_RUNTIME_ASSERT(
                        p6.GetDegree() > 0,
                        "Unexpected degree for p6.");

                    finder(p6, roots);
                    for (size_t i = 0; i < roots.size(); ++i)
                    {
                        uniqueRoots.insert(roots[i]);
                    }

                    for (auto const& cs : uniqueRoots)
                    {
                        if (std::fabs(cs) <= C_<T>(1))
                        {
                            T temp = std::max(C_<T>(1) - cs * cs, C_<T>(0));
                            T sn = std::sqrt(temp);
                            pairs[numPairs++] = std::make_pair(cs, sn);
                            if (sn != C_<T>(0))
                            {
                                pairs[numPairs++] = std::make_pair(cs, -sn);
                            }
                        }
                    }
                }

                std::array<ClosestInfo, 16> candidates{};
                for (size_t i = 0; i < numPairs; ++i)
                {
                    ClosestInfo& info = candidates[i];
                    Vector3<T> delta = D + r1 * (pairs[i].first * U1 + pairs[i].second * V1);
                    info.circle1Closest = circle0.center + delta;
                    T N0dDelta = Dot(N0, delta);
                    T lenN0xDelta = Length(Cross(N0, delta));
                    if (lenN0xDelta > C_<T>(0))
                    {
                        T diff = lenN0xDelta - r0;
                        info.sqrDistance = N0dDelta * N0dDelta + diff * diff;
                        delta -= N0dDelta * circle0.normal;
                        Normalize(delta);
                        info.circle0Closest = circle0.center + r0 * delta;
                        info.equidistant = false;
                    }
                    else
                    {
                        Vector3<T> r0U0 = r0 * GetOrthogonal(N0, true);
                        Vector3<T> diff = delta - r0U0;
                        info.sqrDistance = Dot(diff, diff);
                        info.circle0Closest = circle0.center + r0U0;
                        info.equidistant = true;
                    }
                }

                std::sort(candidates.begin(), candidates.begin() + numPairs);

                output.numClosestPairs = 1;
                output.sqrDistance = candidates[0].sqrDistance;
                output.circle0Closest[0] = candidates[0].circle0Closest;
                output.circle1Closest[0] = candidates[0].circle1Closest;
                output.equidistant = candidates[0].equidistant;
                if (roots.size() > 1 &&
                    candidates[1].sqrDistance == candidates[0].sqrDistance)
                {
                    output.numClosestPairs = 2;
                    output.circle0Closest[1] = candidates[1].circle0Closest;
                    output.circle1Closest[1] = candidates[1].circle1Closest;
                }
            }
            else
            {
                // The planes of the circles are parallel. Whether the planes
                // are the same or different, the problem reduces to
                // determining how two circles in the same plane are
                // separated, tangent with one circle outside the other,
                // overlapping or one circle contained inside the other
                // circle.
                DoQueryParallelPlanes(circle0, circle1, D, output);
            }

            output.distance = std::sqrt(output.sqrDistance);
            return output;
        }

    private:
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

            inline Polynomial1<T> const& operator[] (size_t i) const
            {
                return mPoly[i];
            }

            inline Polynomial1<T>& operator[] (size_t i)
            {
                return mPoly[i];
            }

            SCPolynomial operator+(SCPolynomial const& object) const
            {
                SCPolynomial output{};
                output.mPoly[0] = mPoly[0] + object.mPoly[0];
                output.mPoly[1] = mPoly[1] + object.mPoly[1];
                return output;
            }

            SCPolynomial operator-(SCPolynomial const& object) const
            {
                SCPolynomial output{};
                output.mPoly[0] = mPoly[0] - object.mPoly[0];
                output.mPoly[1] = mPoly[1] - object.mPoly[1];
                return output;
            }

            SCPolynomial operator*(SCPolynomial const& object) const
            {
                // 1 - c^2
                Polynomial1<T> omcsqr{ C_<T>(1), C_<T>(0), -C_<T>(1) };
                SCPolynomial output{};
                output.mPoly[0] = mPoly[0] * object.mPoly[0] + omcsqr * mPoly[1] * object.mPoly[1];
                output.mPoly[1] = mPoly[0] * object.mPoly[1] + mPoly[1] * object.mPoly[0];
                return output;
            }

            SCPolynomial operator*(T scalar) const
            {
                SCPolynomial output{};
                output.mPoly[0] = scalar * mPoly[0];
                output.mPoly[1] = scalar * mPoly[1];
                return output;
            }

        private:
            // poly0(c) + s * poly1(c)
            std::array<Polynomial1<T>, 2> mPoly;
        };

        struct ClosestInfo
        {
            ClosestInfo()
                :
                sqrDistance(C_<T>(0)),
                circle0Closest{},
                circle1Closest{},
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

        // The two circles are in parallel planes where D = C1 - C0, the
        // difference of circle centers.
        void DoQueryParallelPlanes(Circle3<T> const& circle0,
            Circle3<T> const& circle1, Vector3<T> const& D, Output& output)
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
            T distance{};
            if (dmr1 >= r0)  // d >= r0 + r1
            {
                // The circles are separated (d > r0 + r1) or tangent with one
                // outside the other (d = r0 + r1).
                distance = dmr1 - r0;
                output.numClosestPairs = 1;
                output.circle0Closest[0] = circle0.center + r0 * U;
                output.circle1Closest[0] = circle1.center - r1 * U;
                output.equidistant = false;
            }
            else // d < r0 + r1
            {
                // The cases implicitly use the knowledge that d >= 0.
                T dpr1 = d + r1;
                if (dpr1 <= r0)
                {
                    // Circle1 is inside circle0.
                    distance = r0 - dpr1;
                    output.numClosestPairs = 1;
                    if (d > C_<T>(0))
                    {
                        output.circle0Closest[0] = circle0.center + r0 * U;
                        output.circle1Closest[0] = circle1.center + r1 * U;
                        output.equidistant = false;
                    }
                    else
                    {
                        // The circles are concentric, so U = (0,0,0).
                        // Construct a vector perpendicular to N0 to use for
                        // closest points.
                        U = GetOrthogonal(circle0.normal, true);
                        output.circle0Closest[0] = circle0.center + r0 * U;
                        output.circle1Closest[0] = circle1.center + r1 * U;
                        output.equidistant = true;
                    }
                }
                else if (dmr1 <= -r0)
                {
                    // Circle0 is inside circle1.
                    distance = -r0 - dmr1;
                    output.numClosestPairs = 1;
                    if (d > C_<T>(0))
                    {
                        output.circle0Closest[0] = circle0.center - r0 * U;
                        output.circle1Closest[0] = circle1.center - r1 * U;
                        output.equidistant = false;
                    }
                    else
                    {
                        // The circles are concentric, so U = (0,0,0).
                        // Construct a vector perpendicular to N0 to use for
                        // closest points.
                        U = GetOrthogonal(circle0.normal, true);
                        output.circle0Closest[0] = circle0.center + r0 * U;
                        output.circle1Closest[0] = circle1.center + r1 * U;
                        output.equidistant = true;
                    }
                }
                else
                {
                    // The circles are overlapping.  The two points of
                    // intersection are C0 + s*(C1-C0) +/- h*Cross(N,U), where
                    // s = (1 + (r0^2 - r1^2)/d^2)/2 and
                    // h = sqrt(r0^2 - s^2 * d^2).
                    T r0sqr = r0 * r0, r1sqr = r1 * r1, dsqr = d * d;
                    T s = (C_<T>(1) + (r0sqr - r1sqr) / dsqr) / C_<T>(2);
                    T arg = std::max(r0sqr - dsqr * s * s, C_<T>(0));
                    T h = std::sqrt(arg);
                    Vector3<T> midpoint = circle0.center + s * compProj;
                    Vector3<T> hNxU = h * Cross(circle0.normal, U);
                    distance = C_<T>(0);
                    output.numClosestPairs = 2;
                    output.circle0Closest[0] = midpoint + hNxU;
                    output.circle0Closest[1] = midpoint - hNxU;
                    output.circle1Closest[0] = output.circle0Closest[0] + normProj;
                    output.circle1Closest[1] = output.circle0Closest[1] + normProj;
                    output.equidistant = false;
                }
            }

            output.sqrDistance = distance * distance + N0dD * N0dD;
        }
    };
}
