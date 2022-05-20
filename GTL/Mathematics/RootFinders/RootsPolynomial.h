// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.20

#pragma once

// Compute all the real-valued roots of a polynomial p(x) whose
// coefficients are real valued.

#include <GTL/Mathematics/Arithmetic/ArbitraryPrecision.h>
#include <GTL/Mathematics/Algebra/Polynomial.h>
#include <GTL/Mathematics/RootFinders/RootsBisection1.h>
#include <GTL/Mathematics/Algebra/Matrix.h>
#include <limits>
#include <numeric>

namespace gtl
{
    class RootsPolynomial
    {
    public:
        using BSN = BSNumber<UIntegerAP32>;

        RootsPolynomial(size_t maxBisections, size_t precision)
            :
            mMaxBisections(maxBisections),
            mPrecision(precision),
            mMaxDegree(0),
            mPascal{},
            mRootStatus(unknownRootStatus),
            mRootIntervals{}
        {
            GTL_ARGUMENT_ASSERT(
                mMaxBisections > 0,
                "The maximum iterations must be positive.");

            GTL_ARGUMENT_ASSERT(
                mPrecision > 0,
                "The precision must be positive.");

            ComputePascalsTriangle();
        }

        // Estimate the roots for a polynomial p(x) with floating-point
        // coefficients. Internally, computations use dyadic rationals
        // (BSNumber). The root finder is faster than the other one where
        // p(x) has BSNumber coefficients. However, floating-point
        // rounding errors can lead to failure to detect nonsimple roots
        // (of even multiplicity). It is also possible that a root of
        // even multiplicity is detected as two roots that are nearly the
        // same number. Root intervals can be requested in case you want
        // to refine the root computations.
        template <typename T, IsFPType<T> = 0>
        void operator()(Polynomial1<T> const& p, std::vector<BSN>& roots,
            bool wantRootIntervals = false)
        {
            roots.reserve(p.GetDegree());
            roots.clear();
            mRootStatus = unknownRootStatus;
            mRootIntervals.reserve(p.GetDegree());
            mRootIntervals.clear();

            // Increase the size of Pascal's triangle if necessary. The
            // elements are used for cofficient construction of the derivative
            // of p(x).
            AddRowsToPascalsTriangle(p.GetDegree());

            // Convert p(x) to reduced(x) where reduced(x) has a nonzero
            // constant term and a nonzero leading term.
            Polynomial1<BSN> reduced;
            int32_t iMin = RemoveLeadingAndTrailingZeros(p, reduced);
            if (iMin == nonzeroConstant)
            {
                mRootStatus = nonzeroConstant;
                return;
            }
            if (iMin == zeroConstant)
            {
                mRootStatus = zeroConstant;
                return;
            }

            // Let p(x) = sum_{i=0}^d p[i] * x^i be the input polynomial.
            // After eliminating the leading and trailing zeros, the
            // polynomial is
            //   p(x) = p[iMin] * x^{iMin} + ... + p[iMax] * x^{iMax}
            //        = x^{iMin} * (p[iMin] + ... + p[iMax] * x^{iMax - iMin})
            //        = x^{iMin} * r(x)
            // p(x) has the root 0 with multiplicity iMin. The reduced
            // polynomial r(x) has degree iMax - iMin and r(0) != 0. The
            // maximum number of real-valued roots of p(x) is iMax. The
            // maximum number of real-valued roots of r(x) is iMax - iMin.
            if (iMin > 0)
            {
                // The polynomial has the root 0 with multiplicity 'iMin'.
                mRootStatus = hasRepeatedRoots;
                roots.push_back(0);
                if (wantRootIntervals)
                {
                    mRootIntervals.push_back({ 0, 0 });
                }
            }

            // Compute an interval (-bound,+bound) that contains all the
            // real-valued roots.
            BSN bound;
            GetRootBound(reduced, bound);

            // Locate real-valued roots.
            Find(reduced, -bound, bound, roots, wantRootIntervals);

            SortRoots(roots, wantRootIntervals);
        }

