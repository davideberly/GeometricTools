// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance from a point to a hyperellipsoid in nD. The
// hyperellipsoid is considered to be a closed surface, not a solid. In 2D,
// this is a point-ellipse distance query. In 3D, this is a point-ellipsoid
// distance query. The following document describes the algorithm.
//   https://www.geometrictools.com/Documentation/DistancePointEllipseEllipsoid.pdf
// The hyperellipsoid can have arbitrary center and orientation; that is, it
// does not have to be axis-aligned with center at the origin.
//
// For the 2D query,
//   Vector2<T> point;  // initialized to something
//   Ellipse2<T> ellipse;  // initialized to something
//   DCPQuery<T, Vector2<T>, Ellipse2<T>> query{};
//   auto output = query(point, ellipse);
//   T distance = output.distance;
//   Vector2<T> closestEllipsePoint = output.closest[1];
//
// For the 3D query,
//   Vector3<T> point;  // initialized to something
//   Ellipsoid3<T> ellipsoid;  // initialized to something
//   DCPQuery<T, Vector3<T>, Ellipsoid3<T>> query{};
//   auto output = query(point, ellipsoid);
//   T distance = output.distance;
//   Vector3<T> closestEllipsoidPoint = output.closest[1];
//
// The input point is stored in closest[0]. The closest point on the
// hyperellipsoid is stored in closest[1].

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Hyperellipsoid.h>
#include <Mathematics/Vector.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <utility>

namespace gte
{
    template <int32_t N, typename T>
    class DCPQuery<T, Vector<N, T>, Hyperellipsoid<N, T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                closest{ Vector<N, T>::Zero(), Vector<N, T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<Vector<N, T>, 2> closest;
        };

        // The query for any hyperellipsoid.
        Result operator()(Vector<N, T> const& point,
            Hyperellipsoid<N, T> const& hyperellipsoid)
        {
            Result result{};

            // Compute the coordinates of Y in the hyperellipsoid coordinate
            // system.
            Vector<N, T> diff = point - hyperellipsoid.center;
            Vector<N, T> y{};
            for (int32_t i = 0; i < N; ++i)
            {
                y[i] = Dot(diff, hyperellipsoid.axis[i]);
            }

            // Compute the closest hyperellipsoid point in the axis-aligned
            // coordinate system.
            Vector<N, T> x{};
            result.sqrDistance = SqrDistance(hyperellipsoid.extent, y, x);
            result.distance = std::sqrt(result.sqrDistance);

            // Convert back to the original coordinate system.
            result.closest[0] = point;
            result.closest[1] = hyperellipsoid.center;
            for (int32_t i = 0; i < N; ++i)
            {
                result.closest[1] += x[i] * hyperellipsoid.axis[i];
            }

            return result;
        }

        // The 'hyperellipsoid' is assumed to be axis-aligned and centered at
        // the origin , so only the extent[] values are used.
        Result operator()(Vector<N, T> const& point, Vector<N, T> const& extent)
        {
            Result result{};
            result.closest[0] = point;
            result.sqrDistance = SqrDistance(extent, point, result.closest[1]);
            result.distance = std::sqrt(result.sqrDistance);
            return result;
        }

    private:
        // The hyperellipsoid is sum_{d=0}^{N-1} (x[d]/e[d])^2 = 1 with no
        // constraints on the orderind of the e[d]. The query point is
        // (y[0],...,y[N-1]) with no constraints on the signs of the
        // components. The function returns the squared distance from the
        // query point to the hyperellipsoid. It also computes the
        // hyperellipsoid point (x[0],...,x[N-1]) that is closest to
        // (y[0],...,y[N-1]).
        T SqrDistance(Vector<N, T> const& e, Vector<N, T> const& y, Vector<N, T>& x)
        {
            // Determine negations for y to the first octant.
            T const zero = static_cast<T>(0);
            std::array<bool, N> negate{};
            for (int32_t i = 0; i < N; ++i)
            {
                negate[i] = (y[i] < zero);
            }

            // Determine the axis order for decreasing extents.
            std::array<std::pair<T, int32_t>, N> permute{};
            for (int32_t i = 0; i < N; ++i)
            {
                permute[i].first = -e[i];
                permute[i].second = i;
            }
            std::sort(permute.begin(), permute.end());

            std::array<int32_t, N> invPermute{};
            for (int32_t i = 0; i < N; ++i)
            {
                invPermute[permute[i].second] = i;
            }

            Vector<N, T> locE{}, locY{};
            for (int32_t i = 0; i < N; ++i)
            {
                int32_t j = permute[i].second;
                locE[i] = e[j];
                locY[i] = std::fabs(y[j]);
            }

            Vector<N, T> locX{};
            T sqrDistance = SqrDistanceSpecial(locE, locY, locX);

            // Restore the axis order and reflections.
            for (int32_t i = 0; i < N; ++i)
            {
                int32_t j = invPermute[i];
                if (negate[i])
                {
                    locX[j] = -locX[j];
                }
                x[i] = locX[j];
            }

            return sqrDistance;
        }

