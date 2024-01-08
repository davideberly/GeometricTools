// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/Matrix3x3.h>
#include <Mathematics/RootsBisection.h>
#include <Mathematics/RootsPolynomial.h>
#include <Mathematics/SymmetricEigensolver3x3.h>
#include <algorithm>
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
    class TIQuery<T, Ellipsoid3<T>, Ellipsoid3<T>>
    {
    public:
        enum class Classification
        {
            ELLIPSOIDS_SEPARATED,
            ELLIPSOIDS_INTERSECTING,
            ELLIPSOID0_CONTAINS_ELLIPSOID1,
            ELLIPSOID1_CONTAINS_ELLIPSOID0,
            INVALID
        };

        struct Result
        {
            Result()
                :
                intersect(false),
                classification(Classification::INVALID)
            {
            }

            // As solids, the ellipsoids intersect as long as they are not
            // separated.
            bool intersect;

            // This is one of the four enumerations listed above.
            Classification classification;
        };

        Result operator()(Ellipsoid3<T> const& ellipsoid0, Ellipsoid3<T> const& ellipsoid1)
        {
            Result result{};

            T const zero = (T)0;
            T const one = (T)1;

            // Get the parameters of ellipsoid0.
            Vector3<T> K0 = ellipsoid0.center;
            Matrix3x3<T> R0;
            R0.SetCol(0, ellipsoid0.axis[0]);
            R0.SetCol(1, ellipsoid0.axis[1]);
            R0.SetCol(2, ellipsoid0.axis[2]);
            Matrix3x3<T> D0{
                one / (ellipsoid0.extent[0] * ellipsoid0.extent[0]), zero, zero,
                zero, one / (ellipsoid0.extent[1] * ellipsoid0.extent[1]), zero,
                zero, zero, one / (ellipsoid0.extent[2] * ellipsoid0.extent[2]) };

            // Get the parameters of ellipsoid1.
            Vector3<T> K1 = ellipsoid1.center;
            Matrix3x3<T> R1;
            R1.SetCol(0, ellipsoid1.axis[0]);
            R1.SetCol(1, ellipsoid1.axis[1]);
            R1.SetCol(2, ellipsoid1.axis[2]);
            Matrix3x3<T> D1{
                one / (ellipsoid1.extent[0] * ellipsoid1.extent[0]), zero, zero,
                zero, one / (ellipsoid1.extent[1] * ellipsoid1.extent[1]), zero,
                zero, zero, one / (ellipsoid1.extent[2] * ellipsoid1.extent[2]) };

            // Compute K2.
            Matrix3x3<T> D0NegHalf{
                ellipsoid0.extent[0], zero, zero,
                zero, ellipsoid0.extent[1], zero,
                zero, zero, ellipsoid0.extent[2] };
            Matrix3x3<T> D0Half{
                one / ellipsoid0.extent[0], zero, zero,
                zero, one / ellipsoid0.extent[1], zero,
                zero, zero, one / ellipsoid0.extent[2] };
            Vector3<T> K2 = D0Half * ((K1 - K0) * R0);

            // Compute M2.
            Matrix3x3<T> R1TR0D0NegHalf = MultiplyATB(R1, R0 * D0NegHalf);
            Matrix3x3<T> M2 = MultiplyATB(R1TR0D0NegHalf, D1) * R1TR0D0NegHalf;

            // Factor M2 = R*D*R^T.
            SymmetricEigensolver3x3<T> es;
            std::array<T, 3> D;
            std::array<std::array<T, 3>, 3> evec;
            es(M2(0, 0), M2(0, 1), M2(0, 2), M2(1, 1), M2(1, 2), M2(2, 2), false, +1, D, evec);
            Matrix3x3<T> R;
            R.SetCol(0, evec[0]);
            R.SetCol(1, evec[1]);
            R.SetCol(2, evec[2]);

            // Compute K = R^T*K2.
            Vector3<T> K = K2 * R;

            // Transformed ellipsoid0 is Z^T*Z = 1 and transformed ellipsoid1
            // is (Z-K)^T*D*(Z-K) = 0.

            // The minimum and maximum squared distances from the origin of
            // points on transformed ellipsoid1 are used to determine whether
            // the ellipsoids intersect, are separated, or one contains the
            // other.
            T minSqrDistance = std::numeric_limits<T>::max();
            T maxSqrDistance = zero;
            int32_t i;

            if (K == Vector3<T>::Zero())
            {
                // The special case of common centers must be handled
                // separately.  It is not possible for the ellipsoids to be
                // separated.
                for (i = 0; i < 3; ++i)
                {
                    T invD = one / D[i];
                    if (invD < minSqrDistance)
                    {
                        minSqrDistance = invD;
                    }
                    if (invD > maxSqrDistance)
                    {
                        maxSqrDistance = invD;
                    }
                }

                if (maxSqrDistance < one)
                {
                    result.classification = Classification::ELLIPSOID0_CONTAINS_ELLIPSOID1;
                }
                else if (minSqrDistance > (T)1)
                {
                    result.classification = Classification::ELLIPSOID1_CONTAINS_ELLIPSOID0;
                }
                else
                {
                    result.classification = Classification::ELLIPSOIDS_INTERSECTING;
                }
                result.intersect = true;
                return result;
            }

            // The closest point P0 and farthest point P1 are solutions to
            // s0*D*(P0 - K) = P0 and s1*D*(P1 - K) = P1 for some scalars s0
            // and s1 that are roots to the function
            //   f(s) = d0*k0^2/(d0*s-1)^2 + d1*k1^2/(d1*s-1)^2
            //          + d2*k2^2/(d2*s-1)^2 - 1
            // where D = diagonal(d0,d1,d2) and K = (k0,k1,k2).
            T d0 = D[0], d1 = D[1], d2 = D[2];
            T c0 = K[0] * K[0], c1 = K[1] * K[1], c2 = K[2] * K[2];

            // Sort the values so that d0 >= d1 >= d2.  This allows us to
            // bound the roots of f(s), of which there are at most 6.
            std::vector<std::pair<T, T>> param(3);
            param[0] = std::make_pair(d0, c0);
            param[1] = std::make_pair(d1, c1);
            param[2] = std::make_pair(d2, c2);
            std::sort(param.begin(), param.end(), std::greater<std::pair<T, T>>());

            std::vector<std::pair<T, T>> valid;
            valid.reserve(3);
            if (param[0].first > param[1].first)
            {
                if (param[1].first > param[2].first)
                {
                    // d0 > d1 > d2
                    for (i = 0; i < 3; ++i)
                    {
                        if (param[i].second > (T)0)
                        {
                            valid.push_back(param[i]);
                        }
                    }
                }
                else
                {
                    // d0 > d1 = d2
                    if (param[0].second > (T)0)
                    {
                        valid.push_back(param[0]);
                    }
                    param[1].second += param[0].second;
                    if (param[1].second > (T)0)
                    {
                        valid.push_back(param[1]);
                    }
                }
            }
            else
            {
                if (param[1].first > param[2].first)
                {
                    // d0 = d1 > d2
                    param[0].second += param[1].second;
                    if (param[0].second > (T)0)
                    {
                        valid.push_back(param[0]);
                    }
                    if (param[2].second > (T)0)
                    {
                        valid.push_back(param[2]);
                    }
                }
                else
                {
                    // d0 = d1 = d2
                    param[0].second += param[1].second + param[2].second;
                    if (param[0].second > (T)0)
                    {
                        valid.push_back(param[0]);
                    }
                }
            }

            size_t numValid = valid.size();
            int32_t numRoots = 0;
            std::array<T, 6> roots{};
            if (numValid == 3)
            {
                GetRoots(valid[0].first, valid[1].first, valid[2].first,
                    valid[0].second, valid[1].second, valid[2].second, numRoots, roots.data());
            }
            else if (numValid == 2)
            {
                GetRoots(valid[0].first, valid[1].first, valid[0].second,
                    valid[1].second, numRoots, roots.data());
            }
            else if (numValid == 1)
            {
                GetRoots(valid[0].first, valid[0].second, numRoots, roots.data());
            }
            else
            {
                // numValid cannot be zero because we already handled case K = 0
                LogError("Unexpected condition.");
            }

            for (i = 0; i < numRoots; ++i)
            {
                T s = roots[i];
                T p0 = d0 * K[0] * s / (d0 * s - (T)1);
                T p1 = d1 * K[1] * s / (d1 * s - (T)1);
                T p2 = d2 * K[2] * s / (d2 * s - (T)1);
                T sqrDistance = p0 * p0 + p1 * p1 + p2 * p2;
                if (sqrDistance < minSqrDistance)
                {
                    minSqrDistance = sqrDistance;
                }
                if (sqrDistance > maxSqrDistance)
                {
                    maxSqrDistance = sqrDistance;
                }
            }

            if (maxSqrDistance < one)
            {
                result.intersect = true;
                result.classification = Classification::ELLIPSOID0_CONTAINS_ELLIPSOID1;
            }
            else if (minSqrDistance > (T)1)
            {
                if (d0 * c0 + d1 * c1 + d2 * c2 > one)
                {
                    result.intersect = false;
                    result.classification = Classification::ELLIPSOIDS_SEPARATED;
                }
                else
                {
                    result.intersect = true;
                    result.classification = Classification::ELLIPSOID1_CONTAINS_ELLIPSOID0;
                }
            }
            else
            {
                result.intersect = true;
                result.classification = Classification::ELLIPSOIDS_INTERSECTING;
            }

            return result;
        }

    private:
        void GetRoots(T d0, T c0, int32_t& numRoots, T* roots)
        {
            // f(s) = d0*c0/(d0*s-1)^2 - 1
            T const one = (T)1;
            T temp = std::sqrt(d0 * c0);
            T inv = one / d0;
            numRoots = 2;
            roots[0] = (one - temp) * inv;
            roots[1] = (one + temp) * inv;
        }

        void GetRoots(T d0, T d1, T c0, T c1, int32_t& numRoots, T* roots)
        {
            // f(s) = d0*c0/(d0*s-1)^2 + d1*c1/(d1*s-1)^2 - 1
            // with d0 > d1

            T const zero = (T)0;
            T const one = (T)1;
            T const two = (T)2;
            T d0c0 = d0 * c0;
            T d1c1 = d1 * c1;

            std::function<T(T)> F = [&one, d0, d1, d0c0, d1c1](T s)
            {
                T invN0 = one / (d0 * s - one);
                T invN1 = one / (d1 * s - one);
                T term0 = d0c0 * invN0 * invN0;
                T term1 = d1c1 * invN1 * invN1;
                T f = term0 + term1 - one;
                return f;
            };

            std::function<T(T)> DF = [&one, &two, d0, d1, d0c0, d1c1](T s)
            {
                T invN0 = one / (d0 * s - one);
                T invN1 = one / (d1 * s - one);
                T term0 = d0 * d0c0 * invN0 * invN0 * invN0;
                T term1 = d1 * d1c1 * invN1 * invN1 * invN1;
                T df = -two * (term0 + term1);
                return df;
            };

            uint32_t const maxIterations = 1024;
            uint32_t iterations;
            numRoots = 0;

            // TODO: What role does epsilon play?
            T const epsilon = (T)0.001;
            T multiplier0 = std::sqrt(two / (one - epsilon));
            T multiplier1 = std::sqrt(one / (one + epsilon));
            T sqrtd0c0 = std::sqrt(d0c0);
            T sqrtd1c1 = std::sqrt(d1c1);
            T invD0 = one / d0;
            T invD1 = one / d1;
            T temp0, temp1, smin, smax, s = (T)0;

            // Compute root in (-infinity,1/d0).
            temp0 = (one - multiplier0 * sqrtd0c0) * invD0;
            temp1 = (one - multiplier0 * sqrtd1c1) * invD1;

            smin = std::min(temp0, temp1);
            LogAssert(F(smin) < zero, "Unexpected condition.");
            smax = (one - multiplier1 * sqrtd0c0) * invD0;
            LogAssert(F(smax) > zero, "Unexpected condition.");
            iterations = RootsBisection<T>::Find(F, smin, smax, maxIterations, s);
            LogAssert(iterations > 0, "Unexpected condition.");

            roots[numRoots++] = s;

            // Compute roots (if any) in (1/d0,1/d1).  It is the case that
            //   F(1/d0) = +infinity, F'(1/d0) = -infinity
            //   F(1/d1) = +infinity, F'(1/d1) = +infinity
            //   F"(s) > 0 for all s in the domain of F
            // Compute the unique root r of F'(s) on (1/d0,1/d1).  The
            // bisector needs only the signs at the endpoints, so we pass -1
            // and +1 instead of the infinite values.  If F(r) < 0, F(s) has
            // two roots in the interval.  If F(r) = 0, F(s) has only one root
            // in the interval.
            T smid = (T)0;
            iterations = RootsBisection<T>::Find(DF, invD0, invD1, -one, one,
                maxIterations, smid);
            LogAssert(iterations > 0, "Unexpected condition.");
            if (F(smid) < zero)
            {
                // Pass in signs rather than infinities, because the bisector
                // cares only about the signs.
                iterations = RootsBisection<T>::Find(F, invD0, smid, one, -one,
                    maxIterations, s);
                LogAssert(iterations > 0, "Unexpected condition.");
                roots[numRoots++] = s;
                iterations = RootsBisection<T>::Find(F, smid, invD1, -one, one,
                    maxIterations, s);
                LogAssert(iterations > 0, "Unexpected condition.");
                roots[numRoots++] = s;
            }

            // Compute root in (1/d1,+infinity).
            temp0 = (one + multiplier0 * sqrtd0c0) * invD0;
            temp1 = (one + multiplier0 * sqrtd1c1) * invD1;
            smax = std::max(temp0, temp1);
            LogAssert(F(smax) < zero, "Unexpected condition.");
            smin = (one + multiplier1 * sqrtd1c1) * invD1;
            LogAssert(F(smin) > zero, "Unexpected condition.");
            iterations = RootsBisection<T>::Find(F, smin, smax, maxIterations, s);
            LogAssert(iterations > 0, "Unexpected condition.");
            roots[numRoots++] = s;
        }

        void GetRoots(T d0, T d1, T d2, T c0, T c1, T c2,
            int32_t& numRoots, T* roots)
        {
            // f(s) = d0*c0/(d0*s-1)^2 + d1*c1/(d1*s-1)^2
            // + d2*c2/(d2*s-1)^2 - 1 with d0 > d1 > d2

            T const zero = (T)0;
            T const one = (T)1;
            T const three = (T)3;
            T d0c0 = d0 * c0;
            T d1c1 = d1 * c1;
            T d2c2 = d2 * c2;

            std::function<T(T)> F = [&one, d0, d1, d2, d0c0, d1c1, d2c2](T s)
            {
                T invN0 = one / (d0 * s - one);
                T invN1 = one / (d1 * s - one);
                T invN2 = one / (d2 * s - one);
                T term0 = d0c0 * invN0 * invN0;
                T term1 = d1c1 * invN1 * invN1;
                T term2 = d2c2 * invN2 * invN2;
                T f = term0 + term1 + term2 - one;
                return f;
            };

            std::function<T(T)> DF = [&one, d0, d1, d2, d0c0, d1c1, d2c2](T s)
            {
                T const two = (T)2;
                T invN0 = one / (d0 * s - one);
                T invN1 = one / (d1 * s - one);
                T invN2 = one / (d2 * s - one);
                T term0 = d0 * d0c0 * invN0 * invN0 * invN0;
                T term1 = d1 * d1c1 * invN1 * invN1 * invN1;
                T term2 = d2 * d2c2 * invN2 * invN2 * invN2;
                T df = -two * (term0 + term1 + term2);
                return df;
            };

            uint32_t const maxIterations = 1024;
            uint32_t iterations;
            numRoots = 0;

            // TODO: What role does epsilon play?
            T epsilon = (T)0.001;
            T multiplier0 = std::sqrt(three / (one - epsilon));
            T multiplier1 = std::sqrt(one / (one + epsilon));
            T sqrtd0c0 = std::sqrt(d0c0);
            T sqrtd1c1 = std::sqrt(d1c1);
            T sqrtd2c2 = std::sqrt(d2c2);
            T invD0 = one / d0;
            T invD1 = one / d1;
            T invD2 = one / d2;
            T temp0, temp1, temp2, smin, smax, s = (T)0;

            // Compute root in (-infinity,1/d0).
            temp0 = (one - multiplier0 * sqrtd0c0) * invD0;
            temp1 = (one - multiplier0 * sqrtd1c1) * invD1;
            temp2 = (one - multiplier0 * sqrtd2c2) * invD2;
            smin = std::min(std::min(temp0, temp1), temp2);
            LogAssert(F(smin) < zero, "Unexpected condition.");
            smax = (one - multiplier1 * sqrtd0c0) * invD0;
            LogAssert(F(smax) > zero, "Unexpected condition.");
            iterations = RootsBisection<T>::Find(F, smin, smax, maxIterations, s);
            LogAssert(iterations > 0, "Unexpected condition.");
            roots[numRoots++] = s;

            // Compute roots (if any) in (1/d0,1/d1).  It is the case that
            //   F(1/d0) = +infinity, F'(1/d0) = -infinity
            //   F(1/d1) = +infinity, F'(1/d1) = +infinity
            //   F"(s) > 0 for all s in the domain of F
            // Compute the unique root r of F'(s) on (1/d0,1/d1).  The
            // bisector needs only the signs at the endpoints, so we pass -1
            // and +1 instead of the infinite values.  If F(r) < 0, F(s) has
            // two roots in the interval.  If F(r) = 0, F(s) has only one root
            // in the interval.
            T smid = (T)0;
            iterations = RootsBisection<T>::Find(DF, invD0, invD1, -one, one,
                maxIterations, smid);
            LogAssert(iterations > 0, "Unexpected condition.");
            if (F(smid) < zero)
            {
                // Pass in signs rather than infinities, because the bisector cares
                // only about the signs.
                iterations = RootsBisection<T>::Find(F, invD0, smid, one, -one,
                    maxIterations, s);
                LogAssert(iterations > 0, "Unexpected condition.");
                roots[numRoots++] = s;
                iterations = RootsBisection<T>::Find(F, smid, invD1, -one, one,
                    maxIterations, s);
                LogAssert(iterations > 0, "Unexpected condition.");
                roots[numRoots++] = s;
            }

            // Compute roots (if any) in (1/d1,1/d2).  It is the case that
            //   F(1/d1) = +infinity, F'(1/d1) = -infinity
            //   F(1/d2) = +infinity, F'(1/d2) = +infinity
            //   F"(s) > 0 for all s in the domain of F
            // Compute the unique root r of F'(s) on (1/d1,1/d2).  The
            // bisector needs only the signs at the endpoints, so we pass -1
            // and +1 instead of the infinite values.  If F(r) < 0, F(s) has
            // two roots in the interval.  If F(r) = 0, F(s) has only one root
            // in the interval.
            iterations = RootsBisection<T>::Find(DF, invD1, invD2, -one, one,
                maxIterations, smid);
            LogAssert(iterations > 0, "Unexpected condition.");
            if (F(smid) < zero)
            {
                // Pass in signs rather than infinities, because the bisector
                // cares only about the signs.
                iterations = RootsBisection<T>::Find(F, invD1, smid, one, -one,
                    maxIterations, s);
                LogAssert(iterations > 0, "Unexpected condition.");
                roots[numRoots++] = s;
                iterations = RootsBisection<T>::Find(F, smid, invD2, -one, one,
                    maxIterations, s);
                LogAssert(iterations > 0, "Unexpected condition.");
                roots[numRoots++] = s;
            }

            // Compute root in (1/d2,+infinity).
            temp0 = (one + multiplier0 * sqrtd0c0) * invD0;
            temp1 = (one + multiplier0 * sqrtd1c1) * invD1;
            temp2 = (one + multiplier0 * sqrtd2c2) * invD2;
            smax = std::max(std::max(temp0, temp1), temp2);
            LogAssert(F(smax) < zero, "Unexpected condition.");
            smin = (one + multiplier1 * sqrtd2c2) * invD2;
            LogAssert(F(smin) > zero, "Unexpected condition.");
            iterations = RootsBisection<T>::Find(F, smin, smax, maxIterations, s);
            LogAssert(iterations > 0, "Unexpected condition.");
            roots[numRoots++] = s;
        }
    };
}