        // Estimate the roots for a polynomial p(x) with BSNumber
        // coefficients. Internally, computations use dyadic rationals
        // (BSNumber). The root finder is slower than the other one where
        // p(x) has floating-point coefficients. The typical approach is
        // to compute a square-free factorization of p(x). Each factor
        // has simple roots, and a bisection search is used to compute
        // root intervals for the simple roots. For each interval,
        // numerical bisection can be used to locate the unique root in
        // that interval.
        // 
        // Currently, BSNumber integer multiplication is naive and slow
        // (schoolbook long multiplication, O(N^2) for two N-bit numbers).
        // As an alternative to square-free factorization, p(x) is
        // factored into the product of two polynomials, p(x) = g(x)*q(x)
        // where g(x) = GCD(p(x),p'(x)). If g = 1, the roots of p(x) are
        // estimated. If degree(g) > 1, the roots of p(x) are the roots
        // of q(x) = 0. Moreover, q(x) has only simple roots, and a
        // bisection search can be applied to bound the roots and estimate
        // them from the root intevals using numerical bisection. The
        // implementation does not attempt to compute root multiplicities,
        // something which is immediate from square-free factorization. The
        // nonsimple roots are solutions to g(x) = 0. Moreover, the roots
        // of q(x) contain the roots of g(x). TODO: Investigate whether a
        // performant algorithm can be to compute the multiplicities of
        // the roots given g(x), performant in the sense of currently having
        // a slow BSNumber integer multiplication.
        // 
        // Root intervals can be requested in case you want to refine the
        // root computations.
        template <typename T, IsAPType<T> = 0>
        void operator()(Polynomial1<T> const& p, std::vector<BSN>& roots,
            bool wantRootIntervals = false)
        {
            static_assert(
                !gtl::has_division_operator<T>::value,
                "The type must be arbitrary precision with no division.");

            roots.reserve(p.GetDegree());
            roots.clear();
            mRootStatus = unknownRootStatus;
            mRootIntervals.reserve(p.GetDegree());
            mRootIntervals.clear();

            // Increase the size of Pascal's triangle if necessary. The
            // elements are used for cofficient construction of the derivative
            // of p(x).
            AddRowsToPascalsTriangle(p.GetDegree());

            // Convert p(x) to reduced(x) where reduced(x) has a nonzero
            // constant term and a nonzero leading term.
            Polynomial1<BSN> reduced;
            int32_t iMin = RemoveLeadingAndTrailingZeros(p, reduced);
            if (iMin == nonzeroConstant)
            {
                mRootStatus = nonzeroConstant;
                return;
            }
            if (iMin == zeroConstant)
            {
                mRootStatus = zeroConstant;
                return;
            }

            // Let p(x) = sum_{i=0}^d p[i] * x^i be the input polynomial.
            // After eliminating the leading and trailing zeros, the
            // polynomial is
            //   p(x) = p[iMin] * x^{iMin} + ... + p[iMax] * x^{iMax}
            //        = x^{iMin} * (p[iMin] + ... + p[iMax] * x^{iMax - iMin})
            //        = x^{iMin} * r(x)
            // p(x) has the root 0 with multiplicity iMin. The reduced
            // polynomial r(x) has degree iMax - iMin and r(0) != 0. The
            // maximum number of real-valued roots of p(x) is iMax. The
            // maximum number of real-valued roots of r(x) is iMax - iMin.
            if (iMin > 0)
            {
                // The polynomial has the root 0 with multiplicity 'iMin'.
                mRootStatus = hasRepeatedRoots;
                roots.push_back(0);
                if (wantRootIntervals)
                {
                    mRootIntervals.push_back({ 0, 0 });
                }
            }

            // Compute an interval (-bound,+bound) that contains all the
            // real-valued roots.
            BSN bound;
            GetRootBound(reduced, bound);

            // Compute a polynomial q0 whose roots are those of 'reduced'
            // but are all simple. If r(x) is the 'reduced' polynomial,
            // and r'(x) is the derivative of r(x), then
            //   r(x) = g(x) * q0(x) / m0
            //   r'(x) = g(x) * q1(x) / m1
            //   q0(x) = 0 has simple roots
            //   g(x) = 0 has nonsimple roots
            // The set roots(g) are contained in the set roots(q0). The
            // polynomial g(x) has factors of the form h(x)^k for a
            // polynomial h(x) and power k > 0. The polynomial q0(x) has
            // a factor h(x).
            Polynomial1<BSN> reducedDerivative = GetDerivative(reduced);
            Polynomial1<BSN> g, q0, q1;
            BSN m0{}, m1{};
            GetPseudoExtendedGCD(reduced, reducedDerivative, g, m0, m1, q0, q1);

            if (g.GetDegree() == 0)
            {
                // All roots of p(x) are simple.
                if (mRootStatus == unknownRootStatus)
                {
                    mRootStatus = hasOnlySimpleRoots;
                }
                Find(reduced, -bound, bound, roots, wantRootIntervals);
            }
            else
            {
                // At least one root of p(x) has multiplicity larger than 1.
                mRootStatus = hasRepeatedRoots;
                Find(q0, -bound, bound, roots, wantRootIntervals);
            }

            SortRoots(roots, wantRootIntervals);
        }