        // The hyperellipsoid is sum_{d=0}^{N-1} (x[d]/e[d])^2 = 1 with the
        // e[d] positive and nonincreasing:  e[d] >= e[d + 1] for all d. The
        // query point is (y[0],...,y[N-1]) with y[d] >= 0 for all d. The
        // function returns the squared distance from the query point to the
        // hyperellipsoid. It also computes the hyperellipsoid point
        // (x[0],...,x[N-1]) that is closest to (y[0],...,y[N-1]), where
        // x[d] >= 0 for all d.
        T SqrDistanceSpecial(Vector<N, T> const& e, Vector<N, T> const& y, Vector<N, T>& x)
        {
            T const zero = static_cast<T>(0);
            T sqrDistance = zero;

            Vector<N, T> ePos{}, yPos{}, xPos{};
            int32_t numPos = 0;
            for (int32_t i = 0; i < N; ++i)
            {
                if (y[i] > zero)
                {
                    ePos[numPos] = e[i];
                    yPos[numPos] = y[i];
                    ++numPos;
                }
                else
                {
                    x[i] = zero;
                }
            }

            if (y[N - 1] > zero)
            {
                sqrDistance = Bisector(numPos, ePos, yPos, xPos);
            }
            else  // y[N-1] = 0
            {
                Vector<N - 1, T> numer{}, denom{};
                T eNm1Sqr = e[N - 1] * e[N - 1];
                for (int32_t i = 0; i < numPos; ++i)
                {
                    numer[i] = ePos[i] * yPos[i];
                    denom[i] = ePos[i] * ePos[i] - eNm1Sqr;
                }

                bool inSubHyperbox = true;
                for (int32_t i = 0; i < numPos; ++i)
                {
                    if (numer[i] >= denom[i])
                    {
                        inSubHyperbox = false;
                        break;
                    }
                }

                bool inSubHyperellipsoid = false;
                if (inSubHyperbox)
                {
                    // yPos[] is inside the axis-aligned bounding box of the
                    // subhyperellipsoid. This intermediate test is designed
                    // to guard against the division by zero when
                    // ePos[i] == e[N-1] for some i.
                    Vector<N - 1, T> xde{};
                    T discr = static_cast<T>(1);
                    for (int32_t i = 0; i < numPos; ++i)
                    {
                        xde[i] = numer[i] / denom[i];
                        discr -= xde[i] * xde[i];
                    }
                    if (discr > zero)
                    {
                        // yPos[] is inside the subhyperellipsoid. The
                        // closest hyperellipsoid point has x[N-1] > 0.
                        sqrDistance = zero;
                        for (int32_t i = 0; i < numPos; ++i)
                        {
                            xPos[i] = ePos[i] * xde[i];
                            T diff = xPos[i] - yPos[i];
                            sqrDistance += diff * diff;
                        }
                        x[N - 1] = e[N - 1] * std::sqrt(discr);
                        sqrDistance += x[N - 1] * x[N - 1];
                        inSubHyperellipsoid = true;
                    }
                }

                if (!inSubHyperellipsoid)
                {
                    // yPos[] is outside the subhyperellipsoid. The closest
                    // hyperellipsoid point has x[N-1] == 0 and is on the
                    // domain-boundary hyperellipsoid.
                    x[N - 1] = zero;
                    sqrDistance = Bisector(numPos, ePos, yPos, xPos);
                }
            }

            // Fill in those x[] values that were not zeroed out initially.
            numPos = 0;
            for (int32_t i = 0; i < N; ++i)
            {
                if (y[i] > zero)
                {
                    x[i] = xPos[numPos];
                    ++numPos;
                }
            }

            return sqrDistance;
        }

        // The bisection algorithm to find the unique root of F(t).
        T Bisector(int32_t numComponents, Vector<N, T> const& e,
            Vector<N, T> const& y, Vector<N, T>& x)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const half = static_cast<T>(0.5);

            T sumZSqr = zero;
            Vector<N, T> z{};
            for (int32_t i = 0; i < numComponents; ++i)
            {
                z[i] = y[i] / e[i];
                sumZSqr += z[i] * z[i];
            }

            if (sumZSqr == one)
            {
                // The point is on the hyperellipsoid.
                for (int32_t i = 0; i < numComponents; ++i)
                {
                    x[i] = y[i];
                }
                return zero;
            }

            T emin = e[numComponents - 1];
            Vector<N, T> pSqr{}, numerator{};
            pSqr.MakeZero();
            numerator.MakeZero();
            for (int32_t i = 0; i < numComponents; ++i)
            {
                T p = e[i] / emin;
                pSqr[i] = p * p;
                numerator[i] = pSqr[i] * z[i];
            }

            T s = zero, smin = z[numComponents - 1] - one, smax{};
            if (sumZSqr < one)
            {
                // The point is strictly inside the hyperellipsoid.
                smax = zero;
            }
            else
            {
                // The point is strictly outside the hyperellipsoid.
                smax = Length(numerator, true) - one;
            }

            // The use of 'double' is intentional in case T is a BSNumber
            // or BSRational type. We want the bisections to terminate in a
            // reasonable amount of time.
            uint32_t const jmax = 2048u;
            for (uint32_t j = 0; j < jmax; ++j)
            {
                s = half * (smin + smax);
                if (s == smin || s == smax)
                {
                    break;
                }

                T g = -one;
                for (int32_t i = 0; i < numComponents; ++i)
                {
                    T ratio = numerator[i] / (s + pSqr[i]);
                    g += ratio * ratio;
                }

                if (g > zero)
                {
                    smin = s;
                }
                else if (g < zero)
                {
                    smax = s;
                }
                else
                {
                    break;
                }
            }

            T sqrDistance = zero;
            for (int32_t i = 0; i < numComponents; ++i)
            {
                x[i] = pSqr[i] * y[i] / (s + pSqr[i]);
                T diff = x[i] - y[i];
                sqrDistance += diff * diff;
            }
            return sqrDistance;
        }
    };

    // Template aliases for convenience.
    template <int32_t N, typename T>
    using DCPPointHyperellipsoid = DCPQuery<T, Vector<N, T>, Hyperellipsoid<N, T>>;

    template <typename T>
    using DCPPoint2Ellipse2 = DCPPointHyperellipsoid<2, T>;

    template <typename T>
    using DCPPoint3Ellipsoid3 = DCPPointHyperellipsoid<3, T>;
}
