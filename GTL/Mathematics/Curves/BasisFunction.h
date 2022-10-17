// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// Let n be the number of control points. Let d be the degree, where
// 1 <= d <= n-1. The number of knots is k = n + d + 1. The knots are t[i] for
// 0 <= i < k and must be nondecreasing, t[i] <= t[i+1], but a knot value can
// be repeated. Let s be the number of distinct knots.  Let the distinct knots
// be u[j] for 0 <= j < s, so u[j] < u[j+1] for all j. The set of u[j] is
// called a 'breakpoint sequence'. Let m[j] >= 1 be the multiplicity; that is,
// if t[i] is the first occurrence of u[j], then t[i+r] = t[i] for
// 1 <= r < m[j]. The multiplicities have the constraints m[0] <= d+1,
// m[s-1] <= d+1 and m[j] <= d for 1 <= j <= s-2.  Also,
// k = sum_{j=0}^{s-1} m[j], which says the multiplicities account for all
// k knots.
//
// Given a knot vector (t[0],...,t[n+d]), the domain of the corresponding
// B-spline curve is the interval [t[d],t[n]].
//
// The corresponding B-spline or NURBS curve is characterized as follows. See
//   Geometric Modeling with Splines: An Introduction,
//   Elaine Cohen, Richard F. Riesenfeld and Gershon Elber,
//   AK Peters, 2001, Natick MA.
// The curve is 'open' when m[0] = m[s-1] = d+1; otherwise, it is 'floating'.
// An open curve is uniform when the knots t[d] through t[n] are equally
// spaced; that is, t[i+1] - t[i] are a common value for d <= i <= n-1. By
// implication, s = n-d+1 and m[j] = 1 for 1 <= j <= s-2. An open curve that
// does not satisfy these conditions is said to be nonuniform. A floating
// curve is uniform when m[j] = 1 for 0 <= j <= s-1 and t[i+1] - t[i] are a
// common value for 0 <= i <= k-2; otherwise, the floating curve is
// nonuniform.
//
// A special case of a floating curve is a periodic curve. The intent is that
// the curve is closed, so the first and last control points should be the
// same, which ensures C^{0} continuity. Higher-order continuity is obtained
// by repeating more control points. If the control points are P[0] through
// P[n-1], append the points P[0] through P[d-1] to ensure C^{d-1} continuity.
// Additionally, the knots must be chosen properly. You may choose t[d]
// through t[n] as you wish. The other knots are defined by
//   t[i] - t[i-1] = t[n-d+i] - t[n-d+i-1]
//   t[n+i] - t[n+i-1] = t[d+i] - t[d+i-1]
// for 1 <= i <= d.

#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <GTL/Utility/Multiarray.h>
#include <cmath>
#include <cstring>

namespace gtl
{
    template <typename T>
    class BasisFunction
    {
    public:
        struct UniqueKnot
        {
            UniqueKnot()
                :
                t(C_<T>(0)),
                multiplicity(0)
            {
            }

            UniqueKnot(T const& inT, size_t inMultiplicity)
                :
                t(inT),
                multiplicity(inMultiplicity)
            {
            }

            T t;
            size_t multiplicity;
        };

        class Input
        {
        public:
            Input()
                :
                numControls(0),
                degree(0),
                uniform(false),
                periodic(false),
                uniqueKnots{}
            {
            }

            // Construct an open uniform curve with t in [0,1].
            Input(size_t inNumControls, size_t inDegree)
                :
                numControls(inNumControls),
                degree(inDegree),
                uniform(true),
                periodic(false),
                uniqueKnots{}
            {
                // Verify numUniqueControls >= 2.
                GTL_ARGUMENT_ASSERT(
                    numControls >= degree + 1,
                    "Invalid number of controls points or degree.");

                size_t const numUniqueKnots = numControls - degree + 1;
                T const denom = static_cast<T>(numUniqueKnots) - C_<T>(1);
                uniqueKnots.resize(numUniqueKnots);
                uniqueKnots.front().t = C_<T>(0);
                uniqueKnots.front().multiplicity = degree + 1;
                for (size_t i = 1; i <= numUniqueKnots - 2; ++i)
                {
                    uniqueKnots[i].t = static_cast<T>(i) / denom;
                    uniqueKnots[i].multiplicity = 1;
                }
                uniqueKnots.back().t = C_<T>(1);
                uniqueKnots.back().multiplicity = degree + 1;
            }