        // Extended information about the roots. The function GetRootStatus()
        // returns one of the following 5 values.
        static int32_t constexpr nonzeroConstant = -1;
        static int32_t constexpr unknownRootStatus = 0;
        static int32_t constexpr hasOnlySimpleRoots = 1;
        static int32_t constexpr hasRepeatedRoots = 2;
        static int32_t constexpr zeroConstant = std::numeric_limits<int32_t>::max();

        inline int32_t GetRootStatus() const
        {
            return mRootStatus;
        }

        std::vector<std::array<BSN, 2>> const& GetRootIntervals() const
        {
            return mRootIntervals;
        }

    private:
        // Remove the leading and trailing zero-valued coefficients of the
        // polynomial. A return value of -1 indicates the polynomial is a
        // nonzero constant, so it has no roots. A return value of
        // std::numeric_limits<int32_t>::max() indicates the polynomial is
        // identically zero (infinitely many roots). A return value of 0
        // indicates the polynomial does not have a root of 0. A positive
        // return value indicates the polynomial has a root of 0 whose
        // multiplicity is the return value.
        template <typename T, IsFPType<T> = 0>
        static int32_t RemoveLeadingAndTrailingZeros(Polynomial1<T> const& p,
            Polynomial1<BSN>& reduced)
        {
            size_t degree = p.GetDegree();
            auto const& coefficient = p.GetCoefficients();

            size_t iMin;
            for (iMin = 0; iMin <= degree; ++iMin)
            {
                if (coefficient[iMin] != C_<T>(0))
                {
                    break;
                }
            }
            if (iMin > degree)
            {
                // The polynomial is identically zero.
                return zeroConstant;
            }

            size_t iMax;
            for (iMax = degree; iMax > iMin; --iMax)
            {
                if (coefficient[iMax] != C_<T>(0))
                {
                    break;
                }
            }
            if (iMax == iMin)
            {
                // The polynomial is a nonzero constant.
                return nonzeroConstant;
            }

            degree = iMax - iMin;
            reduced.SetDegree(degree);
            for (size_t i = iMin, j = 0; i <= iMax; ++i, ++j)
            {
                reduced[j] = p[i];
            }

            return static_cast<int32_t>(iMin);
        }

        template <typename T, IsAPType<T> = 0>
        static int32_t RemoveLeadingAndTrailingZeros(Polynomial1<T> const& p,
            Polynomial1<BSN>& reduced)
        {
            size_t degree = p.GetDegree();
            auto const& coefficient = p.GetCoefficients();

            size_t iMin;
            for (iMin = 0; iMin <= degree; ++iMin)
            {
                if (coefficient[iMin].GetSign() != 0)
                {
                    break;
                }
            }
            if (iMin > degree)
            {
                // The polynomial is identically zero.
                return zeroConstant;
            }

            size_t iMax;
            for (iMax = degree; iMax > iMin; --iMax)
            {
                if (coefficient[iMax].GetSign() != 0)
                {
                    break;
                }
            }
            if (iMax == iMin)
            {
                // The polynomial is a nonzero constant.
                return nonzeroConstant;
            }

            degree = iMax - iMin;
            reduced.SetDegree(degree);
            for (size_t i = iMin, j = 0; i <= iMax; ++i, ++j)
            {
                reduced[j] = p[i];
            }

            return static_cast<int32_t>(iMin);
        }

        // The real-valued roots (if any) are in the interval
        // (-bound,+bound).
        static void GetRootBound(Polynomial1<BSN> const& p, BSN& bound)
        {
            // Compute a number larger than Cauchy bound for f(x). The
            // increase ensures that the sets p((-infinity,-bound]) and
            // p([+bound,+infinity)) do not contain 0.
            size_t const degree = p.GetDegree();
            int32_t const minExponent = p[degree].GetExponent();
            int32_t maxExponent = p[0].GetExponent();
            for (size_t i = 1; i < degree; ++i)
            {
                int32_t exponent = p[i].GetExponent();
                if (exponent > maxExponent)
                {
                    maxExponent = exponent;
                }
            }
            BSN const one(1);
            bound = one + std::ldexp(one, maxExponent + 1 - minExponent);
        }

