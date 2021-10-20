// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.10.17

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Circle3.h>
#include <Mathematics/Line.h>
#include <Mathematics/RootsBisection.h>
#include <Mathematics/RootsPolynomial.h>

// The 3D line-circle distance algorithm is described in
// https://www.geometrictools.com/Documentation/DistanceToCircle3.pdf
// The notation used in the code matches that of the document.

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line3<T>, Circle3<T>>
    {
    public:
        // The possible number of closest line-circle pairs is 1, 2 or all
        // circle points.  If 1 or 2, numClosestPairs is set to this number
        // and 'equidistant' is false; the number of valid elements in
        // lineClosest[] and circleClosest[] is numClosestPairs.  If all
        // circle points are closest, the line must be C+t*N where C is the
        // circle center, N is the normal to the plane of the circle, and
        // lineClosest[0] is set to C.  In this case, 'equidistant' is true
        // and circleClosest[0] is set to C+r*U, where r is the circle
        // and U is a vector perpendicular to N.
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                numClosestPairs(0),
                lineClosest{ Vector3<T>::Zero(), Vector3<T>::Zero() },
                circleClosest{ Vector3<T>::Zero(), Vector3<T>::Zero() },
                equidistant(false)
            {
            }

            T distance, sqrDistance;
            size_t numClosestPairs;
            std::array<Vector3<T>, 2> lineClosest, circleClosest;
            bool equidistant;
        };

        // The polynomial-based algorithm.  Type T can be floating-point or
        // rational.
        Result operator()(Line3<T> const& line, Circle3<T> const& circle)
        {
            Result result{};
            Vector3<T> const vzero = Vector3<T>::Zero();
            T const zero = (T)0;

            Vector3<T> D = line.origin - circle.center;
            Vector3<T> NxM = Cross(circle.normal, line.direction);
            Vector3<T> NxD = Cross(circle.normal, D);
            T t = zero;

            if (NxM != vzero)
            {
                if (NxD != vzero)
                {
                    T NdM = Dot(circle.normal, line.direction);
                    if (NdM != zero)
                    {
                        // H(t) = (a*t^2 + 2*b*t + c)*(t + d)^2
                        //        - r^2*(a*t + b)^2
                        //      = h0 + h1*t + h2*t^2 + h3*t^3 + h4*t^4
                        T a = Dot(NxM, NxM), b = Dot(NxM, NxD);
                        T c = Dot(NxD, NxD), d = Dot(line.direction, D);
                        T rsqr = circle.radius * circle.radius;
                        T asqr = a * a, bsqr = b * b, dsqr = d * d;
                        T h0 = c * dsqr - bsqr * rsqr;
                        T h1 = (T)2 * (c * d + b * dsqr - a * b * rsqr);
                        T h2 = c + (T)4 * b * d + a * dsqr - asqr * rsqr;
                        T h3 = (T)2 * (b + a * d);
                        T h4 = a;

                        std::map<T, int32_t> rmMap{};
                        RootsPolynomial<T>::template SolveQuartic<T>(h0, h1, h2, h3, h4, rmMap);
                        std::array<ClosestInfo, 4> candidates{};
                        size_t numRoots = 0;
                        for (auto const& rm : rmMap)
                        {
                            t = rm.first;
                            ClosestInfo info{};
                            Vector3<T> NxDelta = NxD + t * NxM;
                            if (NxDelta != vzero)
                            {
                                GetPair(line, circle, D, t, info.lineClosest, info.circleClosest);
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
                            candidates[numRoots++] = info;
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
                    }
                    else
                    {
                        // The line is parallel to the plane of the circle.
                        // The polynomial has the form
                        // H(t) = (t+v)^2*[(t+v)^2-(r^2-u^2)].
                        T u = Dot(NxM, D), v = Dot(line.direction, D);
                        T discr = circle.radius * circle.radius - u * u;
                        if (discr > zero)
                        {
                            result.numClosestPairs = 2;
                            T rootDiscr = std::sqrt(discr);
                            t = -v + rootDiscr;
                            GetPair(line, circle, D, t, result.lineClosest[0],
                                result.circleClosest[0]);
                            t = -v - rootDiscr;
                            GetPair(line, circle, D, t, result.lineClosest[1],
                                result.circleClosest[1]);
                        }
                        else
                        {
                            result.numClosestPairs = 1;
                            t = -v;
                            GetPair(line, circle, D, t, result.lineClosest[0],
                                result.circleClosest[0]);
                        }
                    }
                }
                else
                {
                    // The line is C+t*M, where M is not parallel to N.  The
                    // polynomial is
                    // H(t) = |Cross(N,M)|^2*t^2*(t^2 - r^2*|Cross(N,M)|^2)
                    // where root t = 0 does not correspond to the global
                    // minimum.  The other roots produce the global minimum.
                    result.numClosestPairs = 2;
                    t = circle.radius * Length(NxM);
                    GetPair(line, circle, D, t, result.lineClosest[0],
                        result.circleClosest[0]);
                    t = -t;
                    GetPair(line, circle, D, t, result.lineClosest[1],
                        result.circleClosest[1]);
                }
                result.equidistant = false;
            }
            else
            {
                if (NxD != vzero)
                {
                    // The line is A+t*N (perpendicular to plane) but with
                    // A != C.  The polyhomial is
                    // H(t) = |Cross(N,D)|^2*(t + Dot(M,D))^2.
                    result.numClosestPairs = 1;
                    t = -Dot(line.direction, D);
                    GetPair(line, circle, D, t, result.lineClosest[0],
                        result.circleClosest[0]);
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

        // The nonpolynomial-based algorithm that uses bisection.  Because the
        // bisection is iterative, you should choose T to be a
        // floating-point type.  However, the algorithm will still work for a
        // rational type, but it is costly because of the increase in
        // arbitrary-size integers used during the bisection.
        Result Robust(Line3<T> const& line, Circle3<T> const& circle)
        {
            // The line is P(t) = B+t*M.  The circle is |X-C| = r with
            // Dot(N,X-C)=0.
            Result result{};
            Vector3<T> vzero = Vector3<T>::Zero();
            T const zero = (T)0;

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
                        T const twoThirds = (T)2 / (T)3;
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

                std::array<ClosestInfo, 4> candidates;
                for (size_t i = 0; i < numRoots; ++i)
                {
                    t = roots[i] + lambda;
                    ClosestInfo info{};
                    Vector3<T> NxDelta = Cross(circle.normal, oldD + t * line.direction);
                    if (NxDelta != vzero)
                    {
                        GetPair(line, circle, oldD, t, info.lineClosest, info.circleClosest);
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
                    GetPair(line, circle, D, -Dot(line.direction, D),
                        result.lineClosest[0], result.circleClosest[0]);
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
        // Support for operator(...).
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

        void GetPair(Line3<T> const& line, Circle3<T> const& circle,
            Vector3<T> const& D, T t, Vector3<T>& lineClosest,
            Vector3<T>& circleClosest)
        {
            Vector3<T> delta = D + t * line.direction;
            lineClosest = circle.center + delta;
            delta -= Dot(circle.normal, delta) * circle.normal;
            Normalize(delta);
            circleClosest = circle.center + circle.radius * delta;
        }

        // Support for Robust(...).  Bisect the function
        //   F(s) = s + m2b2 - r*m0sqr*s/sqrt(m0sqr*s*s + b1sqr)
        // on the specified interval [smin,smax].
        T Bisect(T m2b2, T rm0sqr, T m0sqr, T b1sqr, T smin, T smax)
        {
            std::function<T(T)> G = [&, m2b2, rm0sqr, m0sqr, b1sqr](T s)
            {
                return s + m2b2 - rm0sqr * s / std::sqrt(m0sqr * s * s + b1sqr);
            };

            // The function is known to be increasing, so we can specify -1 and +1
            // as the function values at the bounding interval endpoints.  The use
            // of 'double' is intentional in case T is a BSNumber or BSRational
            // type.  We want the bisections to terminate in a reasonable amount of
            // time.
            uint32_t const maxIterations = GTE_C_MAX_BISECTIONS_GENERIC;
            T root = static_cast<T>(0);
            RootsBisection<T>::Find(G, smin, smax, (T)-1, (T)+1, maxIterations, root);
            return root;
        }
    };
}
