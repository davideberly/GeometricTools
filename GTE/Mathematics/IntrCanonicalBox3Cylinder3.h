// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The query is for finite cylinders. The cylinder and box are considered to
// be solids. The cylinder has center C, unit-length axis direction D, radius
// r and height h. The canonical box has center at the origin and extents E.
//
// The abstract algorithm clips the canonical box by the planes of the
// cylinder end disks to obtain a convex polyhedron Q. This polyhedron is
// projected to a convex polygon P in the plane Dot(D, X - C) = 0. The
// cylinder axis projects to C. The box and cylinder intersect when
// Distance(C,P) <= r. If C is inside or on P, the distance is 0 and there
// is an intersection. If C is outside P, the distance is the minimum of
// the distances from C to the edges of P.
//
// A direct implementation computes a set of points S whose members include
// vertices of Q that are contained by the slab and include intersection
// points between edges of Q and the slab planes. The points of S are
// projected to the plane Dot(D, X - C) = 0 and represented by 2-tuples
// (a,b) = a*U + b*V, where {D, U, V} is a right-handed orthonormal basis.
// A convex hull algorithm is applied to the projected 2-tuples to obtain P.
// An implementation can be found at
//   https://github.com/SebWouters/AabbCyl
// Thanks to Seb Wouters for pointing out the numerical robustness problems
// when using a floating-point LCP solver to implement the query.
// 
// An alternate implementation is described in
//   https://www.geometrictools.com/Documentation/IntersectionBoxCylinder.pdf
// and avoids using a generic convex hull algorithm. Reductions in dimension
// occur based on the number of 0-valued components of the cylinder axis
// direction.