        void Find(Polynomial1<BSN> const& p, BSN const& xMin, BSN const& xMax,
            std::vector<BSN>& pRoots, bool wantRootIntervals)
        {
            RootsBisection1<BSN> bisector(mMaxBisections, mPrecision);
            BSN root, polyAtRoot;

            size_t const pDegree = p.GetDegree();
            size_t order = pDegree - 1;
            Polynomial1<BSN> qder{ p[order], GetModulate(order, 1) * p[pDegree] };
            auto qderEvaluator = [&qder](BSN const& x) { return qder(x); };
            std::vector<BSN> dRoots;
            dRoots.reserve(pDegree);
            if (bisector(qderEvaluator, xMin, xMax, root, polyAtRoot))
            {
                dRoots.push_back(root);
            }

            if (pDegree > 1)
            {
                Polynomial1<BSN> q(pDegree);  // Allocates storage once.
                auto qEvaluator = [&q](BSN const& x) { return q(x); };
                std::vector<BSN> roots;
                roots.reserve(pDegree);
                --order;
                for (size_t i = 1, qDegree = 2; i < pDegree; ++i, ++qDegree, --order)
                {
                    q.SetDegree(qDegree);
                    q[0] = p[order];
                    for (size_t power = 1, j = order + 1; power <= qDegree; ++power, ++j)
                    {
                        BSN const& modulate = GetModulate(order, power);
                        q[power] = modulate * p[j];
                    }

                    // The parity and sign are used to determine the signs at
                    // the interval endpoints:
                    //   sign(q(xMin)) = sign(q(-infinity))
                    //   sign(q(xMax)) = sign(q(+infinity))
                    // The parity is -1 when degree(q) is odd or +1 when
                    // degree(q) is even. The sign is that of the coefficient
                    // of the highest-order term of q.
                    int32_t parity = 1 - 2 * (static_cast<int32_t>(qDegree & 1));
                    int32_t sign = q[qDegree].GetSign();

                    roots.clear();
                    if (dRoots.size() > 0)
                    {
                        // Find root on [xMin, dRoots[0]].
                        BSN qMin = sign * parity;
                        BSN qMax = q(dRoots[0]);
                        if (bisector(qEvaluator, xMin, dRoots[0],
                            qMin, qMax, root, polyAtRoot))
                        {
                            roots.push_back(root);
                            if (wantRootIntervals && qDegree == pDegree)
                            {
                                mRootIntervals.push_back(
                                    { bisector.GetFinalTMin(), bisector.GetFinalTMax() });
                            }
                        }

                        // Find root on [dRoots[i], dRoots[i+1]].
                        for (size_t j = 0; j + 2 <= dRoots.size(); ++j)
                        {
                            qMin = std::move(qMax);
                            qMax = q(dRoots[j + 1]);
                            if (bisector(qEvaluator, dRoots[j], dRoots[j + 1],
                                qMin, qMax, root, polyAtRoot))
                            {
                                roots.push_back(root);
                                if (wantRootIntervals && qDegree == pDegree)
                                {
                                    mRootIntervals.push_back(
                                        { bisector.GetFinalTMin(), bisector.GetFinalTMax() });
                                }
                            }
                        }

                        // Find root on [dpRoots[numDRoots-1], xMax].
                        qMin = std::move(qMax);
                        qMax = sign;
                        if (bisector(qEvaluator, dRoots[dRoots.size() - 1], xMax,
                            qMin, qMax, root, polyAtRoot))
                        {
                            roots.push_back(root);
                            if (wantRootIntervals && qDegree == pDegree)
                            {
                                mRootIntervals.push_back(
                                    { bisector.GetFinalTMin(), bisector.GetFinalTMax() });
                            }
                        }
                    }
                    else
                    {
                        // The polynomial is monotone on [xMin, xMax], so it
                        // has at most one root.
                        BSN qMin = sign * parity;
                        BSN qMax = sign;
                        if (bisector(qEvaluator, xMin, xMax,
                            qMin, qMax, root, polyAtRoot))
                        {
                            roots.push_back(root);
                            if (wantRootIntervals && qDegree == pDegree)
                            {
                                mRootIntervals.push_back(
                                    { bisector.GetFinalTMin(), bisector.GetFinalTMax() });
                            }
                        }
                    }

                    dRoots.resize(roots.size());
                    std::copy(roots.begin(), roots.end(), dRoots.begin());
                }
            }
            else
            {
                if (wantRootIntervals)
                {
                    mRootIntervals.push_back(
                        { bisector.GetFinalTMin(), bisector.GetFinalTMax() });
                }
            }

            size_t j = pRoots.size();
            pRoots.resize(pRoots.size() + dRoots.size());
            for (size_t i = 0; i < dRoots.size(); ++i, ++j)
            {
                pRoots[j] = std::move(dRoots[i]);
            }
        }