            size_t numControls;
            size_t degree;
            bool uniform;
            bool periodic;
            std::vector<UniqueKnot> uniqueKnots;
        };

        // Construction and destruction. The determination that the curve is
        // open or floating is based on the multiplicities. The 'uniform'
        // input is used to avoid misclassifications due to floating-point
        // rounding errors. Specifically, the breakpoints might be equally
        // spaced (uniform) as real numbers, but the floating-point
        // representations can have rounding errors that cause the knot
        // differences not to be exactly the same constant. A periodic curve
        // can have uniform or nonuniform knots. This object makes copies of
        // the input arrays.
        BasisFunction()
            :
            mNumControls(0),
            mDegree(0),
            mTMin(C_<T>(0)),
            mTMax(C_<T>(0)),
            mTLength(C_<T>(0)),
            mOpen(false),
            mUniform(false),
            mPeriodic(false)
        {
        }

        BasisFunction(Input const& input)
            :
            mNumControls(0),
            mDegree(0),
            mTMin(C_<T>(0)),
            mTMax(C_<T>(0)),
            mTLength(C_<T>(0)),
            mOpen(false),
            mUniform(false),
            mPeriodic(false)
        {
            Create(input);
        }


        ~BasisFunction() = default;

        // Support for explicit creation in classes that have std::array
        // members involving BasisFunction. This is a call-once function.
        void Create(Input const& input)
        {
            GTL_ARGUMENT_ASSERT(
                mNumControls == 0 && mDegree == 0,
                "The object is already created.");

            GTL_ARGUMENT_ASSERT(
                input.numControls >= 2,
                "Invalid number of control points.");

            GTL_ARGUMENT_ASSERT(
                1 <= input.degree && input.degree < input.numControls,
                "Invalid degree.");

            GTL_ARGUMENT_ASSERT(
                input.uniqueKnots.size() >= 2,
                "Invalid number of unique knots.");

            mNumControls = (input.periodic ? input.numControls + input.degree : input.numControls);
            mDegree = input.degree;
            mUniform = input.uniform;
            mPeriodic = input.periodic;

            size_t const numUniqueKnots = input.uniqueKnots.size();
            mUniqueKnots.resize(numUniqueKnots);
            std::copy(input.uniqueKnots.begin(),
                input.uniqueKnots.begin() + numUniqueKnots,
                mUniqueKnots.begin());

            T u = mUniqueKnots.front().t;
            for (size_t i = 1; i < numUniqueKnots - 1; ++i)
            {
                T uNext = mUniqueKnots[i].t;
                GTL_RUNTIME_ASSERT(
                    u < uNext,
                    "Unique knots are not strictly increasing.");

                u = uNext;
            }

            size_t mult0 = mUniqueKnots.front().multiplicity;
            GTL_RUNTIME_ASSERT(
                mult0 >= 1 && mult0 <= mDegree + 1,
                "Invalid first multiplicity.");

            size_t mult1 = mUniqueKnots.back().multiplicity;
            GTL_RUNTIME_ASSERT(
                mult1 >= 1 && mult1 <= mDegree + 1,
                "Invalid last multiplicity.");

            for (size_t i = 1; i <= numUniqueKnots - 2; ++i)
            {
                size_t mult = mUniqueKnots[i].multiplicity;
                GTL_RUNTIME_ASSERT(
                    mult >= 1 && mult <= mDegree + 1,
                    "Invalid interior multiplicity.");
            }

            mOpen = (mult0 == mult1 && mult0 == mDegree + 1);

            mKnots.resize(mNumControls + mDegree + 1);
            mKeys.resize(numUniqueKnots);
            size_t sum = 0;
            for (size_t i = 0, j = 0; i < numUniqueKnots; ++i)
            {
                T tCommon = mUniqueKnots[i].t;
                size_t mult = mUniqueKnots[i].multiplicity;
                for (size_t k = 0; k < mult; ++k, ++j)
                {
                    mKnots[j] = tCommon;
                }

                mKeys[i].first = tCommon;
                mKeys[i].second = sum - 1;
                sum += mult;
            }

            mTMin = mKnots[mDegree];
            mTMax = mKnots[mNumControls];
            mTLength = mTMax - mTMin;

            size_t numRows = mDegree + 1;
            size_t numCols = mNumControls + mDegree;
            for (size_t i = 0; i < 4; ++i)
            {
                mJet[i] = Multiarray<T, true>{ numCols, numRows };
                mJet[i].fill(C_<T>(0));
            }
        }

