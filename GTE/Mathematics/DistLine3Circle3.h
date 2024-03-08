// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.11.20

#pragma once

// The 3D line-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Circle3.h>
#include <Mathematics/Line.h>
#include <Mathematics/RootsBisection.h>
#include <Mathematics/RootsPolynomial.h>
#include <Mathematics/Polynomial1.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line3<T>, Circle3<T>>
    {
    public:
        // The possible number of closest line-circle pairs is 1, 2 or all
        // circle points. If 1 or 2, numClosestPairs is set to this number
        // and 'equidistant' is false; the number of valid elements in
        // lineClosest[] and circleClosest[] is numClosestPairs. If all
        // circle points are closest, the line must be C+s*N where C is the
        // circle center, N is the normal to the plane of the circle, and
        // lineClosest[0] is set to C. In this case, 'equidistant' is true
        // and circleClosest[0] is set to C+r*U, where r is the circle
        // and U is a vector perpendicular to N.
        struct Result
        {
            Result()
                :
                numClosestPairs(0),
                lineClosest{ Vector3<T>::Zero(), Vector3<T>::Zero() },
                circleClosest{ Vector3<T>::Zero(), Vector3<T>::Zero() },
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                equidistant(false)
            {
            }

            size_t numClosestPairs;
            std::array<Vector3<T>, 2> lineClosest, circleClosest;
            T distance, sqrDistance;
            bool equidistant;
        };

        // The polynomial-based algorithm. Type T can be floating-point or
        // rational.
        Result operator()(Line3<T> const& line, Circle3<T> const& circle,
            bool useBisection = false)
        {
            // The constructor sets result.equidistant to false.
            Result result{};

            // Translate the line and circle so that the circle center is the
            // origin (0,0,0).
            auto const& M = line.direction;
            Vector3<T> D = line.origin - circle.center;
            auto const& N = circle.normal;
            T r = circle.radius;
            Vector3<T> NxM = Cross(N, M);
            Vector3<T> NxD = Cross(N, D);

            Vector3<T> const vzero = Vector3<T>::Zero();
            T const zero = static_cast<T>(0);

            if (NxM != vzero)
            {
                // The line is not perpendicular to the plane of the circle.
                if (NxD != vzero)
                {
                    T NdM = Dot(circle.normal, line.direction);
                    if (NdM != zero)
                    {
                        // Section 4.5. The general case where H(t) is a
                        // quartic polynomial.

                        // Choose a different line origin E so that
                        // Dot(NxM,NxE) = 0.
                        T s = -Dot(NxM, NxD) / Dot(NxM, NxM);
                        Vector3<T> E = s * M + D;

                        // Compute the right-handed orthonormal basis {U,V,N}.
                        // system with basis {U,V,N}.
                        Vector3<T> V = NxM;
                        (void)Normalize(V);
                        Vector3<T> U = Cross(V, N);

                        // Compute the representations of M and E in the
                        // orthonormal basis. The basis was chosen so that
                        // m1 = Dot(V,M) = 0 and e0 = Dot(U,E) = 0.
                        T m0 = Dot(U, M);
                        T m1 = Dot(V, M); (void)m1;
                        T m2 = NdM;
                        T e0 = Dot(U, E); (void)e0;
                        T e1 = Dot(V, E);
                        T e2 = Dot(N, E);

                        // Section 4.5. Solve roots using bisection when
                        // useBisection is true, using quartic root solver
                        // when bisection is false.
                        GeneralCase(m0, m2, e1, e2, r, useBisection, result);

                        // Convert back to the coordinate system where the
                        // circle center is at the origin.
                        for (size_t i = 0; i < result.numClosestPairs; ++i)
                        {
                            auto& lClosest = result.lineClosest[i];
                            auto& cClosest = result.circleClosest[i];
                            lClosest = lClosest[0] * U + lClosest[1] * V + lClosest[2] * N;
                            cClosest = cClosest[0] * U + cClosest[1] * V + cClosest[2] * N;
                        }
                    }
                    else
                    {
                        // Section 4.4. The line is parallel to the plane of
                        // the circle. The line origin is not on the normal
                        // line C+s*N (handled instead by Section 4.3).
                        LineParallelToPlane(M, D, N, r, NxD, result);
                    }
                }
                else
                {
                    // Section 4.3. The line is not perpendicular to the
                    // plane of the circle. The line origin is on the normal
                    // line C+s*N.
                    LineNotPerpendicularToPlaneOriginOnNormalLine(M, D, N, r, NxM, result);
                }
            }
            else
            {
                // The line is perpendicular to the plane of the circle.
                if (NxD != vzero)
                {
                    // Section 4.2. The line does not contain the circle
                    // center.
                    LinePerpendicularToPlaneNotContainCenter(M, D, N, r, result);
                }
                else
                {
                    // Section 4.1. The line contains the circle center, so
                    // the center is the closest point for the line and all
                    // circle points are equidistant from it.
                    LinePerpendicularToPlaneContainCenter(N, r, result);
                    result.equidistant = true;
                }
            }

            // Translate the closest points back to the original coordinate
            // system.
            for (size_t i = 0; i < result.numClosestPairs; ++i)
            {
                result.lineClosest[i] += circle.center;
                result.circleClosest[i] += circle.center;
            }

            Vector3<T> diff = result.lineClosest[0] - result.circleClosest[0];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

        // The nonpolynomial-based algorithm that uses bisection.
        Result Robust(Line3<T> const& line, Circle3<T> const& circle)
        {
            // The line is P(t) = B+t*M.  The circle is |X-C| = r with
            // Dot(N,X-C)=0.
            Result result{};
            Vector3<T> vzero = Vector3<T>::Zero();
            T const zero = static_cast<T>(0);

            Vector3<T> D = line.origin - circle.center;
            Vector3<T> MxN = Cross(line.direction, circle.normal);
            Vector3<T> DxN = Cross(D, circle.normal);

            T m0sqr = Dot(MxN, MxN);
            if (m0sqr > zero)
            {
                // Compute the critical points s for F'(s) = 0.
                T s{}, t{};
                size_t numRoots = 0;
                std::array<T, 3> roots{};

                // The line direction M and the plane normal N are not
                // parallel.  Move the line origin B = (b0,b1,b2) to
                // B' = B + lambda*line.direction = (0,b1',b2').
                T m0 = std::sqrt(m0sqr);
                T rm0 = circle.radius * m0;
                T lambda = -Dot(MxN, DxN) / m0sqr;
                Vector3<T> oldD = D;
                D += lambda * line.direction;
                DxN += lambda * MxN;
                T m2b2 = Dot(line.direction, D);
                T b1sqr = Dot(DxN, DxN);
                if (b1sqr > zero)
                {
                    // B' = (0,b1',b2') where b1' != 0.  See Sections 1.1.2
                    // and 1.2.2 of the PDF documentation.
                    T b1 = std::sqrt(b1sqr);
                    T rm0sqr = circle.radius * m0sqr;
                    if (rm0sqr > b1)
                    {
                        T const twoThirds = static_cast<T>(2) / static_cast<T>(3);
                        T sHat = std::sqrt(std::pow(rm0sqr * b1sqr, twoThirds) - b1sqr) / m0;
                        T gHat = rm0sqr * sHat / std::sqrt(m0sqr * sHat * sHat + b1sqr);
                        T cutoff = gHat - sHat;
                        if (m2b2 <= -cutoff)
                        {
                            s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2, -m2b2 + rm0);
                            roots[numRoots++] = s;
                            if (m2b2 == -cutoff)
                            {
                                roots[numRoots++] = -sHat;
                            }
                        }
                        else if (m2b2 >= cutoff)
                        {
                            s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2 - rm0, -m2b2);
                            roots[numRoots++] = s;
                            if (m2b2 == cutoff)
                            {
                                roots[numRoots++] = sHat;
                            }
                        }
                        else
                        {
                            if (m2b2 <= zero)
                            {
                                s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2, -m2b2 + rm0);
                                roots[numRoots++] = s;
                                s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2 - rm0, -sHat);
                                roots[numRoots++] = s;
                            }
                            else
                            {
                                s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2 - rm0, -m2b2);
                                roots[numRoots++] = s;
                                s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, sHat, -m2b2 + rm0);
                                roots[numRoots++] = s;
                            }
                        }
                    }
                    else
                    {
                        if (m2b2 < zero)
                        {
                            s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2, -m2b2 + rm0);
                        }
                        else if (m2b2 > zero)
                        {
                            s = Bisect(m2b2, rm0sqr, m0sqr, b1sqr, -m2b2 - rm0, -m2b2);
                        }
                        else
                        {
                            s = zero;
                        }
                        roots[numRoots++] = s;
                    }
                }
                else
                {
                    // The new line origin is B' = (0,0,b2').
                    if (m2b2 < zero)
                    {
                        s = -m2b2 + rm0;
                        roots[numRoots++] = s;
                    }
                    else if (m2b2 > zero)
                    {
                        s = -m2b2 - rm0;
                        roots[numRoots++] = s;
                    }
                    else
                    {
                        s = -m2b2 + rm0;
                        roots[numRoots++] = s;
                        s = -m2b2 - rm0;
                        roots[numRoots++] = s;
                    }
                }

                std::array<ClosestInfo, 4> candidates{};
                for (size_t i = 0; i < numRoots; ++i)
                {
                    t = roots[i] + lambda;
                    ClosestInfo info{};
                    Vector3<T> NxDelta = Cross(circle.normal, oldD + t * line.direction);
                    if (NxDelta != vzero)
                    {
                        GetClosestPair(line.direction, oldD, circle.normal, circle.radius, t,
                            info.lineClosest, info.circleClosest);
                        info.lineClosest += circle.center;
                        info.circleClosest += circle.center;
                        info.equidistant = false;
                    }
                    else
                    {
                        Vector3<T> U = GetOrthogonal(circle.normal, true);
                        info.lineClosest = circle.center;
                        info.circleClosest = circle.center + circle.radius * U;
                        info.equidistant = true;
                    }
                    Vector3<T> diff = info.lineClosest - info.circleClosest;
                    info.sqrDistance = Dot(diff, diff);
                    candidates[i] = info;
                }

                std::sort(candidates.begin(), candidates.begin() + numRoots);

                result.numClosestPairs = 1;
                result.lineClosest[0] = candidates[0].lineClosest;
                result.circleClosest[0] = candidates[0].circleClosest;
                if (numRoots > 1 &&
                    candidates[1].sqrDistance == candidates[0].sqrDistance)
                {
                    result.numClosestPairs = 2;
                    result.lineClosest[1] = candidates[1].lineClosest;
                    result.circleClosest[1] = candidates[1].circleClosest;
                }

                result.equidistant = false;
            }
            else
            {
                // The line direction and the plane normal are parallel.
                if (DxN != vzero)
                {
                    // The line is A+t*N but with A != C.
                    result.numClosestPairs = 1;
                    T t = -Dot(line.direction, D);
                    GetClosestPair(line.direction, D, circle.normal, circle.radius, t,
                        result.lineClosest[0], result.circleClosest[0]);
                    result.lineClosest[0] += circle.center;
                    result.circleClosest[0] += circle.center;
                    result.equidistant = false;
                }
                else
                {
                    // The line is C+t*N, so C is the closest point for the
                    // line and all circle points are equidistant from it.
                    Vector3<T> U = GetOrthogonal(circle.normal, true);
                    result.numClosestPairs = 1;
                    result.lineClosest[0] = circle.center;
                    result.circleClosest[0] = circle.center + circle.radius * U;
                    result.equidistant = true;
                }
            }

            Vector3<T> diff = result.lineClosest[0] - result.circleClosest[0];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

    private:
        // Get the closest pair of line and circle points. The circle center
        // is assumed to be at the origin.
        void GetClosestPair(
            Vector3<T> const& M, Vector3<T> const& D,
            Vector3<T> const& N, T r, T t,
            Vector3<T>& lineClosest, Vector3<T>& circleClosest)
        {
            lineClosest = t * M + D;
            Vector3<T> project = lineClosest - Dot(N, lineClosest) * N;
            Normalize(project);
            circleClosest = r * project;
        }

        void GeneralCase(T m0, T m2, T e1, T e2, T r, bool useBisection, Result& result)
        {
            // The closest points are computed in the {U,V,N} basis. They must
            // be converted back to Cartesian coordinates by the caller.
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T m0sqr = m0 * m0;
            T rm0sqr = r * m0sqr;
            T e1sqr = e1 * e1;
            T m2e2 = m2 * e2;
            Vector3<T> M{ m0, zero, m2 }, E{ zero, e1, e2 }, N{ zero, zero, one };

            if (e2 != zero)
            {
                // This function is called when Dot(N,M) != 0, so M is not
                // parallel to the plane of the circle. The implication is that
                // there is a unique pair of closest points. Compute the
                // candidate pairs of closest points and choose the pair
                // attaining the minimum distance.
                size_t numRoots = 0;
                std::array<T, 4> roots{};

                if (useBisection)
                {
                    // Section 4.5.3, use bisection to compute roots of F'(t).
                    T rm0 = r * std::fabs(m0);
                    T t{};

                    if (rm0sqr > std::fabs(e1))
                    {
                        // G'(0) > 1
                        T const twoThirds = static_cast<T>(2) / static_cast<T>(3);
                        T tHat = std::sqrt(std::pow(rm0sqr * e1sqr, twoThirds) - e1sqr) / std::fabs(m0);
                        T gHat = rm0sqr * tHat / std::sqrt(m0sqr * tHat * tHat + e1sqr);
                        T cutoff = gHat - tHat;
                        if (m2e2 <= -cutoff)
                        {
                            t = Bisect(m2e2, rm0sqr, m0sqr, e1sqr, -m2e2, -m2e2 + rm0);
                            roots[numRoots++] = t;
                            if (m2e2 == -cutoff)
                            {
                                roots[numRoots++] = -tHat;
                            }
                        }
                        else if (m2e2 >= cutoff)
                        {
                            t = Bisect(m2e2, rm0sqr, m0sqr, e1sqr, -m2e2 - rm0, -m2e2);
                            roots[numRoots++] = t;
                            if (m2e2 == cutoff)
                            {
                                roots[numRoots++] = tHat;
                            }
                        }
                        else
                        {
                            if (m2e2 <= zero)
                            {
                                t = Bisect(m2e2, rm0sqr, m0sqr, e1sqr, -m2e2, -m2e2 + rm0);
                                roots[numRoots++] = t;
                                t = Bisect(m2e2, rm0sqr, m0sqr, e1sqr, -m2e2 - rm0, -tHat);
                                roots[numRoots++] = t;
                            }
                            else
                            {
                                t = Bisect(m2e2, rm0sqr, m0sqr, e1sqr, -m2e2 - rm0, -m2e2);
                                roots[numRoots++] = t;
                                t = Bisect(m2e2, rm0sqr, m0sqr, e1sqr, tHat, -m2e2 + rm0);
                                roots[numRoots++] = t;
                            }
                        }
                    }
                    else
                    {
                        // G'(0) <= 1, unique root for F'(t)
                        if (m2e2 < zero)
                        {
                            t = Bisect(m2e2, rm0sqr, m0sqr, e1sqr, -m2e2, -m2e2 + rm0);
                        }
                        else if (m2e2 > zero)
                        {
                            t = Bisect(m2e2, rm0sqr, m0sqr, e1sqr, -m2e2 - rm0, -m2e2);
                        }
                        else
                        {
                            t = zero;
                        }
                        roots[numRoots++] = t;
                    }

                }
                else
                {
                    // Section 4.5.2, use quartic root solver to compute roots of H(t).
                    // 
                    // H(t) = (t + m2*e2)^2 * (m0^2*t^2 + e1^2) - r^2*m0^4*t^2
                    //      = h0 + h1 * t + h2 * t^2 + h3 * t^3 + h4 * t^4
                    Polynomial1<T> poly0{ m2e2, one };
                    Polynomial1<T> poly1{ e1sqr, zero, m0sqr };
                    Polynomial1<T> poly2{ zero, zero, rm0sqr * rm0sqr };
                    Polynomial1<T> h = poly0 * poly0 * poly1 - poly2;

                    std::map<T, int32_t> rmMap{};
                    RootsPolynomial<T>::template SolveQuartic<T>(h[0], h[1], h[2], h[3], h[4], rmMap);
                    for (auto const& rm : rmMap)
                    {
                        roots[numRoots++] = rm.first;
                    }
                }

                T minSqrDistance = std::numeric_limits<T>::max();
                result.numClosestPairs = 1;
                for (size_t i = 0; i < numRoots; ++i)
                {
                    Vector3<T> lClosest{}, cClosest{};
                    GetClosestPair(M, E, N, r, roots[i], lClosest, cClosest);
                    Vector3<T> diff = lClosest - cClosest;
                    T sqrDistance = Dot(diff, diff);
                    if (sqrDistance < minSqrDistance)
                    {
                        result.lineClosest[0] = lClosest;
                        result.circleClosest[0] = cClosest;
                        minSqrDistance = sqrDistance;
                    }
                }
            }
            else
            {
                // Section 4.5, line origin is on plane of circle.
                T discr = e1sqr - rm0sqr * rm0sqr;
                if (discr >= zero)
                {
                    result.numClosestPairs = 1;
                    GetClosestPair(M, E, N, r, zero,
                        result.lineClosest[0], result.circleClosest[0]);
                }
                else // discr < zero
                {
                    result.numClosestPairs = 2;
                    T t = std::sqrt(-discr / m0sqr);
                    GetClosestPair(M, E, N, r, -t,
                        result.lineClosest[0], result.circleClosest[0]);
                    GetClosestPair(M, E, N, r, +t,
                        result.lineClosest[1], result.circleClosest[1]);
                }
            }
        }

        void LineParallelToPlane(
            Vector3<T> const& M, Vector3<T> const& D,
            Vector3<T> const& N, T r, Vector3<T> const& NxD,
            Result& result)
        {
            // H(t) = (t + v)^2 * [(t + v)^2 - (r^2 - u^2)]
            // A root is t = -v. If r^2 > u^2, two other roots are
            // t = -v - sqrt(r^2 - u^2) and t = -v + sqrt(r^2 - u^2).
            T const zero = static_cast<T>(0);
            T u = Dot(NxD, M);
            T v = Dot(M, D);
            T discr = r * r - u * u;
            if (discr > zero)  // u^2 < r^2
            {
                T rootDiscr = std::sqrt(discr);
                result.numClosestPairs = 2;
                GetClosestPair(M, D, N, r, -v - rootDiscr,
                    result.lineClosest[0], result.circleClosest[0]);
                GetClosestPair(M, D, N, r, -v + rootDiscr,
                    result.lineClosest[1], result.circleClosest[1]);
            }
            else // u^2 >= r^2
            {
                result.numClosestPairs = 1;
                GetClosestPair(M, D, N, r, -v,
                    result.lineClosest[0], result.circleClosest[0]);
            }
        }

        void LineNotPerpendicularToPlaneOriginOnNormalLine(
            Vector3<T> const& M, Vector3<T> const& D,
            Vector3<T> const& N, T r, Vector3<T> const& NxM, Result& result)
        {
            // H(t) = |Cross(N,M)|^2*t^2*((t+Dot(M,D))^2-r^2*|Cross(N,M)|^2)
            // The roots are t = 0, t = -Dot(M,D) - r*|Cross(M,N)|, and
            // t = -Dot(M,D) + r*|Cross(M,N)|, but t = 0 is extraneous.
            T const zero = static_cast<T>(0);
            T MdD = Dot(M, D);
            T rLenCrossMN = r * Length(NxM);
            if (MdD > zero)
            {
                result.numClosestPairs = 1;
                GetClosestPair(M, D, N, r, -MdD - rLenCrossMN,
                    result.lineClosest[0], result.circleClosest[0]);
            }
            else if (MdD < zero)
            {
                result.numClosestPairs = 1;
                GetClosestPair(M, D, N, r, -MdD + rLenCrossMN,
                    result.lineClosest[0], result.circleClosest[0]);
            }
            else // MdD = 0
            {
                result.numClosestPairs = 2;
                GetClosestPair(M, D, N, r, -rLenCrossMN,
                    result.lineClosest[0], result.circleClosest[0]);
                GetClosestPair(M, D, N, r, +rLenCrossMN,
                    result.lineClosest[1], result.circleClosest[1]);
            }
        }

        void LinePerpendicularToPlaneNotContainCenter(
            Vector3<T> const& M, Vector3<T> const& D,
            Vector3<T> const& N, T r, Result& result)
        {
            // H(t) = |Cross(N,D)|^2 * (t + Dot(M,D))^2
            result.numClosestPairs = 1;
            GetClosestPair(M, D, N, r, -Dot(M, D),
                result.lineClosest[0], result.circleClosest[0]);
        }

        void LinePerpendicularToPlaneContainCenter(
            Vector3<T> const& N, T r, Result& result)
        {
            // H(t) = 0 for all t.
            Vector3<T> U = GetOrthogonal(N, true);
            result.numClosestPairs = 1;
            result.lineClosest[0] = Vector3<T>::Zero();
            result.circleClosest[0] = r * U;
        }

        // Support for Robust(...).
        struct ClosestInfo
        {
            ClosestInfo()
                :
                sqrDistance(static_cast<T>(0)),
                lineClosest(Vector3<T>::Zero()),
                circleClosest(Vector3<T>::Zero()),
                equidistant(false)
            {
            }

            bool operator< (ClosestInfo const& info) const
            {
                return sqrDistance < info.sqrDistance;
            }

            T sqrDistance;
            Vector3<T> lineClosest, circleClosest;
            bool equidistant;
        };

        // Bisect the function
        //   F(s) = s + m2b2 - r*m0sqr*s/sqrt(m0sqr*s*s + e1sqr)
        // on the specified interval [smin,smax].
        T Bisect(T m2b2, T rm0sqr, T m0sqr, T e1sqr, T smin, T smax)
        {
            std::function<T(T)> G = [&, m2b2, rm0sqr, m0sqr, e1sqr](T s)
            {
                return s + m2b2 - rm0sqr * s / std::sqrt(m0sqr * s * s + e1sqr);
            };

            // The function is known to be increasing, so we can specify -1 and +1
            // as the function values at the bounding interval endpoints.  The use
            // of 'double' is intentional in case T is a BSNumber or BSRational
            // type.  We want the bisections to terminate in a reasonable amount of
            // time.
            uint32_t const maxIterations = 2048u;
            T root = static_cast<T>(0);
            T one = static_cast<T>(1);
            RootsBisection<T>::Find(G, smin, smax, -one, one, maxIterations, root);
            return root;
        }
    };
}