#include <Mathematics/Logger.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/CanonicalBox.h>
#include <Mathematics/Cylinder3.h>
#include <Mathematics/Vector2.h>
#include <Mathematics/Vector3.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, CanonicalBox3<T>, Cylinder3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(CanonicalBox3<T> const& box, Cylinder3<T> const& cylinder)
        {
            LogAssert(
                cylinder.IsFinite(),
                "Infinite cylinders are not yet supported.");

            // The result.intersect is initially false.
            Result result{};

            if (BoxIsOutsideCylinderSlab(box, cylinder))
            {
                // The box does not intersect the slab, so it does not
                // intersect the cylinder.
                result.intersect = false;
                return result;
            }

            // Apply reflections to obtain a cylinder whose axis direction is
            // in the first octant (positive- or zero-valued components). The
            // reflections applied to the canonical box do not require any
            // computational changes.
            T const zero = static_cast<T>(0);
            Vector3<T> C = cylinder.axis.origin;
            Vector3<T> D = cylinder.axis.direction;
            T const& r = cylinder.radius;
            T hDiv2 = static_cast<T>(0.5) * cylinder.height;
            Vector3<T> const& E = box.extent;
            for (int32_t i = 0; i < 3; ++i)
            {
                if (D[i] < zero)
                {
                    C[i] = -C[i];
                    D[i] = -D[i];
                }
            }

            // D is now in the first octant. The box vertices are
            //   V[0] = (-E[0],-E[1],-E[2]), V[4] = (-E[0],-E[1],+E[2]) 
            //   V[1] = (+E[0],-E[1],-E[2]), V[5] = (+E[0],-E[1],+E[2])
            //   V[2] = (-E[0],+E[1],-E[2]), V[6] = (-E[0],+E[1],+E[2])
            //   V[3] = (+E[0],+E[1],-E[2]), V[7] = (+E[0],+E[1],+E[2])
            if (D[0] > zero)
            {
                if (D[1] > zero)
                {
                    if (D[2] > zero)  // (+,+,+)
                    {
                        result.intersect = DoQueryNoZeros(C, D, r, hDiv2, E);
                    }
                    else  // (+,+,0)
                    {
                        result.intersect = DoQueryOneZero({ 0, 1, 2 }, C, D, r, hDiv2, E);
                    }
                }
                else
                {
                    if (D[2] > zero)  // (+,0,+)
                    {
                        result.intersect = DoQueryOneZero({ 2, 0, 1 }, C, D, r, hDiv2, E);
                    }
                    else  // (+,0,0)
                    {
                        result.intersect = DoQueryTwoZeros({ 0, 1, 2 }, C, r, E);
                    }
                }
            }
            else
            {
                if (D[1] > zero)
                {
                    if (D[2] > zero)  // (0,+,+)
                    {
                        result.intersect = DoQueryOneZero({ 1, 2, 0 }, C, D, r, hDiv2, E);
                    }
                    else  // (0,+,0)
                    {
                        result.intersect = DoQueryTwoZeros({ 1, 2, 0 }, C, r, E);
                    }
                }
                else
                {
                    if (D[2] > zero)  // (0,0,+)
                    {
                        result.intersect = DoQueryTwoZeros({ 2, 0, 1 }, C, r, E);
                    }
                    else  // (0,0,0)
                    {
                        LogError("The cylinder direction cannot be (0,0,0).");
                    }
                }
            }

            return result;
        }

    private:
        // Test whether the box is outside the slab contained by the planes
        // of the cylinder end disks. This is accomplished by computing the
        // interval of projection of the box onto the cylinder axis.
        static bool BoxIsOutsideCylinderSlab(CanonicalBox3<T> const& box,
            Cylinder3<T> const& cylinder)
        {
            // Convenient quantities.
            Vector3<T> const& C = cylinder.axis.origin;
            Vector3<T> const& D = cylinder.axis.direction;
            Vector3<T> absD{ std::fabs(D[0]), std::fabs(D[1]), std::fabs(D[2]) };
            T hDiv2 = static_cast<T>(0.5) * cylinder.height;
            Vector3<T> const& E = box.extent;

            // Let g be the interval center, p be the interval radius and h be
            // the cylinder height. The culling test is g - p > h/2 (box above
            // the slab) or g + p < -h/2 (box below the slab). The tests can
            // be rewritten as g > p + h/2 or -g > p + h/2. In turn these are
            // combined to |g| > p + h/2.
            T intervalCenter = -Dot(D, C);  // Dot(D, boxCenter-cylCenter)
            T intervalRadius = Dot(E, absD);
            if (std::fabs(intervalCenter) > intervalRadius + hDiv2)
            {
                // The box does not intersect the slab, so it does not
                // intersect the cylinder.
                return true;
            }
            else
            {
                // The box intersects the slab. At this time it is unknown
                // whether the box intersects the cylinder.
                return false;
            }
        }

        static bool CylinderAxisIntersectsBox3D(Vector3<T> const& C, Vector3<T> const& D,
            T const& hDiv2, Vector3<T> const& E)
        {
            Vector3<T> negEmCDivD
            {
                (-E[0] - C[0]) / D[0],
                (-E[1] - C[1]) / D[1],
                (-E[2] - C[2]) / D[2]
            };

            Vector3<T> posEmCDivD
            {
                (E[0] - C[0]) / D[0],
                (E[1] - C[1]) / D[1],
                (E[2] - C[2]) / D[2]
            };

            T max01 = std::max(negEmCDivD[0], negEmCDivD[1]);
            T max23 = std::max(negEmCDivD[2], -hDiv2);
            T lower = std::max(max01, max23);
            T min01 = std::min(posEmCDivD[0], posEmCDivD[1]);
            T min23 = std::min(posEmCDivD[2], hDiv2);
            T upper = std::min(min01, min23);
            return lower <= upper;
        }

        // Compute the distance from (0,0) to the projection of the segment
        // <P0,P1>. The projection plane has origin C and is spanned by the
        // orthonormal vectors W0 and W1.
        static T ComputeSqrDistance(Vector3<T> const& P0, Vector3<T> const& P1,
            Vector3<T> const& C, Vector3<T> const& W0, Vector3<T> const& W1)
        {
            Vector3<T> P0mC = P0 - C;
            Vector3<T> P1mC = P1 - C;
            Vector2<T> Q0{ Dot(W0, P0mC), Dot(W1, P0mC) };
            Vector2<T> Q1{ Dot(W0, P1mC), Dot(W1, P1mC) };

            T const zero = static_cast<T>(0);
            Vector2<T> direction = Q1 - Q0;
            T s = Dot(direction, Q1);
            if (s <= zero)
            {
                return Dot(Q1, Q1);
            }
            else
            {
                s = Dot(direction, Q0);
                if (s >= zero)
                {
                    return Dot(Q0, Q0);
                }
                else
                {
                    s /= Dot(direction, direction);
                    Vector2<T> closest = Q0 - s * direction;
                    return Dot(closest, closest);
                }
            }
        }

        static bool DoQueryTwoZeros(std::array<int32_t, 3> const& i,
            Vector3<T> const& C, T const& r, Vector3<T> const& E)
        {
            // The 2-tuple (C[i[1]], C[i[2]]) is the projected cylinder axis.
            // The 2-tuple (E[i[1]], E[i[2]]) is the extent of the projected
            // canonical box, which is an axis-aligned rectangle.
            T absC1 = std::fabs(C[i[1]]), absC2 = std::fabs(C[i[2]]);
            T E1 = E[i[1]], E2 = E[i[2]];

            // Test whether the cylinder axis and canonical box intersect.
            if (absC1 <= E1 && absC2 <= E2)
            {
                return true;
            }

            // Compute the squared distance from the projected cylinder axis
            // to the projected canonical box.
            T const zero = static_cast<T>(0);
            T sqrDistance = zero;
            T delta = absC1 - E1;
            if (delta > zero)
            {
                sqrDistance += delta * delta;
            }

            delta = absC2 - E2;
            if (delta > zero)
            {
                sqrDistance += delta * delta;
            }

            return sqrDistance <= r * r;
        }

        static bool DoQueryOneZero(std::array<int32_t, 3> const& i,
            Vector3<T> const& C, Vector3<T> const& D, T const& r, T const& hDiv2,
            Vector3<T> const& E)
        {
            T const& c0 = C[i[0]];
            T const& c1 = C[i[1]];
            T const& c2 = C[i[2]];
            T const& d0 = D[i[0]];
            T const& d1 = D[i[1]];
            T const& e0 = E[i[0]];
            T const& e1 = E[i[1]];
            T const& e2 = E[i[2]];
            T e0pc0 = e0 + c0, e0mc0 = e0 - c0, e1pc1 = e1 + c1, e1mc1 = e1 - c1;

            // Test whether the cylinder axis and canonical box intersect.
            T absC2 = std::fabs(c2);
            if (absC2 <= e2)
            {
                std::array<T, 2> negEmCDivD{ -e0pc0 / d0, -e1pc1 / d1 };
                std::array<T, 2> posEmCDivD{ e0mc0 / d0, e1mc1 / d1 };
                T lower = std::max(std::max(negEmCDivD[0], negEmCDivD[1]), -hDiv2);
                T upper = std::min(std::min(posEmCDivD[0], posEmCDivD[1]), hDiv2);
                if (lower <= upper)
                {
                    return true;
                }
            }

            // Compute the squared distance from the projected cylinder axis
            // (a point) to the projected convex polyhedron (a rectangle).
            T const zero = static_cast<T>(0);
            T sMin = zero, tHat = d1 * e1mc1 - d0 * e0pc0;
            if (-hDiv2 <= tHat)
            {
                if (tHat <= hDiv2)
                {
                    sMin = -(d0 * e1mc1 + d1 * e0pc0);
                }
                else  // tHat > +h/2
                {
                    sMin = -(e0pc0 + d0 * hDiv2) / d1;
                }
            }
            else  // tHat < -h/2
            {
                sMin = -(e1mc1 + d1 * hDiv2) / d0;
            }

            T sMax = zero, tBar = d0 * e0mc0 - d1 * e1pc1;
            if (-hDiv2 <= tBar)
            {
                if (tBar <= hDiv2)
                {
                    sMax = d0 * e1pc1 + d1 * e0mc0;
                }
                else  // tBar > +h/2
                {
                    sMax = (e1pc1 + d1 * hDiv2) / d0;
                }
            }
            else  // tBar < -h/2
            {
                sMax = (e0mc0 + d0 * hDiv2) / d1;
            }

            LogAssert(
                sMin < sMax,
                "The s-interval is invalid, which is unexpected.");

            T sqrDistance = zero;
            if (zero < sMin)
            {
                sqrDistance += sMin * sMin;
            }
            else if (sMax < zero)
            {
                sqrDistance += sMax * sMax;
            }

            T delta = absC2 - e2;
            if (delta > zero)
            {
                sqrDistance += delta * delta;
            }

            return sqrDistance <= r * r;
        }

        static bool DoQueryNoZeros(Vector3<T> const& C, Vector3<T> const& D,
            T const& r, T const& hDiv2, Vector3<T> const& E)
        {
            // Test whether the cylinder axis and canonical box intersect.
            std::array<T, 3> negEmCDivD
            {
                (-E[0] - C[0]) / D[0],
                (-E[1] - C[1]) / D[1],
                (-E[2] - C[2]) / D[2]
            };

            std::array<T, 3> posEmCDivD
            {
                (E[0] - C[0]) / D[0],
                (E[1] - C[1]) / D[1],
                (E[2] - C[2]) / D[2]
            };

            T max01 = std::max(negEmCDivD[0], negEmCDivD[1]);
            T max23 = std::max(negEmCDivD[2], -hDiv2);
            T lower = std::max(max01, max23);
            T min01 = std::min(posEmCDivD[0], posEmCDivD[1]);
            T min23 = std::min(posEmCDivD[2], hDiv2);
            T upper = std::min(min01, min23);
            if (lower <= upper)
            {
                return true;
            }

            // Compute t[i] = Dot(D, V[i] - C) for box vertices V[i]. These
            // are used in computing the intervals associated with extreme
            // edges.
            T dotDC = Dot(D, C);
            T d0e0 = D[0] * E[0], d1e1 = D[1] * E[1], d2e2 = D[2] * E[2];
            T t1 = +d0e0 - d1e1 - d2e2 - dotDC, s1p = t1 + hDiv2, s1n = t1 - hDiv2;
            T t2 = -d0e0 + d1e1 - d2e2 - dotDC, s2p = t2 + hDiv2, s2n = t2 - hDiv2;
            T t3 = +d0e0 + d1e1 - d2e2 - dotDC, s3p = t3 + hDiv2, s3n = t3 - hDiv2;
            T t4 = -d0e0 - d1e1 + d2e2 - dotDC, s4p = t4 + hDiv2, s4n = t4 - hDiv2;
            T t5 = +d0e0 - d1e1 + d2e2 - dotDC, s5p = t5 + hDiv2, s5n = t5 - hDiv2;
            T t6 = -d0e0 + d1e1 + d2e2 - dotDC, s6p = t6 + hDiv2, s6n = t6 - hDiv2;

            // Compute an orthonormal basis containing D.
            std::array<Vector3<T>, 3> basis{};
            basis[0] = D;
            ComputeOrthogonalComplement(1, basis.data());
            auto const& W0 = basis[1];
            auto const& W1 = basis[2];

            T const zero = static_cast<T>(0);
            T sqrRadius = r * r;
            T sqrDistance{};
            Vector3<T> P0{}, P1{};

            // (U0, -U1)
            lower = (s1p >= zero ? -E[2] : -E[2] - s1p / D[2]);
            upper = (s5n <= zero ? +E[2] : +E[2] - s5n / D[2]);
            if (lower <= upper)
            {
                P0 = { +E[0], -E[1], lower };
                P1 = { +E[0], -E[1], upper };
                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (U1, -U0)
            lower = (s2p >= zero ? -E[2] : -E[2] - s2p / D[2]);
            upper = (s6n <= zero ? +E[2] : +E[2] - s6n / D[2]);
            if (lower <= upper)
            {
                P0 = { -E[0], +E[1], lower };
                P1 = { -E[0], +E[1], upper };
                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (U0, -U2)
            lower = (s1p >= zero ? -E[1] : -E[1] - s1p / D[1]);
            upper = (s3n <= zero ? +E[1] : +E[1] - s3n / D[1]);
            if (lower <= upper)
            {
                P0 = { +E[0], lower, -E[2] };
                P1 = { +E[0], upper, -E[2] };
                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (U2, -U0)
            lower = (s4p >= zero ? -E[1] : -E[1] - s4p / D[1]);
            upper = (s6n <= zero ? +E[1] : +E[1] - s6n / D[1]);
            if (lower <= upper)
            {
                P0 = { -E[0], lower, +E[2] };
                P1 = { -E[0], upper, +E[2] };
                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (U1, -U2)
            lower = (s2p >= zero ? -E[0] : -E[0] - s2p / D[0]);
            upper = (s3n <= zero ? +E[0] : +E[0] - s3n / D[0]);
            if (lower <= upper)
            {
                P0 = { lower, +E[1], -E[2] };
                P1 = { upper, +E[1], -E[2] };
                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (U2, -U1)
            lower = (s4p >= zero ? -E[0] : -E[0] - s4p / D[0]);
            upper = (s5n <= zero ? +E[0] : +E[0] - s5n / D[0]);
            if (lower <= upper)
            {
                P0 = { lower, -E[1], +E[2] };
                P1 = { upper, -E[1], +E[2] };
                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (U0, -D)
            lower = (s3p >= zero ? -E[2] : -E[2] - s3p / D[2]);
            upper = (s5p <= zero ? +E[2] : +E[2] - s5p / D[2]);
            if (lower <= upper)
            {
                if (s3p >= zero)
                {
                    P0 = { +E[0], +E[1] - s3p / D[1], -E[2] };
                }
                else
                {
                    P0 = { +E[0], +E[1], -E[2] - s3p / D[2] };
                }

                if (s5p <= zero)
                {
                    P1 = { +E[0], -E[1] - s5p / D[1], +E[2] };
                }
                else
                {
                    P1 = { +E[0], -E[1], +E[2] - s5p / D[2] };
                }

                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (D, -U0)
            lower = (s2n >= zero ? -E[2] : -E[2] - s2n / D[2]);
            upper = (s4n <= zero ? +E[2] : +E[2] - s4n / D[2]);
            if (lower <= upper)
            {
                if (s2n >= zero)
                {
                    P0 = { -E[0], +E[1] - s2n / D[1], -E[2] };
                }
                else
                {
                    P0 = { -E[0], +E[1], -E[2] - s2n / D[2] };
                }

                if (s4n <= zero)
                {
                    P1 = { -E[0], -E[1] - s4n / D[1], +E[2] };
                }
                else
                {
                    P1 = { -E[0], -E[1], +E[2] - s4n / D[2] };
                }

                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (U1, -D)
            lower = (s6p >= zero ? -E[0] : -E[0] - s6p / D[0]);
            upper = (s3p <= zero ? +E[0] : +E[0] - s3p / D[0]);
            if (lower <= upper)
            {
                if (s6p >= zero)
                {
                    P0 = { -E[0], +E[1], +E[2] - s6p / D[2] };
                }
                else
                {
                    P0 = { -E[0] - s6p / D[0], +E[1], +E[2] };
                }

                if (s3p <= zero)
                {
                    P1 = { +E[0], +E[1], -E[2] - s3p / D[2] };
                }
                else
                {
                    P1 = { +E[0] - s3p / D[0], -E[1], -E[2] };
                }

                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (D, -U1)
            lower = (s4n >= zero ? -E[0] : -E[0] - s4n / D[0]);
            upper = (s1n <= zero ? +E[0] : +E[0] - s1n / D[0]);
            if (lower <= upper)
            {
                if (s4n >= zero)
                {
                    P0 = { -E[0], -E[1], +E[2] - s4n / D[2] };
                }
                else
                {
                    P0 = { -E[0] - s4n / D[0], -E[1], +E[2] };
                }

                if (s1n <= zero)
                {
                    P1 = { +E[0], -E[1], -E[2] - s1n / D[2] };
                }
                else
                {
                    P1 = { +E[0] - s1n / D[0], -E[1], -E[2] };
                }

                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (U2, -D)
            lower = (s5p >= zero ? -E[1] : -E[1] - s5p / D[2]);
            upper = (s6p <= zero ? +E[1] : +E[1] - s6p / D[2]);
            if (lower <= upper)
            {
                if (s5p >= zero)
                {
                    P0 = { +E[0] - s5p / D[0], -E[1], +E[2] };
                }
                else
                {
                    P0 = { +E[0], -E[1] - s5p / D[1], +E[2] };
                }

                if (s6p <= zero)
                {
                    P1 = { -E[0] - s6p / D[0], +E[1], +E[2] };
                }
                else
                {
                    P1 = { -E[0], E[1] - s6p / D[1], +E[2] };
                }

                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            // (D, -U2)
            lower = (s1n >= zero ? -E[1] : -E[1] - s1n / D[2]);
            upper = (s2n <= zero ? +E[1] : +E[1] - s2n / D[2]);
            if (lower <= upper)
            {
                if (s1n >= zero)
                {
                    P0 = { +E[0] - s1n / D[0], -E[1], -E[2] };
                }
                else
                {
                    P0 = { +E[0], -E[1] - s1n / D[1], -E[2] };
                }

                if (s2n <= zero)
                {
                    P1 = { -E[0] - s2n / D[0], +E[1], -E[2] };
                }
                else
                {
                    P1 = { -E[0], E[1] - s2n / D[1], -E[2] };
                }

                sqrDistance = ComputeSqrDistance(P0, P1, C, W0, W1);
                if (sqrDistance <= sqrRadius)
                {
                    return true;
                }
            }

            return false;
        }
    };
}
