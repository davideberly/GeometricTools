// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/DCPQuery.h>
#include <Mathematics/Circle3.h>
#include <Mathematics/Polynomial1.h>
#include <Mathematics/RootsPolynomial.h>
#include <set>

// The 3D circle-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document.

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

        Result operator()(Circle3<T> const& circle0, Circle3<T> const& circle1)
        {
            Result result{};
            Vector3<T> const vzero = Vector3<T>::Zero();
            T const zero = (T)0;

            Vector3<T> N0 = circle0.normal, N1 = circle1.normal;
            T r0 = circle0.radius, r1 = circle1.radius;
            Vector3<T> D = circle1.center - circle0.center;
            Vector3<T> N0xN1 = Cross(N0, N1);

            if (N0xN1 != vzero)
            {
                // Get parameters for constructing the degree-8 polynomial phi.
                T const one = (T)1, two = (T)2;
                T r0sqr = r0 * r0, r1sqr = r1 * r1;

                // Compute U1 and V1 for the plane of circle1.
                std::array<Vector3<T>, 3> basis{};
                basis[0] = circle1.normal;
                ComputeOrthogonalComplement(1, basis.data());
                Vector3<T> U1 = basis[1], V1 = basis[2];

                // Construct the polynomial phi(cos(theta)).
                Vector3<T> N0xD = Cross(N0, D);
                Vector3<T> N0xU1 = Cross(N0, U1), N0xV1 = Cross(N0, V1);
                T a0 = r1 * Dot(D, U1), a1 = r1 * Dot(D, V1);
                T a2 = Dot(N0xD, N0xD), a3 = r1 * Dot(N0xD, N0xU1);
                T a4 = r1 * Dot(N0xD, N0xV1), a5 = r1sqr * Dot(N0xU1, N0xU1);
                T a6 = r1sqr * Dot(N0xU1, N0xV1), a7 = r1sqr * Dot(N0xV1, N0xV1);
                Polynomial1<T> p0{ a2 + a7, two * a3, a5 - a7 };
                Polynomial1<T> p1{ two * a4, two * a6 };
                Polynomial1<T> p2{ zero, a1 };
                Polynomial1<T> p3{ -a0 };
                Polynomial1<T> p4{ -a6, a4, two * a6 };
                Polynomial1<T> p5{ -a3, a7 - a5 };
                Polynomial1<T> tmp0{ one, zero, -one };
                Polynomial1<T> tmp1 = p2 * p2 + tmp0 * p3 * p3;
                Polynomial1<T> tmp2 = two * p2 * p3;
                Polynomial1<T> tmp3 = p4 * p4 + tmp0 * p5 * p5;
                Polynomial1<T> tmp4 = two * p4 * p5;
                Polynomial1<T> p6 = p0 * tmp1 + tmp0 * p1 * tmp2 - r0sqr * tmp3;
                Polynomial1<T> p7 = p0 * tmp2 + p1 * tmp1 - r0sqr * tmp4;

                // Parameters for polynomial root finding. The roots[] array
                // stores the roots. We need only the unique ones, which is
                // the responsibility of the set uniqueRoots. The pairs[]
                // array stores the (cosine,sine) information mentioned in the
                // PDF. TODO: Choose the maximum number of iterations for root
                // finding based on specific polynomial data?
                uint32_t const maxIterations = 128;
                int32_t degree = 0;
                size_t numRoots = 0;
                std::array<T, 8> roots{};
                std::set<T> uniqueRoots{};
                size_t numPairs = 0;
                std::array<std::pair<T, T>, 16> pairs{};
                T temp = zero, sn = zero;

                if (p7.GetDegree() > 0 || p7[0] != zero)
                {
                    // H(cs,sn) = p6(cs) + sn * p7(cs)
                    Polynomial1<T> phi = p6 * p6 - tmp0 * p7 * p7;
                    degree = static_cast<int32_t>(phi.GetDegree());
                    LogAssert(degree > 0, "Unexpected degree for phi.");
                    numRoots = RootsPolynomial<T>::Find(degree, &phi[0], maxIterations, roots.data());
                    for (size_t i = 0; i < numRoots; ++i)
                    {
                        uniqueRoots.insert(roots[i]);
                    }

                    for (auto const& cs : uniqueRoots)
                    {
                        if (std::fabs(cs) <= one)
                        {
                            temp = p7(cs);
                            if (temp != zero)
                            {
                                sn = -p6(cs) / temp;
                                pairs[numPairs++] = std::make_pair(cs, sn);
                            }
                            else
                            {
                                temp = std::max(one - cs * cs, zero);
                                sn = std::sqrt(temp);
                                pairs[numPairs++] = std::make_pair(cs, sn);
                                if (sn != zero)
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
                    degree = static_cast<int32_t>(p6.GetDegree());
                    LogAssert(degree > 0, "Unexpected degree for p6.");
                    numRoots = RootsPolynomial<T>::Find(degree, &p6[0], maxIterations, roots.data());
                    for (size_t i = 0; i < numRoots; ++i)
                    {
                        uniqueRoots.insert(roots[i]);
                    }

                    for (auto const& cs : uniqueRoots)
                    {
                        if (std::fabs(cs) <= one)
                        {
                            temp = std::max(one - cs * cs, zero);
                            sn = std::sqrt(temp);
                            pairs[numPairs++] = std::make_pair(cs, sn);
                            if (sn != zero)
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
                    if (lenN0xDelta > (T)0)
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

                result.numClosestPairs = 1;
                result.sqrDistance = candidates[0].sqrDistance;
                result.circle0Closest[0] = candidates[0].circle0Closest;
                result.circle1Closest[0] = candidates[0].circle1Closest;
                result.equidistant = candidates[0].equidistant;
                if (numRoots > 1 &&
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
                DoQueryParallelPlanes(circle0, circle1, D, result);
            }

            result.distance = std::sqrt(result.sqrDistance);
            return result;
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
        }
    };
}
