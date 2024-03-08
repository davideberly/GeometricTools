// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.11.27

#pragma once

// The Solve functions return the real-valued roots of the polynomial
//   p(x) = p[0] + p[1] * x + p[2] * x^2 + ... + p[d] * x^d
// The derivative is
//   p'(x) = p[1] + 2 * p[2] * x + ... + d * p[d] * x^{d-1}.
// If r0 and r1 are consecutive roots of p'(x), say r0 < r1, then p(t) is
// monotonic on the open interval (r0,r1). Additionally, if
// p(r0) * p(r1) <= 0, then p(x) has a unique root on the closed interval
// [r0,r1]. Using this observation, one can compute the derivatives through
// order d for p(x), find roots for the derivative of order k+1, and then
// use these to bound roots for the derivative of order k. This is a
// recursive formulation, implemented as recursive function calls. TODO:
// Replace the recursive function calls with simulated recursion to avoid
// overflowing the call stack.
//
// The old code, now deprecated, is RootsPolynomial<T>::Find(...). It uses
// only floating-point arithmetic. The rounding errors in computing the
// coefficients of the polynomial derivatives can be catastrophic, leading
// to extremely inaccurate roots. Estimation of roots to the order k+1
// derivative uses bisection which is fast. Unfortunately, fast and
// inaccurate is not desirable. The old code does support a template type
// for rational numbers, but the bisection takes so long that it effectively
// never converges.
// 
// The new code uses a mixture of rational arithmetic and floating-point
// arithmetic. The coefficients of the polynomial derivatives are computed
// using rational arithmetic. When it comes time for bisection, intervals
// are located for which the rational polynomial values at the endpoints
// have opposite signs. Rational-valued bisection effectively does not
// converge (the number of bits in a rational is extremely large), so instead
// the polynomial coefficients are rounded to the nearest floating-point
// numbers and the polynomial is evaluated at the endpoints using
// floating-point arithmetic. Special handling is given to the case where
// the rational polynomial values have opposite signs but the floating-point
// polynomial values do not.