        void SortRoots(std::vector<BSN>& roots, bool wantRootIntervals)
        {
            if (roots.size() > 1)
            {
                struct RootEx
                {
                    RootEx()
                        :
                        root{},
                        index(0)
                    {
                    }

                    BSN root;
                    size_t index;

                    bool operator<(RootEx const& other) const
                    {
                        return root < other.root;
                    }
                };

                std::vector<RootEx> rootEx(roots.size());
                for (size_t i = 0; i < roots.size(); ++i)
                {
                    rootEx[i].root = std::move(roots[i]);
                    rootEx[i].index = i;
                }

                std::sort(rootEx.begin(), rootEx.end());
                for (size_t i = 0; i < roots.size(); ++i)
                {
                    roots[i] = std::move(rootEx[i].root);
                }

                if (wantRootIntervals)
                {
                    std::vector<std::array<BSN, 2>> temp(roots.size());
                    for (size_t i = 0; i < roots.size(); ++i)
                    {
                        temp[i] = std::move(mRootIntervals[rootEx[i].index]);
                    }
                    mRootIntervals = std::move(temp);
                }
            }
        }

        void ComputePascalsTriangle()
        {
            // Compute the triangle of C(n,k) for n <= initialMaxDegree. If
            // an operator()(...) call is made with a polynomial of degree
            // larger than initialMaxDegree, additional rows of the triangle
            // are computed at that time.
            static size_t constexpr initialMaxDegree = 16;

            // The maximum value in a row of Pascal's triangle is computed
            // as follows. Let C(n,k) be the combinatorial "n choose k".
            //
            // Let n = 2*m be even; then C(2*0,0) = 1 and
            //   C(2*m,m) = ((4m-2)/m)*C(2*(m-1),m-1) for m >= 1
            // Let n = 2*m+1 be odd; then C(2*0+1,0) = 1 and
            //   C(2*m+1,m) = ((4*m+2)/(m+1))*C(2*(m-1)+1,m-1) for m >= 1

            // C(n,k) = mPascal[n * (n + 1) / 2 + k]
            mMaxDegree = initialMaxDegree;
            mPascal.resize((mMaxDegree + 1) * (mMaxDegree + 2) / 2);

            SetPascal(0, 0, 1);
            for (size_t n = 1; n <= mMaxDegree; ++n)
            {
                SetPascal(n, 0, 1);
                for (size_t k = 1; k < n; ++k)
                {
                    SetPascal(n, k, GetPascal(n - 1, k - 1) + GetPascal(n - 1, k));
                }
                SetPascal(n, n, 1);
            }
        }

        void AddRowsToPascalsTriangle(size_t degree)
        {
            if (degree > mMaxDegree)
            {
                mPascal.resize((degree + 1) * (degree + 2) / 2);
                for (size_t n = mMaxDegree + 1; n <= degree; ++n)
                {
                    SetPascal(n, 0, 1);
                    for (size_t k = 1; k < n; ++k)
                    {
                        SetPascal(n, k, GetPascal(n - 1, k - 1) + GetPascal(n - 1, k));
                    }
                    SetPascal(n, n, 1);
                }
                mMaxDegree = degree;
            }
        }

        inline void SetPascal(size_t n, size_t k, BSN const& value)
        {
            mPascal[n * (n + 1) / 2 + k] = value;
        }

        inline BSN const& GetPascal(size_t n, size_t k) const
        {
            return mPascal[n * (n + 1) / 2 + k];
        }

        inline BSN const& GetModulate(size_t order, size_t power)
        {
            return GetPascal(order + power, power);
        }

        size_t mMaxBisections, mPrecision;
        size_t mMaxDegree;
        std::vector<BSN> mPascal;
        int32_t mRootStatus;
        std::vector<std::array<BSN, 2>> mRootIntervals;

        friend class UnitTestRootsPolynomial;
    };
}