        // Member access.
        inline size_t GetNumControls() const
        {
            return mNumControls;
        }

        inline size_t GetDegree() const
        {
            return mDegree;
        }

        inline size_t GetNumUniqueKnots() const
        {
            return mUniqueKnots.size();
        }

        inline std::vector<UniqueKnot> const& GetUniqueKnots() const
        {
            return mUniqueKnots;
        }

        inline size_t GetNumKnots() const
        {
            return mKnots.size();
        }

        inline std::vector<T> const& GetKnots() const
        {
            return mKnots;
        }

        inline T const& GetMinDomain() const
        {
            return mTMin;
        }

        inline T const& GetMaxDomain() const
        {
            return mTMax;
        }

        inline bool IsOpen() const
        {
            return mOpen;
        }

        inline bool IsUniform() const
        {
            return mUniform;
        }

        inline bool IsPeriodic() const
        {
            return mPeriodic;
        }

        // Evaluation of the basis function and its derivatives through 
        // order 3. For the function value only, pass order 0. For the
        // function and first derivative, pass order 1, and so on.
        void Evaluate(T t, size_t order, size_t& minIndex, size_t& maxIndex) const
        {
            GTL_ARGUMENT_ASSERT(
                order <= 3,
                "Invalid order.");

            size_t i = GetIndex(t);
            mJet[0](i, 0) = C_<T>(1);

            if (order >= 1)
            {
                mJet[1](i, 0) = C_<T>(0);
                if (order >= 2)
                {
                    mJet[2](i, 0) = C_<T>(0);
                    if (order >= 3)
                    {
                        mJet[3](i, 0) = C_<T>(0);
                    }
                }
            }

            T n0 = t - mKnots[i], n1 = mKnots[i + 1] - t;
            T e0, e1, d0, d1, invD0, invD1;
            for (size_t j = 1; j <= mDegree; j++)
            {
                d0 = mKnots[i + j] - mKnots[i];
                d1 = mKnots[i + 1] - mKnots[i - j + 1];
                invD0 = (d0 > C_<T>(0) ? C_<T>(1) / d0 : C_<T>(0));
                invD1 = (d1 > C_<T>(0) ? C_<T>(1) / d1 : C_<T>(0));

                e0 = n0 * mJet[0](i, j - 1);
                mJet[0](i, j) = e0 * invD0;
                e1 = n1 * mJet[0](i - j + 1, j - 1);
                mJet[0](i - j, j) = e1 * invD1;

                if (order >= 1)
                {
                    e0 = n0 * mJet[1](i, j - 1) + mJet[0](i, j - 1);
                    mJet[1](i, j) = e0 * invD0;
                    e1 = n1 * mJet[1](i - j + 1, j - 1) - mJet[0](i - j + 1, j - 1);
                    mJet[1](i - j, j) = e1 * invD1;

                    if (order >= 2)
                    {
                        e0 = n0 * mJet[2](i, j - 1) + C_<T>(2) * mJet[1](i, j - 1);
                        mJet[2](i, j) = e0 * invD0;
                        e1 = n1 * mJet[2](i - j + 1, j - 1) - C_<T>(2) * mJet[1](i - j + 1, j - 1);
                        mJet[2](i - j, j) = e1 * invD1;

                        if (order >= 3)
                        {
                            e0 = n0 * mJet[3](i, j - 1) + C_<T>(3) * mJet[2](i, j - 1);
                            mJet[3](i, j) = e0 * invD0;
                            e1 = n1 * mJet[3](i - j + 1, j - 1) - C_<T>(3) * mJet[2](i - j + 1, j - 1);
                            mJet[3](i - j, j) = e1 * invD1;
                        }
                    }
                }
            }

            for (size_t j = 2; j <= mDegree; ++j)
            {
                for (size_t k = i - j + 1; k < i; ++k)
                {
                    n0 = t - mKnots[k];
                    n1 = mKnots[k + j + 1] - t;
                    d0 = mKnots[k + j] - mKnots[k];
                    d1 = mKnots[k + j + 1] - mKnots[k + 1];
                    invD0 = (d0 > C_<T>(0) ? C_<T>(1) / d0 : C_<T>(0));
                    invD1 = (d1 > C_<T>(0) ? C_<T>(1) / d1 : C_<T>(0));

                    e0 = n0 * mJet[0](k, j - 1);
                    e1 = n1 * mJet[0](k + 1, j - 1);
                    mJet[0](k, j) = e0 * invD0 + e1 * invD1;

                    if (order >= 1)
                    {
                        e0 = n0 * mJet[1](k, j - 1) + mJet[0](k, j - 1);
                        e1 = n1 * mJet[1](k + 1, j - 1) - mJet[0](k + 1, j - 1);
                        mJet[1](k, j) = e0 * invD0 + e1 * invD1;

                        if (order >= 2)
                        {
                            e0 = n0 * mJet[2](k, j - 1) + C_<T>(2) * mJet[1](k, j - 1);
                            e1 = n1 * mJet[2](k + 1, j - 1) - C_<T>(2) * mJet[1](k + 1, j - 1);
                            mJet[2](k, j) = e0 * invD0 + e1 * invD1;

                            if (order >= 3)
                            {
                                e0 = n0 * mJet[3](k, j - 1) + C_<T>(3) * mJet[2](k, j - 1);
                                e1 = n1 * mJet[3](k + 1, j - 1) - C_<T>(3) * mJet[2](k + 1, j - 1);
                                mJet[3](k, j) = e0 * invD0 + e1 * invD1;
                            }
                        }
                    }
                }
            }

            minIndex = i - mDegree;
            maxIndex = i;
        }