#include <Mathematics/ArbitraryPrecision.h>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <limits>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace gte
{
    template <typename T>
    class RootsGeneralPolynomial
    {
    public:
        using Rational = BSRational<UIntegerAP32>;

        static void Solve(std::vector<T> const& p, bool useThreading, std::vector<T>& roots)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "For now, only floating-point types are supported.");

            roots.clear();

            // The order of p is p.size(). The degree of p is p.size() - 1.
            if (p.size() <= 1)
            {
                // The polynomial is identically a constant. Do not report
                // roots even when that constant is 0.
                return;
            }

            // Remove high-order zero-valued coefficients.
            size_t degree = p.size() - 1;
            while (degree >= 1 && p[degree] == static_cast<T>(0))
            {
                --degree;
            }

            if (degree == 1)
            {
                roots.push_back(-p[0] / p[1]);
                return;
            }

            if (degree == 0)
            {
                // The polynomial is identically a constant. Do not report
                // roots even when that constant is 0.
                return;
            }

            // At this time the degree is at least 2. Create a polynomial for
            // p(x) that has rational coefficients.
            std::vector<Rational> rP(p.size());
            for (size_t i = 0; i <= degree; ++i)
            {
                rP[i] = p[i];
            }

            // Make the polynomial monic. Theoretically, this is irrelevant
            // when estimating roots for a polynomial with rational
            // coefficients. However, during the recursion the rational
            // coefficients can be quite large, so using a monic polynomial
            // helps with robustness.
            Rational const rOne = static_cast<Rational>(1);
            auto& rLast = rP[degree];
            if (rLast != rOne)
            {
                for (size_t i = 0; i < degree; ++i)
                {
                    rP[i] /= rLast;
                }
                rP[degree] = static_cast<Rational>(1);
            }

            // Compute Cauchy bounds and solve for roots using recursion on
            // the polynomial degree.
            std::vector<Rational> rRoots{};
            InitiateSolver(rP, useThreading, rRoots);

            // Convert the rational roots to floating-point.
            roots.resize(rRoots.size());
            for (size_t i = 0; i < rRoots.size(); ++i)
            {
                roots[i] = static_cast<T>(rRoots[i]);
            }
        }

        static void Solve(std::vector<Rational> const& rP, bool useThreading, std::vector<Rational>& rRoots)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "For now, only floating-point types are supported.");

            rRoots.clear();

            // The order of p is p.size(). The degree of p is p.size() - 1.
            if (rP.size() <= 1)
            {
                // The polynomial is identically a constant. Do not report
                // roots even when that constant is 0.
                return;
            }

            // Remove high-order zero-valued coefficients.
            size_t degree = rP.size() - 1;
            while (degree >= 1 && rP[degree].GetSign() == 0)
            {
                --degree;
            }

            if (degree == 1)
            {
                rRoots.push_back(-rP[0] / rP[1]);
                return;
            }

            if (degree == 0)
            {
                // The polynomial is identically a constant. Do not report
                // roots even when that constant is 0.
                return;
            }

            // At this time the degree is at least 2.

            // Make the polynomial monic. Theoretically, this is irrelevant
            // when estimating roots for a polynomial with rational
            // coefficients. However, during the recursion the rational
            // coefficients can be quite large, so using a monic polynomial
            // helps with robustness.
            Rational const rOne = static_cast<Rational>(1);
            std::vector<Rational> rPMonic(rP.size());
            auto& rLast = rP[degree];
            if (rLast != rOne)
            {
                for (size_t i = 0; i < degree; ++i)
                {
                    rPMonic[i] = rP[i] / rLast;
                }
                rPMonic[degree] = rOne;
            }
            else
            {
                rPMonic = rP;
            }

            // Compute Cauchy bounds and solve for roots using recursion on
            // the polynomial degree.
            InitiateSolver(rPMonic, useThreading, rRoots);
        }

    private:
        static void InitiateSolver(std::vector<Rational> const& rP, bool useThreading,
            std::vector<Rational>& rRoots)
        {
            // Compute Cauchy bounds to obtain an interval containing the
            // roots of p(x). At this time the polynomial is monic.
            size_t degree = rP.size() - 1;
            Rational rCauchyBound = std::fabs(rP[0]);
            for (size_t i = 1; i + 1 <= degree; ++i)
            {
                Rational rCandidate = std::fabs(rP[i]);
                if (rCandidate > rCauchyBound)
                {
                    rCauchyBound = rCandidate;
                }
            }
            rCauchyBound += static_cast<Rational>(1);

            // Solve recursively in degree.
            SolveRecursive(rP, -rCauchyBound, rCauchyBound, useThreading, rRoots);
        }

        static void SolveRecursive(std::vector<Rational> const& rP, Rational const& rXMin,
            Rational const& rXMax, bool useThreading, std::vector<Rational>& rRoots)
        {
            // The base of the recursion.
            Rational const rZero = static_cast<Rational>(0);
            size_t degree = rP.size() - 1;
            if (degree == 1)
            {
                if (rP[1] != rZero)
                {
                    rRoots.push_back(Rational(-rP[0] / rP[1]));
                }
                return;
            }

            // Compute the derivative polynomial p'(x) of p(x) using rational
            // numbers.
            std::vector<Rational> rPDerivative(degree);
            for (size_t i0 = 0, i1 = 1; i1 <= degree; i0 = i1++)
            {
                rPDerivative[i0] = rP[i1] * static_cast<Rational>(i1);
            }

            // Estimate the roots of the derivative polynomial.
            std::vector<Rational> rRootsDerivative{};
            SolveRecursive(rPDerivative, rXMin, rXMax, true, rRootsDerivative);

            // Round the coefficients of rP(x) to floating-point numbers. This
            // is used for fast performance by floating-point-based bisection.
            std::vector<T> tP(rP.size());
            for (size_t i = 0; i < rP.size(); ++i)
            {
                tP[i] = static_cast<T>(rP[i]);
            }

            // The polynomial is monotonic between consecutive roots of the
            // derivative. This feature and the polynomial values at the
            // derivative roots are used to compute polynomial roots via
            // bisection.
            Rational rRoot{};
            if (rRootsDerivative.size() > 0)
            {
                if (useThreading)
                {
                    std::vector<std::thread> process(rRootsDerivative.size() + 1);
                    std::vector<std::pair<Rational, bool>> rootInfo(process.size(),
                        std::make_pair(Rational(0), false));

                    // Let rXUpper = rRootsDerivative.front(). Estimate a
                    // root, if any, on the interval [rXMin,rXUpper].
                    process.front() = std::thread(
                        [&tP, &rP, &rXMin, &rRootsDerivative, &rootInfo]()
                        {
                            rootInfo.front().second = Bisect(tP, rP,
                            rXMin, rRootsDerivative.front(),
                            rootInfo.front().first);
                        });

                    // Let rXLower = rRootsDerivative[i] and let rXUpper =
                    // rRootsDerivative[i+1]. Estimate a root, if any, on
                    // [rXLower,rXUpper].
                    for (size_t i0 = 0, i1 = 1; i1 < rRootsDerivative.size(); i0 = i1++)
                    {
                        // The loop counters must be passed by value.
                        // Otherwise, they will be modified for the next
                        // process[] constructor but the previous process[] is
                        // trying to use the older values.
                        process[i1] = std::thread(
                            [&tP, &rP, i0, i1, &rRootsDerivative, &rootInfo]()
                            {
                                rootInfo[i1].second = Bisect(tP, rP,
                                rRootsDerivative[i0], rRootsDerivative[i1],
                                rootInfo[i1].first);
                            });
                    }

                    // Let rXLower = rRootsDerivative.back(). Estimate a root,
                    // if any, on the interval [rXLower,rXMax].
                    process.back() = std::thread(
                        [&tP, &rP, &rRootsDerivative, &rXMax, &rootInfo]()
                        {
                            rootInfo.back().second = Bisect(tP, rP,
                            rRootsDerivative.back(), rXMax,
                            rootInfo.back().first);
                        });

                    for (size_t i = 0; i < process.size(); ++i)
                    {
                        process[i].join();
                        if (rootInfo[i].second)
                        {
                            rRoots.push_back(rootInfo[i].first);
                        }
                    }
                }
                else
                {
                    // Let rXUpper = rRootsDerivative.front(). Estimate a root, if
                    // any, on the interval [rXMin,rXUpper].
                    if (Bisect(tP, rP, rXMin, rRootsDerivative.front(), rRoot))
                    {
                        rRoots.push_back(rRoot);
                    }

                    // Let rXLower = rRootsDerivative[i] and let rXUpper =
                    // rRootsDerivative[i+1]. Estimate a root, if any, on
                    // [rXLower,rXUpper].
                    for (size_t i0 = 0, i1 = 1; i1 < rRootsDerivative.size(); i0 = i1++)
                    {
                        if (Bisect(tP, rP, rRootsDerivative[i0], rRootsDerivative[i1], rRoot))
                        {
                            rRoots.push_back(rRoot);
                        }
                    }

                    // Let rXLower = rRootsDerivative.back(). Estimate a root, if
                    // any, on the interval [rXLower,rXMax].
                    if (Bisect(tP, rP, rRootsDerivative.back(), rXMax, rRoot))
                    {
                        rRoots.push_back(rRoot);
                    }
                }
            }
            else
            {
                // The polynomial is monotone on [rXMin,rXMax], so it has at
                // most one root.
                if (Bisect(tP, rP, rXMin, rXMax, rRoot))
                {
                    rRoots.push_back(rRoot);
                }
            }
        }

        static bool Bisect(std::vector<T> const& tP, std::vector<Rational> const& rP,
            Rational const& rXMin, Rational const& rXMax, Rational& rRoot)
        {
            // The first interval is [-cauchyBound,derivativeRoot.first]. It
            // is possible that p'(x) has a root smaller than the minimum root
            // of p(x) in which case the incoming interval endpoints are not
            // correctly ordered. Such an interval cannot produce a root of
            // p(x). The last interval is [derivative.last,+cauchyBound]. It
            // is possible that p'(x) has a root larger than the maximum root
            // of p(x) in which case the incoming interval endpoints are not
            // correctly ordered. Such an interval cannot produce a root of
            // p(x).
            if (rXMin >= rXMax)
            {
                return false;
            }

            Rational rPMin = Evaluate(rP, rXMin);
            int32_t signRPMin = rPMin.GetSign();
            if (signRPMin == 0)
            {
                rRoot = rXMin;
                return true;
            }

            Rational rPMax = Evaluate(rP, rXMax);
            int32_t signRPMax = rPMax.GetSign();
            if (signRPMax == 0)
            {
                // Do not return the root rXMax. The next interval will be
                // responsible for managing this root.
                return false;
            }

            if (signRPMin * signRPMax > 0)
            {
                // The polynomial p(x) is monotone on [rXMin,rXMax], so it
                // cannot have a root on the interval.
                return false;
            }

            // At this time rPMin and rPMax have opposite signs. There must
            // be a unique root on [rXMin,rXMax] because the derivative is not
            // zero on the interval which implies that p(x) is monotone on
            // the interval.

            // Use floating-point arithmetic for speed. Be aware that the
            // conversions from rational numbers to floating-point numbers
            // can affect sign tests. Recompute the endpoint tests for the
            // floating-point numbers.
            T const zero = static_cast<T>(0);

            T tXMin = static_cast<T>(rXMin);
            T tPMin = Evaluate(tP, tXMin);
            int32_t signTPMin = (tPMin > zero ? +1 : (tPMin < zero ? -1 : 0));
            if (signTPMin == 0)
            {
                rRoot = rXMin;
                return true;
            }

            T tXMax = static_cast<T>(rXMax);
            T tPMax = Evaluate(tP, tXMax);
            int32_t signTPMax = (tPMax > zero ? +1 : (tPMax < zero ? -1 : 0));
            if (signTPMax == 0)
            {
                // Do not return the root rXMax. The next interval will be
                // responsible for managing this root.
                return false;
            }

            if (signTPMin * signTPMax > 0)
            {
                // We know that rPMin and rPMax have opposite signs. Rounding
                // errors lead to tPMin and tPMax having the same sign. Rather
                // than return 'false' as in the rational arithmetic case,
                // return 'true' with the root given by the intersection of
                // the x-axis with the line through (rXMin,rPMin) and
                // (rXMax,rPMax). This amounts to approximating a nearly flat
                // polynomial on the interval by a line segment.
                rRoot = (rXMin * rPMax - rXMax * rPMin) / (rPMax - rPMin);
                return true;
            }

            // At this time tPMin and tPMax have opposite signs. Bisect to
            // find a root. In theory the root is unique, but floating-point
            // rounding errors can lead to multiple roots (all approximately
            // the same floating-point number).

            // The maximum number of iterations suffices for convergence when
            // using floating-point numbers ('float' or 'double').
            size_t const maxIterations = 4096;
            T const tHalf = static_cast<T>(0.5);
            T tRoot{};
            for (size_t i = 0; i < maxIterations; ++i)
            {
                tRoot = tHalf * (tXMin + tXMax);

                // The test is designed for 'float' or 'double' when tXMin
                // and tXMax are consecutive floating-point numbers.
                if (tRoot == tXMin || tRoot == tXMax)
                {
                    break;
                }

                T tPAtRoot = Evaluate(tP, tRoot);
                int32_t signTPAtRoot = (tPAtRoot > zero ? +1 : (tPAtRoot < zero ? -1 : 0));
                int32_t sign = signTPAtRoot * signTPMin;
                if (sign < 0)
                {
                    tXMax = tRoot;
                    tPMax = tPAtRoot;
                }
                else if (sign > 0)
                {
                    tXMin = tRoot;
                    tPMin = tPAtRoot;
                }
                else
                {
                    // The root is exactly tRoot.
                    break;
                }
            }

            rRoot = static_cast<Rational>(tRoot);
            return true;
        }

        template <typename Numeric>
        static Numeric Evaluate(std::vector<Numeric> const& p, Numeric const& x)
        {
            size_t constexpr smax = std::numeric_limits<size_t>::max();
            size_t i = p.size() - 1;
            Numeric result = p[i];
            while (--i != smax)
            {
                result = x * result + p[i];
            }
            return result;
        }
    };
}
