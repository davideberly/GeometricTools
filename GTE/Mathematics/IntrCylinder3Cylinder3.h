// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Test for intersection of two finite cylinders using the method of
// separating axes. The algorithm is described in the document
// https://www.geometrictools.com/Documentation/IntersectionOfCylinders.pdf

#include <Mathematics/TIQuery.h>
#include <Mathematics/Constants.h>
#include <Mathematics/Functions.h>
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/RootsBisection.h>
#include <Mathematics/Minimize1.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

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
                separated(false),
                separatingDirection{}
            {
            }

            bool separated;
            Vector3<T> separatingDirection;
        };

        TIQuery(size_t numLines)
            :
            mNumLines(numLines),
            mW0{},
            mR0(static_cast<T>(0)),
            mHalfH0(static_cast<T>(0)),
            mW1{},
            mR1(static_cast<T>(0)),
            mHalfH1(static_cast<T>(0)),
            mDelta{},
            mLengthDelta(static_cast<T>(0)),
            mW0xW1{},
            mBasis{},
            mLineOrigin{},
            mLineDirection{},
            mQ0{},
            mQ1{},
            mDiffQ0P{},
            mDiffQ1P{},
            mDGDT([this](T const& t) { return DGDT(t); })
        {
        }

        Result operator()(Cylinder3<T> const& cylinder0, Cylinder3<T> const& cylinder1)
        {
            // The constructor sets result.separated to false and
            // result.separatingDirection to (0,0,0).
            Result result{};

            T const zero = static_cast<T>(0);
            mDelta = cylinder1.axis.origin - cylinder0.axis.origin;
            mLengthDelta = Length(mDelta);
            if (mLengthDelta == zero)
            {
                return result;
            }

            T const half = static_cast<T>(0.5);
            mW0 = cylinder0.axis.direction;
            mR0 = cylinder0.radius;
            mHalfH0 = half * cylinder0.height;
            mW1 = cylinder1.axis.direction;
            mR1 = cylinder1.radius;
            mHalfH1 = half * cylinder1.height;
            mW0xW1 = Cross(mW0, mW1);
            T lengthW0xW1 = Length(mW0xW1), test = zero;
            if (lengthW0xW1 > zero)
            {
                // Test for separation by W0.
                T absDotW0W1 = std::fabs(Dot(mW0, mW1));
                T absDotW0Delta = std::fabs(Dot(mW0, mDelta));
                test = mR1 * lengthW0xW1 + mHalfH0 + mHalfH1 * absDotW0W1 - absDotW0Delta;
                if (test < zero)
                {
                    result.separated = true;
                    result.separatingDirection = mW0;
                    return result;
                }

                // Test for separation by W1.
                T absDotW1Delta = std::fabs(Dot(mW1, mDelta));
                test = mR0 * lengthW0xW1 + mHalfH0 * absDotW0W1 + mHalfH1 - absDotW1Delta;
                if (test < zero)
                {
                    result.separated = true;
                    result.separatingDirection = mW1;
                    return result;
                }

                // Test for separation by W0xW1.
                T absDotW0xW1Delta = std::fabs(Dot(mW0xW1, mDelta));
                test = (mR0 + mR1) * lengthW0xW1 - absDotW0xW1Delta;
                if (test < zero)
                {
                    result.separated = true;
                    result.separatingDirection = mW0xW1;
                    Normalize(result.separatingDirection);
                    return result;
                }

                // Test for separation by Delta.
                test = mR0 * Length(Cross(mDelta, mW0)) + mR1 * Length(Cross(mDelta, mW1)) +
                    mHalfH0 * absDotW0Delta + mHalfH1 * absDotW1Delta - Dot(mDelta, mDelta);
                if (test < zero)
                {
                    result.separated = true;
                    result.separatingDirection = mDelta;
                    Normalize(result.separatingDirection);
                    return result;
                }

                // Test for separation by other directions. This function
                // implements the minimum search described in the PDF.
                if (SeparatedByOtherDirections(result.separatingDirection))
                {
                    result.separated = true;
                    return result;
                }
            }
            else
            {
                // Test for separation by height.
                T dotDeltaW0 = Dot(mDelta, mW0);
                test = mHalfH0 + mHalfH1 - std::fabs(dotDeltaW0);
                if (test < zero)
                {
                    result.separated = true;
                    result.separatingDirection = mW0;
                    return result;
                }

                // Test for separation radially.
                test = mR0 + mR1 - Length(Cross(mDelta, mW0));
                if (test < zero)
                {
                    result.separated = true;
                    result.separatingDirection = mDelta - dotDeltaW0 * mW0;
                    Normalize(result.separatingDirection);
                    return result;
                }
            }

            return result;
        }

    private:
        // Maximum number of iterations to locate an endpoint of a
        // root-bounding interval for g'(t).
        size_t const imax = static_cast<size_t>(
            std::numeric_limits<T>::max_exponent);

        // Maximum number of bisections for the bisector that locates
        // roots of g'(t).
        uint32_t const maxBisections = static_cast<uint32_t>(
            std::numeric_limits<T>::max_exponent -
            std::numeric_limits<T>::min_exponent +
            std::numeric_limits<T>::digits);

        bool SeparatedByOtherDirections(Vector3<T>& separatingDirection)
        {
            // Convert to the coordinate system where N = Delta/|Delta| is the
            // north pole of the hemisphere to be searched. Using the notation
            // of the PDF, N is mBasis[1], U is mBasis[0] and V is mBasis[1].
            // The ComputeOrthonormalBasis function will normalize mBasis[2]
            // before computing orthogonal vectors mBasis[0] and mBasis[1].
            std::array<Vector3<T>, 3> tempBasis{};  // { N, U, V }
            tempBasis[0] = mDelta;
            ComputeOrthogonalComplement(1, tempBasis.data());
            mBasis[0] = std::move(tempBasis[1]);  // { U, V, N }
            mBasis[1] = std::move(tempBasis[2]);
            mBasis[2] = std::move(tempBasis[0]);
            mW0 = { Dot(mBasis[0], mW0), Dot(mBasis[1], mW0), Dot(mBasis[2], mW0) };
            mW1 = { Dot(mBasis[0], mW1), Dot(mBasis[1], mW1), Dot(mBasis[2], mW1) };
            mW0xW1 = { Dot(mBasis[0], mW0xW1), Dot(mBasis[1], mW0xW1), Dot(mBasis[2], mW0xW1) };

            // The axis directions and their cross product must be in the
            // hemisphere with north pole N.
            T const zero = static_cast<T>(0);
            if (mW0[2] < zero)
            {
                mW0 = -mW0;
            }
            if (mW1[2] < zero)
            {
                mW1 = -mW1;
            }
            if (mW0xW1[2] < zero)
            {
                mW0xW1 = -mW0xW1;
            }

            // Compute the common origin for the line discontinuities.
            T const one = static_cast<T>(1);
            mLineOrigin = { mW0xW1[0] / mW0xW1[2], mW0xW1[1] / mW0xW1[2], one };

            // Compute the point discontinuities.
            mQ0 = { mW0[0] / mW0[2], mW0[1] / mW0[2], one };
            mQ1 = { mW1[0] / mW1[2], mW1[1] / mW1[2], one };
            mDiffQ0P = mQ0 - mLineOrigin;
            mDiffQ1P = mQ1 - mLineOrigin;

            // Search the lines with common origin for a separating direction.
            std::vector<T> lineMinimum(mNumLines + 1);
            T multiplier = static_cast<T>(GTE_C_PI) / static_cast<T>(mNumLines);
            T tMin = zero, gMin = zero;
            for (size_t i = 0; i < mNumLines; ++i)
            {
                // The line direction is (cos(angle), sin(angle), 0).
                T angle = multiplier * static_cast<T>(i);

                // Compute the minimum of g(t) on the line P + t * L(angle).
                mLineDirection = { std::cos(angle), std::sin(angle), zero };
                ComputeLineMinimum(mLineDirection, tMin, gMin);
                lineMinimum[i] = gMin;

                // Exit early when a line minimum is negative.
                if (gMin < zero)
                {
                    // Transform to the original coordinate system.
                    Vector3<T> polePoint = mLineOrigin + tMin * mLineDirection;
                    separatingDirection = polePoint[0] * mBasis[0] + polePoint[1] * mBasis[1] + mBasis[2];
                    Normalize(separatingDirection);
                    return true;
                }
            }
            lineMinimum[mNumLines] = lineMinimum[0];

            // The mNumLines samples did not produce a negative minimum. Use a
            // derivativeless minimizer to refine the search.

            // The function to minimize. The input is the angle of the line
            // and the output is the minimum of g(t) along that line.
            auto lineFunction = [this, &tMin, &zero](T const& angle)
            {
                mLineDirection = { std::cos(angle), std::sin(angle), zero };
                T gMin = zero;
                ComputeLineMinimum(mLineDirection, tMin, gMin);
                return gMin;
            };

            Minimize1<T> minimizer(lineFunction, 1, maxBisections, zero, zero);

            // Locate a triple of angles that bracket the minimum.
            std::array<size_t, 3> bracket = { mNumLines - 1, 0, 1 };
            for (size_t i0 = 0, i1 = 1, i2 = 2; i2 <= mNumLines; i0 = i1, i1 = i2++)
            {
                if (lineMinimum[i1] < lineMinimum[bracket[1]])
                {
                    bracket = { i0, i1, i2 };
                }
            }

            T angle0 = multiplier * static_cast<T>(bracket[0]);
            T angle1 = multiplier * static_cast<T>(bracket[1]);
            T angle2 = multiplier * static_cast<T>(bracket[2]);

            // If bracket = { mNumLines - 1, 0, 1 }, then angle0 > angle1.
            // The minimizer expects angle0 < angle1, so subtract pi from
            // angle0 to generate an equivalent angle.
            if (bracket[1] == 0)
            {
                angle0 -= static_cast<T>(GTE_C_PI);
            }

            T angleMin = zero;
            minimizer.GetMinimum(angle0, angle2, angle1, angleMin, gMin);
            if (gMin < zero)
            {
                // Reconstruct the tMin value associated with angleMin.
                lineFunction(angleMin);

                // Transform to the original coordinate system.
                Vector3<T> polePoint = mLineOrigin + tMin * mLineDirection;
                separatingDirection = polePoint[0] * mBasis[0] + polePoint[1] * mBasis[1] + mBasis[2];
                Normalize(separatingDirection);
                return true;
            }

            return false;
        }

        void ComputeLineMinimum(Vector3<T> const& L, T& tMin, T& gMin) const
        {
            T const zero = static_cast<T>(0);
            Vector3<T> LPerp{ L[1], -L[0] , zero };
            T dgdt0n = zero, dgdt0p = zero, dgdt1n = zero, dgdt1p = zero;
            if (Dot(LPerp, mDiffQ0P) != zero)
            {
                // Q0 is not on the line P + t * L(angle).
                if (Dot(LPerp, mDiffQ1P) != zero)
                {
                    // Q1 is not on the line P + t * L(angle).
                    LimitsDGDTZero(dgdt0n, dgdt0p);
                    ComputeMinimumSingularZero(dgdt0n, dgdt0p, tMin, gMin);
                }
                else
                {
                    // Q1 is not on the line P + t * L(angle).
                    LimitsDGDTZero(dgdt0n, dgdt0p);
                    LimitsDGDTOneQ1(dgdt1n, dgdt1p);
                    ComputeMinimumSingularZeroOne(dgdt0n, dgdt0p, dgdt1n, dgdt1p, tMin, gMin);
                }
            }
            else
            {
                // Q0 is on the line P + t * L(angle).
                LimitsDGDTZero(dgdt0n, dgdt0p);
                LimitsDGDTOneQ0(dgdt1n, dgdt1p);
                ComputeMinimumSingularZeroOne(dgdt0n, dgdt0p, dgdt1n, dgdt1p, tMin, gMin);
            }
        }

        // Compute one-sided limits along the line D(t) = P+t*L at t = 0.
        void LimitsDGDTZero(T& dgdt0n, T& dgdt0p) const
        {
            Vector3<T> crossPW0 = Cross(mLineOrigin, mW0);
            Vector3<T> crossPW1 = Cross(mLineOrigin, mW1);
            Vector3<T> crossLW0 = Cross(mLineDirection, mW0);
            Vector3<T> crossLW1 = Cross(mLineDirection, mW1);
            T dotLW0 = Dot(mLineDirection, mW0);
            T dotLW1 = Dot(mLineDirection, mW1);
            T r0Term = mR0 * Dot(crossPW0, crossLW0) / Length(crossPW0);
            T r1Term = mR1 * Dot(crossPW1, crossLW1) / Length(crossPW1);
            T sumRTerms = r0Term + r1Term;
            T prdH0Term = mHalfH0 * std::fabs(dotLW0);
            T prdH1Term = mHalfH1 * std::fabs(dotLW1);
            T sumHTerms = prdH0Term + prdH1Term;
            dgdt0n = sumRTerms - sumHTerms;
            dgdt0p = sumRTerms + sumHTerms;
        }

        // Compute one-sided limits along the line D(t) = (1-t)*P + t*(Q0-P)
        // at t = 1.
        void LimitsDGDTOneQ0(T& dgdt1n, T& dgdt1p) const
        {
            Vector3<T> crossQ0W1 = Cross(mQ0, mW1);
            Vector3<T> crossLW0 = Cross(mLineDirection, mW0);
            Vector3<T> crossLW1 = Cross(mLineDirection, mW1);
            T dotLW0 = Dot(mLineDirection, mW0);
            T dotLW1 = Dot(mLineDirection, mW1);
            T r0Term = mR0 * Length(crossLW0);
            T r1Term = mR1 * Dot(crossQ0W1, crossLW1) / Length(crossQ0W1);
            T prdH0Term = mHalfH0 * std::fabs(dotLW0);
            T prdH1Term = mHalfH1 * std::fabs(dotLW1);
            T sumHTerms = prdH0Term + prdH1Term;
            T sumRHTerms = r1Term + sumHTerms;
            dgdt1n = sumRHTerms - r0Term;
            dgdt1p = sumRHTerms + r0Term;
        }

        // Compute one-sided limits along the line D(t) = (1-t)*P + t*(Q1-P)
        // at t = 1.
        void LimitsDGDTOneQ1(T& dgdt1n, T& dgdt1p) const
        {
            Vector3<T> crossQ1W0 = Cross(mQ1, mW0);
            Vector3<T> crossLW0 = Cross(mLineDirection, mW0);
            Vector3<T> crossLW1 = Cross(mLineDirection, mW1);
            T dotLW0 = Dot(mLineDirection, mW0);
            T dotLW1 = Dot(mLineDirection, mW1);
            T r0Term = mR0 * Dot(crossQ1W0, crossLW0) / Length(crossQ1W0);
            T r1Term = mR1 * Length(crossLW1);
            T prdH0Term = mHalfH0 * std::fabs(dotLW0);
            T prdH1Term = mHalfH1 * std::fabs(dotLW1);
            T sumHTerms = prdH0Term + prdH1Term;
            T sumRHTerms = r0Term + sumHTerms;
            dgdt1n = sumRHTerms - r1Term;
            dgdt1p = sumRHTerms + r1Term;
        }

        void LimitsDGDTInfinity(T& dgdtInfinity)
        {
            Vector3<T> crossLW0 = Cross(mLineDirection, mW0);
            Vector3<T> crossLW1 = Cross(mLineDirection, mW1);
            T dotLW0 = Dot(mLineDirection, mW0);
            T dotLW1 = Dot(mLineDirection, mW1);
            dgdtInfinity = mR0 * Length(crossLW0) + mR1 * Length(crossLW1)
                + mHalfH0 * std::fabs(dotLW0) + mHalfH1 * std::fabs(dotLW1);
        }

        // Evaluation of g(t) = F(D(t)).
        T G(T const& t) const
        {
            Vector3<T> D = mLineOrigin + mLineDirection * t;

            Vector3<T> crossDW0 = Cross(D, mW0);
            Vector3<T> crossDW1 = Cross(D, mW1);
            T dotDW0 = Dot(D, mW0);
            T dotDW1 = Dot(D, mW1);

            T r0Term = mR0 * Length(crossDW0);
            T r1Term = mR1 * Length(crossDW1);
            T h0Term = mHalfH0 * std::fabs(dotDW0);
            T h1Term = mHalfH1 * std::fabs(dotDW1);

            T result = r0Term + r1Term + h0Term + h1Term - mLengthDelta;
            return result;
        };

        // Evaluation of g'(t) except at derivative singularities
        // t = 0 and t = 1.
        T DGDT(T const& t) const
        {
            Vector3<T> D = mLineOrigin + mLineDirection * t;

            Vector3<T> crossDW0 = Cross(D, mW0);
            Vector3<T> crossDW1 = Cross(D, mW1);
            Vector3<T> crossLW0 = Cross(mLineDirection, mW0);
            Vector3<T> crossLW1 = Cross(mLineDirection, mW1);
            T dotLW0 = Dot(mLineDirection, mW0);
            T dotLW1 = Dot(mLineDirection, mW1);

            T r0Term = mR0 * Dot(crossDW0, crossLW0) / Length(crossDW0);
            T r1Term = mR1 * Dot(crossDW1, crossLW1) / Length(crossDW1);
            T sgn = gte::sign(t);
            T h0Term = mHalfH0 * std::fabs(dotLW0) * sgn;
            T h1Term = mHalfH1 * std::fabs(dotLW1) * sgn;

            T result = r0Term + r1Term + h0Term + h1Term;
            return result;
        }

        void ComputeMinimumSingularZero(T const& dgdt0n, T const& dgdt0p,
            T& tMin, T& gMin) const
        {
            // Determine the interval that contains the unique root of g'(t)
            // by analyzing the one-sided limits g'(0^-) and g'(0^+).
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);
            T const negOne = static_cast<T>(-1);

            if (dgdt0n > zero)
            {
                // The root of g'(t) occurs on (-infinity,0).
                T t0 = negOne, dgdtT0 = zero;
                for (size_t i = 0; i < imax; ++i)
                {
                    dgdtT0 = DGDT(t0);
                    if (dgdtT0 < zero)
                    {
                        break;
                    }
                    t0 *= two;
                }

                (void)RootsBisection<T>::Find(mDGDT, t0, zero, dgdtT0, dgdt0n,
                    maxBisections, tMin);
            }
            else if (dgdt0p < zero)
            {
                // The root of g'(t) occurs on (0,+infinity).
                T t1 = one, dgdtT1 = zero;
                for (size_t i = 0; i < imax; ++i)
                {
                    dgdtT1 = DGDT(t1);
                    if (dgdtT1 > zero)
                    {
                        break;
                    }
                    t1 *= two;
                }

                (void)RootsBisection<T>::Find(mDGDT, zero, t1, dgdt0p, dgdtT1,
                    maxBisections, tMin);
            }
            else
            {
                // At this time, g'(0^-) <= 0 <= g'(0+). The minimum of g(t)
                // occurs at g(0).
                tMin = zero;
            }

            gMin = G(tMin);
        }

        void ComputeMinimumSingularZeroOne(T const& dgdt0n, T const& dgdt0p,
            T const& dgdt1n, T const& dgdt1p, T& tMin, T& gMin) const
        {
            // Determine the interval that contains the unique root of g'(t)
            // by analyzing the one-sided limits g'(0^-), g'(0^+), g'(1^-)
            // and g'(1^+).
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const two = static_cast<T>(2);
            T const negOne = static_cast<T>(-1);

            if (dgdt0n > zero)
            {
                // The root of g'(t) occurs on (-infinity,0).
                T t0 = negOne, dgdtT0 = zero;
                for (size_t i = 0; i < imax; ++i)
                {
                    dgdtT0 = DGDT(t0);
                    if (dgdtT0 < zero)
                    {
                        break;
                    }
                    t0 *= two;
                }

                (void)RootsBisection<T>::Find(mDGDT, t0, zero, dgdtT0, dgdt0n,
                    maxBisections, tMin);
            }
            else if (dgdt0p < zero)
            {
                // The root of g'(t) occurs on (1,+infinity).
                T t1 = two, dgdtT1 = zero;
                for (size_t i = 0; i < imax; ++i)
                {
                    dgdtT1 = DGDT(t1);
                    if (dgdtT1 > zero)
                    {
                        break;
                    }
                    t1 *= two;
                }

                (void)RootsBisection<T>::Find(mDGDT, one, t1, dgdt1p, dgdtT1,
                    maxBisections, tMin);
            }
            else
            {
                // At this time, g'(0^-) <= 0 <= g'(1^+).
                if (dgdt0p < zero)
                {
                    if (dgdt1n > zero)
                    {
                        // The root of g'(t) occurs on (0,1).
                        (void)RootsBisection<T>::Find(mDGDT, zero, one, dgdt0p, dgdt1n,
                            maxBisections, tMin);
                    }
                    else
                    {
                        // The minimum of g(t) occurs at g(1).
                        tMin = one;
                    }
                }
                else
                {
                    // The minimum of g(t) occurs at g(0).
                    tMin = zero;
                }
            }

            gMin = G(tMin);
        }


        // The number of lines to search in the pole plane for the minimum.
        size_t mNumLines;

        // Cylinder 0.
        Vector3<T> mW0; // W0
        T mR0;          // r0
        T mHalfH0;      // h0/2

        // Cylinder 1.
        Vector3<T> mW1; // W1
        T mR1;          // r1
        T mHalfH1;      // h1/2

        // Members dependent on both cylinders.
        Vector3<T> mDelta;  // C1 - C0 (difference of centers)
        T mLengthDelta;     // Length(Delta)
        Vector3<T> mW0xW1;  // Cross(W0, W1);
        std::array<Vector3<T>, 3> mBasis;  // { U, V, N }

        // Members to support the line search for a minimum;
        Vector3<T> mLineOrigin, mLineDirection, mQ0, mQ1, mDiffQ0P, mDiffQ1P;
        std::function<T(T const&)> mDGDT;
    };
}