        // Access the results of the call to Evaluate(...).  The index i must
        // satisfy minIndex <= i <= maxIndex.  If it is not, the function
        // returns zero.  The separation of evaluation and access is based on
        // local control of the basis function; that is, only the accessible
        // values are (potentially) not zero.
        T const& GetValue(size_t order, size_t i) const
        {
            GTL_ARGUMENT_ASSERT(
                order < 4 && i < mNumControls + mDegree,
                "Invalid order or index.");

            return mJet[order](i, mDegree);
        }

    private:
        // Determine the index i for which knot[i] <= t < knot[i+1].  The
        // t-value is modified (wrapped for periodic splines, clamped for
        // nonperiodic splines).
        size_t GetIndex(T& t) const
        {
            // Find the index i for which knot[i] <= t < knot[i+1].
            if (mPeriodic)
            {
                // Wrap to [tmin,tmax].
                T r = std::fmod(t - mTMin, mTLength);
                if (r < C_<T>(0))
                {
                    r += mTLength;
                }
                t = mTMin + r;
            }

            // Clamp to [tmin,tmax]. For the periodic case, this handles
            // small numerical rounding errors near the domain endpoints.
            if (t <= mTMin)
            {
                t = mTMin;
                return mDegree;
            }
            if (t >= mTMax)
            {
                t = mTMax;
                return mNumControls - 1;
            }

            // At this point, tmin < t < tmax.
            for (auto const& key : mKeys)
            {
                if (t < key.first)
                {
                    return key.second;
                }
            }

            GTL_RUNTIME_ERROR(
                "Unexpected condition. This code should not be reached.");
        }

        // Constructor inputs and values derived from them.
        size_t mNumControls;
        size_t mDegree;
        T mTMin, mTMax, mTLength;
        bool mOpen;
        bool mUniform;
        bool mPeriodic;
        std::vector<UniqueKnot> mUniqueKnots;
        std::vector<T> mKnots;

        // Lookup information for the GetIndex() function. The first element
        // of the pair is a unique knot value. The second element is the
        // index in mKnots[] for the last occurrence of that knot value.
        std::vector<std::pair<T, size_t>> mKeys;

        // Storage for the basis functions and their first three derivatives;
        // mJet[i] is array[d+1][n+d].
        mutable std::array<Multiarray<T, true>, 4> mJet;
    };
}
