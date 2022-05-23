// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/DistanceClosestPointQuery.h>
#include <GTL/Mathematics/Primitives/ND/Hyperellipsoid.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T, size_t N>
    class DCPQuery<T, Vector<T, N>, Hyperellipsoid<T, N>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<Vector<T, N>, 2> closest;
        };

        // The query for any hyperellipsoid.
        Output operator()(Vector<T, N> const& point,
            Hyperellipsoid<T, N> const& hyperellipsoid)
        {
            Output output{};

            // Compute the coordinates of Y in the hyperellipsoid coordinate
            // system.
            Vector<T, N> diff = point - hyperellipsoid.center;
            Vector<T, N> y{};
            for (size_t i = 0; i < N; ++i)
            {
                y[i] = Dot(diff, hyperellipsoid.axis[i]);
            }

            // Compute the closest hyperellipsoid point in the axis-aligned
            // coordinate system.
            Vector<T, N> x{};
            output.sqrDistance = SqrDistance(hyperellipsoid.extent, y, x);
            output.distance = std::sqrt(output.sqrDistance);

            // Convert back to the original coordinate system.
            output.closest[0] = point;
            output.closest[1] = hyperellipsoid.center;
            for (size_t i = 0; i < N; ++i)
            {
                output.closest[1] += x[i] * hyperellipsoid.axis[i];
            }

            return output;
        }

        // The 'hyperellipsoid' is assumed to be axis-aligned and centered at
        // the origin, so only the extent[] values are used.
        Output operator()(Vector<T, N> const& point, Vector<T, N> const& extent)
        {
            Output output{};
            output.closest[0] = point;
            output.sqrDistance = SqrDistance(extent, point, output.closest[1]);
            output.distance = std::sqrt(output.sqrDistance);
            return output;
        }

    private:
        // The hyperellipsoid is sum_{d=0}^{N-1} (x[d]/e[d])^2 = 1 with no
        // constraints on the orderind of the e[d]. The query point is
        // (y[0],...,y[N-1]) with no constraints on the signs of the
        // components. The function returns the squared distance from the
        // query point to the hyperellipsoid. It also computes the
        // hyperellipsoid point (x[0],...,x[N-1]) that is closest to
        // (y[0],...,y[N-1]).
        T SqrDistance(Vector<T, N> const& e, Vector<T, N> const& y, Vector<T, N>& x)
        {
            // Determine negations for y to the first octant.
            std::array<bool, N> negate{};
            for (size_t i = 0; i < N; ++i)
            {
                negate[i] = (y[i] < C_<T>(0));
            }

            // Determine the axis order for decreasing extents.
            std::array<std::pair<T, size_t>, N> permute{};
            for (size_t i = 0; i < N; ++i)
            {
                permute[i].first = -e[i];
                permute[i].second = i;
            }
            std::sort(permute.begin(), permute.end());

            std::array<size_t, N> invPermute{};
            for (size_t i = 0; i < N; ++i)
            {
                invPermute[permute[i].second] = i;
            }

            Vector<T, N> locE{}, locY{};
            for (size_t i = 0; i < N; ++i)
            {
                size_t j = permute[i].second;
                locE[i] = e[j];
                locY[i] = std::fabs(y[j]);
            }

            Vector<T, N> locX{};
            T sqrDistance = SqrDistanceSpecial(locE, locY, locX);

            // Restore the axis order and reflections.
            for (size_t i = 0; i < N; ++i)
            {
                size_t j = invPermute[i];
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
        T SqrDistanceSpecial(Vector<T, N> const& e, Vector<T, N> const& y, Vector<T, N>& x)
        {
            T sqrDistance = C_<T>(0);

            Vector<T, N> ePos{}, yPos{}, xPos{};
            size_t numPos = 0;
            for (size_t i = 0; i < N; ++i)
            {
                if (y[i] > C_<T>(0))
                {
                    ePos[numPos] = e[i];
                    yPos[numPos] = y[i];
                    ++numPos;
                }
                else
                {
                    x[i] = C_<T>(0);
                }
            }

            if (y[N - 1] > C_<T>(0))
            {
                sqrDistance = Bisector(numPos, ePos, yPos, xPos);
            }
            else  // y[N-1] = 0
            {
                Vector<T, N - 1> numer{}, denom{};
                T eNm1Sqr = e[N - 1] * e[N - 1];
                for (size_t i = 0; i < numPos; ++i)
                {
                    numer[i] = ePos[i] * yPos[i];
                    denom[i] = ePos[i] * ePos[i] - eNm1Sqr;
                }

                bool inSubHyperbox = true;
                for (size_t i = 0; i < numPos; ++i)
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
                    Vector<T, N - 1> xde{};
                    T discr = C_<T>(1);
                    for (size_t i = 0; i < numPos; ++i)
                    {
                        xde[i] = numer[i] / denom[i];
                        discr -= xde[i] * xde[i];
                    }
                    if (discr > C_<T>(0))
                    {
                        // yPos[] is inside the subhyperellipsoid. The
                        // closest hyperellipsoid point has x[N-1] > 0.
                        sqrDistance = C_<T>(0);
                        for (size_t i = 0; i < numPos; ++i)
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
                    x[N - 1] = C_<T>(0);
                    sqrDistance = Bisector(numPos, ePos, yPos, xPos);
                }
            }

            // Fill in those x[] values that were not zeroed out initially.
            numPos = 0;
            for (size_t i = 0; i < N; ++i)
            {
                if (y[i] > C_<T>(0))
                {
                    x[i] = xPos[numPos];
                    ++numPos;
                }
            }

            return sqrDistance;
        }

        // The bisection algorithm to find the unique root of F(t).
        T Bisector(size_t numComponents, Vector<T, N> const& e,
            Vector<T, N> const& y, Vector<T, N>& x)
        {
            T sumZSqr = C_<T>(0);
            Vector<T, N> z{};
            for (size_t i = 0; i < numComponents; ++i)
            {
                z[i] = y[i] / e[i];
                sumZSqr += z[i] * z[i];
            }

            if (sumZSqr == C_<T>(1))
            {
                // The point is on the hyperellipsoid.
                for (size_t i = 0; i < numComponents; ++i)
                {
                    x[i] = y[i];
                }
                return C_<T>(0);
            }

            T emin = e[numComponents - 1];
            Vector<T, N> pSqr{}, numerator{};  // zero vectors
            for (size_t i = 0; i < numComponents; ++i)
            {
                T p = e[i] / emin;
                pSqr[i] = p * p;
                numerator[i] = pSqr[i] * z[i];
            }

            T s = C_<T>(0), smin = z[numComponents - 1] - C_<T>(1), smax{};
            if (sumZSqr < C_<T>(1))
            {
                // The point is strictly inside the hyperellipsoid.
                smax = C_<T>(0);
            }
            else
            {
                // The point is strictly outside the hyperellipsoid.
                smax = Length(numerator) - C_<T>(1);
            }

            size_t constexpr jmax = 2048;
            for (size_t j = 0; j < jmax; ++j)
            {
                s = C_<T>(1, 2) * (smin + smax);
                if (s == smin || s == smax)
                {
                    break;
                }

                T g = -C_<T>(1);
                for (size_t i = 0; i < numComponents; ++i)
                {
                    T ratio = numerator[i] / (s + pSqr[i]);
                    g += ratio * ratio;
                }

                if (g > C_<T>(0))
                {
                    smin = s;
                }
                else if (g < C_<T>(0))
                {
                    smax = s;
                }
                else
                {
                    break;
                }
            }

            T sqrDistance = C_<T>(0);
            for (size_t i = 0; i < numComponents; ++i)
            {
                x[i] = pSqr[i] * y[i] / (s + pSqr[i]);
                T diff = x[i] - y[i];
                sqrDistance += diff * diff;
            }
            return sqrDistance;
        }
    };
}
