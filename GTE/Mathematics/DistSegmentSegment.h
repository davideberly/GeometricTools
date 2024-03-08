// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the closest points for two segments in nD.
// 
// The segments are P[0] + s[0] * (P[1] - P[0]) for 0 <= s[0] <= 1 and
// Q[0] + s[1] * (Q[1] - Q[0]) for 0 <= s[1] <= 1. The D[i] are not required
// to be unit length.
// 
// The closest point on segment[i] is stored in closest[i] with parameter[i]
// storing s[i]. When there are infinitely many choices for the pair of
// closest points, only one of them is returned.
// 
// The algorithm is robust even for nearly parallel segments. Effectively, it
// uses a conjugate gradient search for the minimum of the squared distance
// function, which avoids the numerical problems introduced by divisions in
// the case the minimum is located at an interior point of the domain. See the
// document
//   https://www.geometrictools.com/Documentation/DistanceLine3Line3.pdf
// for details.

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Segment.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Segment<N, T>, Segment<N, T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                parameter{ static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector<N, T>::Zero(), Vector<N, T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 2> parameter;
            std::array<Vector<N, T>, 2> closest;
        };

        // These two functions are exact for computing Result::sqrDistance
        // when T is a rational type.
        Result operator()(Segment<N, T> const& segment0, Segment<N, T> const& segment1)
        {
            return operator()(segment0.p[0], segment0.p[1], segment1.p[0], segment1.p[1]);
        }

        Result operator()(Vector<N, T> const& P0, Vector<N, T> const& P1,
            Vector<N, T> const& Q0, Vector<N, T> const& Q1)
        {
            Vector<N, T> P1mP0 = P1 - P0;
            Vector<N, T> Q1mQ0 = Q1 - Q0;
            Vector<N, T> P0mQ0 = P0 - Q0;
            T a = Dot(P1mP0, P1mP0);
            T b = Dot(P1mP0, Q1mQ0);
            T c = Dot(Q1mQ0, Q1mQ0);
            T d = Dot(P1mP0, P0mQ0);
            T e = Dot(Q1mQ0, P0mQ0);
            T det = a * c - b * b;
            T s, t, nd, bmd, bte, ctd, bpe, ate, btd;

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            if (det > zero)
            {
                bte = b * e;
                ctd = c * d;
                if (bte <= ctd)  // s <= 0
                {
                    s = zero;
                    if (e <= zero)  // t <= 0
                    {
                        // region 6
                        t = zero;
                        nd = -d;
                        if (nd >= a)
                        {
                            s = one;
                        }
                        else if (nd > zero)
                        {
                            s = nd / a;
                        }
                        // else: s is already zero
                    }
                    else if (e < c)  // 0 < t < 1
                    {
                        // region 5
                        t = e / c;
                    }
                    else  // t >= 1
                    {
                        // region 4
                        t = one;
                        bmd = b - d;
                        if (bmd >= a)
                        {
                            s = one;
                        }
                        else if (bmd > zero)
                        {
                            s = bmd / a;
                        }
                        // else:  s is already zero
                    }
                }
                else  // s > 0
                {
                    s = bte - ctd;
                    if (s >= det)  // s >= 1
                    {
                        // s = 1
                        s = one;
                        bpe = b + e;
                        if (bpe <= zero)  // t <= 0
                        {
                            // region 8
                            t = zero;
                            nd = -d;
                            if (nd <= zero)
                            {
                                s = zero;
                            }
                            else if (nd < a)
                            {
                                s = nd / a;
                            }
                            // else: s is already one
                        }
                        else if (bpe < c)  // 0 < t < 1
                        {
                            // region 1
                            t = bpe / c;
                        }
                        else  // t >= 1
                        {
                            // region 2
                            t = one;
                            bmd = b - d;
                            if (bmd <= zero)
                            {
                                s = zero;
                            }
                            else if (bmd < a)
                            {
                                s = bmd / a;
                            }
                            // else:  s is already one
                        }
                    }
                    else  // 0 < s < 1
                    {
                        ate = a * e;
                        btd = b * d;
                        if (ate <= btd)  // t <= 0
                        {
                            // region 7
                            t = zero;
                            nd = -d;
                            if (nd <= zero)
                            {
                                s = zero;
                            }
                            else if (nd >= a)
                            {
                                s = one;
                            }
                            else
                            {
                                s = nd / a;
                            }
                        }
                        else  // t > 0
                        {
                            t = ate - btd;
                            if (t >= det)  // t >= 1
                            {
                                // region 3
                                t = one;
                                bmd = b - d;
                                if (bmd <= zero)
                                {
                                    s = zero;
                                }
                                else if (bmd >= a)
                                {
                                    s = one;
                                }
                                else
                                {
                                    s = bmd / a;
                                }
                            }
                            else  // 0 < t < 1
                            {
                                // region 0
                                s /= det;
                                t /= det;
                            }
                        }
                    }
                }
            }
            else
            {
                // The segments are parallel. The quadratic factors to
                //   R(s,t) = a*(s-(b/a)*t)^2 + 2*d*(s - (b/a)*t) + f
                // where a*c = b^2, e = b*d/a, f = |P0-Q0|^2, and b is not
                // zero. R is constant along lines of the form s-(b/a)*t = k
                // and its occurs on the line a*s - b*t + d = 0. This line
                // must intersect both the s-axis and the t-axis because 'a'
                // and 'b' are not zero. Because of parallelism, the line is
                // also represented by -b*s + c*t - e = 0.
                //
                // The code determines an edge of the domain [0,1]^2 that
                // intersects the minimum line, or if none of the edges
                // intersect, it determines the closest corner to the minimum
                // line. The conditionals are designed to test first for
                // intersection with the t-axis (s = 0) using
                // -b*s + c*t - e = 0 and then with the s-axis (t = 0) using
                // a*s - b*t + d = 0.

                // When s = 0, solve c*t - e = 0 (t = e/c).
                if (e <= zero)  // t <= 0
                {
                    // Now solve a*s - b*t + d = 0 for t = 0 (s = -d/a).
                    t = zero;
                    nd = -d;
                    if (nd <= zero)  // s <= 0
                    {
                        // region 6
                        s = zero;
                    }
                    else if (nd >= a)  // s >= 1
                    {
                        // region 8
                        s = one;
                    }
                    else  // 0 < s < 1
                    {
                        // region 7
                        s = nd / a;
                    }
                }
                else if (e >= c)  // t >= 1
                {
                    // Now solve a*s - b*t + d = 0 for t = 1 (s = (b-d)/a).
                    t = one;
                    bmd = b - d;
                    if (bmd <= zero)  // s <= 0
                    {
                        // region 4
                        s = zero;
                    }
                    else if (bmd >= a)  // s >= 1
                    {
                        // region 2
                        s = one;
                    }
                    else  // 0 < s < 1
                    {
                        // region 3
                        s = bmd / a;
                    }
                }
                else  // 0 < t < 1
                {
                    // The point (0,e/c) is on the line and domain, so we have
                    // one point at which R is a minimum.
                    s = zero;
                    t = e / c;
                }
            }

            Result result{};
            result.parameter[0] = s;
            result.parameter[1] = t;
            result.closest[0] = P0 + s * P1mP0;
            result.closest[1] = Q0 + t * Q1mQ0;
            Vector<N, T> diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

        // These two functions are exact for computing Result::sqrDistance
        // when T is a rational type. However, it is generally more robust
        // than the operator()(...) functions when T is a floating-point type.
        Result ComputeRobust(Segment<N, T> const& segment0, Segment<N, T> const& segment1)
        {
            return ComputeRobust(segment0.p[0], segment0.p[1], segment1.p[0], segment1.p[1]);
        }

        Result ComputeRobust(Vector<N, T> const& P0, Vector<N, T> const& P1,
            Vector<N, T> const& Q0, Vector<N, T> const& Q1)
        {
            Result result{};

            // The code allows degenerate line segments; that is, P0 and P1
            // can be the same point or Q0 and Q1 can be the same point.  The
            // quadratic function for squared distance between the segment is
            //   R(s,t) = a*s^2 - 2*b*s*t + c*t^2 + 2*d*s - 2*e*t + f
            // for (s,t) in [0,1]^2 where
            //   a = Dot(P1-P0,P1-P0), b = Dot(P1-P0,Q1-Q0), c = Dot(Q1-Q0,Q1-Q0),
            //   d = Dot(P1-P0,P0-Q0), e = Dot(Q1-Q0,P0-Q0), f = Dot(P0-Q0,P0-Q0)
            Vector<N, T> P1mP0 = P1 - P0;
            Vector<N, T> Q1mQ0 = Q1 - Q0;
            Vector<N, T> P0mQ0 = P0 - Q0;
            T a = Dot(P1mP0, P1mP0);
            T b = Dot(P1mP0, Q1mQ0);
            T c = Dot(Q1mQ0, Q1mQ0);
            T d = Dot(P1mP0, P0mQ0);
            T e = Dot(Q1mQ0, P0mQ0);

            // The derivatives dR/ds(i,j) at the four corners of the domain.
            T f00 = d;
            T f10 = f00 + a;
            T f01 = f00 - b;
            T f11 = f10 - b;

            // The derivatives dR/dt(i,j) at the four corners of the domain.
            T g00 = -e;
            T g10 = g00 - b;
            T g01 = g00 + c;
            T g11 = g10 + c;

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            if (a > zero && c > zero)
            {
                // Compute the solutions to dR/ds(s0,0) = 0 and
                // dR/ds(s1,1) = 0.  The location of sI on the s-axis is
                // stored in classifyI (I = 0 or 1).  If sI <= 0, classifyI
                // is -1.  If sI >= 1, classifyI is 1.  If 0 < sI < 1,
                // classifyI is 0.  This information helps determine where to
                // search for the minimum point (s,t).  The fij values are
                // dR/ds(i,j) for i and j in {0,1}.

                std::array<T, 2> sValue
                {
                    GetClampedRoot(a, f00, f10),
                    GetClampedRoot(a, f01, f11)
                };

                std::array<int32_t, 2> classify{};
                for (size_t i = 0; i < 2; ++i)
                {
                    if (sValue[i] <= zero)
                    {
                        classify[i] = -1;
                    }
                    else if (sValue[i] >= one)
                    {
                        classify[i] = +1;
                    }
                    else
                    {
                        classify[i] = 0;
                    }
                }

                if (classify[0] == -1 && classify[1] == -1)
                {
                    // The minimum must occur on s = 0 for 0 <= t <= 1.
                    result.parameter[0] = zero;
                    result.parameter[1] = GetClampedRoot(c, g00, g01);
                }
                else if (classify[0] == +1 && classify[1] == +1)
                {
                    // The minimum must occur on s = 1 for 0 <= t <= 1.
                    result.parameter[0] = one;
                    result.parameter[1] = GetClampedRoot(c, g10, g11);
                }
                else
                {
                    // The line dR/ds = 0 intersects the domain [0,1]^2 in a
                    // nondegenerate segment. Compute the endpoints of that
                    // segment, end[0] and end[1]. The edge[i] flag tells you
                    // on which domain edge end[i] lives: 0 (s=0), 1 (s=1),
                    // 2 (t=0), 3 (t=1).
                    std::array<int32_t, 2> edge{ 0, 0 };
                    std::array<std::array<T, 2>, 2> end{};
                    ComputeIntersection(sValue, classify, b, f00, f10, edge, end);

                    // The directional derivative of R along the segment of
                    // intersection is
                    //   H(z) = (end[1][1]-end[1][0]) *
                    //          dR/dt((1-z)*end[0] + z*end[1])
                    // for z in [0,1]. The formula uses the fact that
                    // dR/ds = 0 on the segment. Compute the minimum of
                    // H on [0,1].
                    ComputeMinimumParameters(edge, end, b, c, e, g00, g10,
                        g01, g11, result.parameter);
                }
            }
            else
            {
                if (a > zero)
                {
                    // The Q-segment is degenerate (Q0 and Q1 are the same
                    // point) and the quadratic is R(s,0) = a*s^2 + 2*d*s + f
                    // and has (half) first derivative F(t) = a*s + d.  The
                    // closest P-point is interior to the P-segment when
                    // F(0) < 0 and F(1) > 0.
                    result.parameter[0] = GetClampedRoot(a, f00, f10);
                    result.parameter[1] = zero;
                }
                else if (c > zero)
                {
                    // The P-segment is degenerate (P0 and P1 are the same
                    // point) and the quadratic is R(0,t) = c*t^2 - 2*e*t + f
                    // and has (half) first derivative G(t) = c*t - e.  The
                    // closest Q-point is interior to the Q-segment when
                    // G(0) < 0 and G(1) > 0.
                    result.parameter[0] = zero;
                    result.parameter[1] = GetClampedRoot(c, g00, g01);
                }
                else
                {
                    // P-segment and Q-segment are degenerate.
                    result.parameter[0] = zero;
                    result.parameter[1] = zero;
                }
            }


            result.closest[0] =
                (one - result.parameter[0]) * P0 + result.parameter[0] * P1;
            result.closest[1] =
                (one - result.parameter[1]) * Q0 + result.parameter[1] * Q1;
            Vector<N, T> diff = result.closest[0] - result.closest[1];
            result.sqrDistance = Dot(diff, diff);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

    private:
        // Compute the root of h(z) = h0 + slope*z and clamp it to the interval
        // [0,1]. It is required that for h1 = h(1), either (h0 < 0 and h1 > 0)
        // or (h0 > 0 and h1 < 0).
        static T GetClampedRoot(T const& slope, T const& h0, T const& h1)
        {
            // Theoretically, r is in (0,1). However, when the slope is
            // nearly zero, then so are h0 and h1. Significant numerical
            // rounding problems can occur when using floating-point
            // arithmetic. If the rounding causes r to be outside the
            // interval, clamp it. It is possible that r is in (0,1) and has
            // rounding errors, but because h0 and h1 are both nearly zero,
            // the quadratic is nearly constant on (0,1). Any choice of p
            // should not cause undesirable accuracy problems for the final
            // distance computation.
            //
            // NOTE: You can use bisection to recompute the root or even use
            // bisection to compute the root and skip the division. This is
            // generally slower, which might be a problem for high-performance
            // applications.

            T const zero = static_cast<T>(0);
            T r;
            if (h0 < zero)
            {
                T const one = static_cast<T>(1);
                if (h1 > zero)
                {
                    r = -h0 / slope;
                    if (r > one)
                    {
                        r = static_cast<T>(0.5);
                    }
                    // The slope is positive and -h0 is positive, so there is
                    // no need to test for a negative value and clamp it.
                }
                else
                {
                    r = one;
                }
            }
            else
            {
                r = zero;
            }
            return r;
        }

        // Compute the intersection of the line dR/ds = 0 with the domain
        // [0,1]^2. The direction of the line dR/ds is conjugate to (1,0),
        // so the algorithm for minimization is effectively the conjugate
        // gradient algorithm for a quadratic function.
        static void ComputeIntersection(std::array<T, 2> const& sValue,
            std::array<int32_t, 2> const& classify, T const& b, T const& f00,
            T const& f10, std::array<int32_t, 2>& edge,
            std::array<std::array<T, 2>, 2>& end)
        {
            // The divisions are theoretically numbers in [0,1]. Numerical
            // rounding errors might cause the result to be outside the
            // interval. When this happens, it must be that both numerator
            // and denominator are nearly zero. The denominator is nearly
            // zero when the segments are nearly perpendicular. The
            // numerator is nearly zero when the P-segment is nearly
            // degenerate (f00 = a is small). The choice of 0.5 should not
            // cause significant accuracy problems.
            //
            // NOTE: You can use bisection to recompute the root or even use
            // bisection to compute the root and skip the division. This is
            // generally slower, which might be a problem for high-performance
            // applications.

            T const zero = static_cast<T>(0);
            T const half = static_cast<T>(0.5);
            T const one = static_cast<T>(1);
            if (classify[0] < 0)
            {
                edge[0] = 0;
                end[0][0] = zero;
                end[0][1] = f00 / b;
                if (end[0][1] < zero || end[0][1] > one)
                {
                    end[0][1] = half;
                }

                if (classify[1] == 0)
                {
                    edge[1] = 3;
                    end[1][0] = sValue[1];
                    end[1][1] = one;
                }
                else  // classify[1] > 0
                {
                    edge[1] = 1;
                    end[1][0] = one;
                    end[1][1] = f10 / b;
                    if (end[1][1] < zero || end[1][1] > one)
                    {
                        end[1][1] = half;
                    }
                }
            }
            else if (classify[0] == 0)
            {
                edge[0] = 2;
                end[0][0] = sValue[0];
                end[0][1] = zero;

                if (classify[1] < 0)
                {
                    edge[1] = 0;
                    end[1][0] = zero;
                    end[1][1] = f00 / b;
                    if (end[1][1] < zero || end[1][1] > one)
                    {
                        end[1][1] = half;
                    }
                }
                else if (classify[1] == 0)
                {
                    edge[1] = 3;
                    end[1][0] = sValue[1];
                    end[1][1] = one;
                }
                else
                {
                    edge[1] = 1;
                    end[1][0] = one;
                    end[1][1] = f10 / b;
                    if (end[1][1] < zero || end[1][1] > one)
                    {
                        end[1][1] = half;
                    }
                }
            }
            else  // classify[0] > 0
            {
                edge[0] = 1;
                end[0][0] = one;
                end[0][1] = f10 / b;
                if (end[0][1] < zero || end[0][1] > one)
                {
                    end[0][1] = half;
                }

                if (classify[1] == 0)
                {
                    edge[1] = 3;
                    end[1][0] = sValue[1];
                    end[1][1] = one;
                }
                else
                {
                    edge[1] = 0;
                    end[1][0] = zero;
                    end[1][1] = f00 / b;
                    if (end[1][1] < zero || end[1][1] > one)
                    {
                        end[1][1] = half;
                    }
                }
            }
        }

        // Compute the location of the minimum of R on the segment of
        // intersection for the line dR/ds = 0 and the domain [0,1]^2.
        static void ComputeMinimumParameters(std::array<int32_t, 2> const& edge,
            std::array<std::array<T, 2>, 2> const& end, T const& b, T const& c,
            T const& e, T const& g00, T const& g10, T const& g01, T const& g11,
            std::array<T, 2>& parameter)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T delta = end[1][1] - end[0][1];
            T h0 = delta * (-b * end[0][0] + c * end[0][1] - e);
            if (h0 >= zero)
            {
                if (edge[0] == 0)
                {
                    parameter[0] = zero;
                    parameter[1] = GetClampedRoot(c, g00, g01);
                }
                else if (edge[0] == 1)
                {
                    parameter[0] = one;
                    parameter[1] = GetClampedRoot(c, g10, g11);
                }
                else
                {
                    parameter[0] = end[0][0];
                    parameter[1] = end[0][1];
                }
            }
            else
            {
                T h1 = delta * (-b * end[1][0] + c * end[1][1] - e);
                if (h1 <= zero)
                {
                    if (edge[1] == 0)
                    {
                        parameter[0] = zero;
                        parameter[1] = GetClampedRoot(c, g00, g01);
                    }
                    else if (edge[1] == 1)
                    {
                        parameter[0] = one;
                        parameter[1] = GetClampedRoot(c, g10, g11);
                    }
                    else
                    {
                        parameter[0] = end[1][0];
                        parameter[1] = end[1][1];
                    }
                }
                else  // h0 < 0 and h1 > 0
                {
                    T z = std::min(std::max(h0 / (h0 - h1), zero), one);
                    T omz = one - z;
                    parameter[0] = omz * end[0][0] + z * end[1][0];
                    parameter[1] = omz * end[0][1] + z * end[1][1];
                }
            }
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPSegmentSegment = DCPQuery<T, Segment<N, T>, Segment<N, T>>;

    template <typename T>
    using DCPSegment2Segment2 = DCPSegmentSegment<2, T>;

    template <typename T>
    using DCPSegment3Segment3 = DCPSegmentSegment<3, T>;
}
