// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the triangles to be solids.
//
// The test-intersection query (TIQuery) uses the method of separating axes
// to determine whether or not the triangles intersect. See
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
// Section 5 describes the finite set of potential separating axes.
//
// The find-intersection query (FIQuery) determines how the two triangles
// are positioned and oriented to each other. The algorithm uses the sign of
// the projections of the vertices of triangle1 onto a normal line that is
// perpendicular to the plane of triangle0. The table of possibilities is
// listed next with n = numNegative, p = numPositive and z = numZero.
//
//   n p z  intersection
//   ------------------------------------
//   0 3 0  none
//   0 2 1  vertex
//   0 1 2  edge
//   0 0 3  coplanar triangles or a triangle is degenerate
//   1 2 0  segment (2 edges clipped)
//   1 1 1  segment (1 edge clipped)
//   1 0 2  edge
//   2 1 0  segment (2 edges clipped)
//   2 0 1  vertex
//   3 0 0  none

#include <Mathematics/TIQuery.h>
#include <Mathematics/FIQuery.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/IntrTriangle2Triangle2.h>
#include <Mathematics/IntrSegment2Triangle2.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace gte
{
    template <typename T>
    class TIQuery<T, Triangle3<T>, Triangle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                contactTime(static_cast<T>(0))
            {
            }

            // The contact time is 0 for stationary triangles. It is
            // nonnegative for moving triangles.
            bool intersect;
            T contactTime;
        };

        // The query is for stationary triangles.
        Result operator()(Triangle3<T> const& inTriangle0, Triangle3<T> const& inTriangle1)
        {
            Result result{};

            // Translate the triangles so that triangle0.v[0] becomes (0,0,0).
            T const zero = static_cast<T>(0);
            Vector3<T> const& origin = inTriangle0.v[0];
            Triangle3<T> triangle0(
                Vector3<T>{ zero, zero, zero },
                inTriangle0.v[1] - origin,
                inTriangle0.v[2] - origin);

            Triangle3<T> triangle1(
                inTriangle1.v[0] - origin,
                inTriangle1.v[1] - origin,
                inTriangle1.v[2] - origin);

            // Get edge directions and a normal vector for triangle0.
            std::array<Vector3<T>, 3> E0
            {
                triangle0.v[1] - triangle0.v[0],
                triangle0.v[2] - triangle0.v[1],
                triangle0.v[0] - triangle0.v[2]
            };
            Vector3<T> N0 = Cross(E0[0], E0[1]);

            // Scale-project triangle1 onto the normal line of triangle0 and
            // test for separation. The translation performed initially
            // ensures that triangle0 projects onto its normal line at t = 0.
            std::array<T, 2> tExtreme0{ zero, zero };
            std::array<T, 2> tExtreme1 = ScaleProjectOntoLine(triangle1, N0);
            if (tExtreme0[1] < tExtreme1[0] || tExtreme1[1] < tExtreme0[0])
            {
                return result;
            }

            // Get edge directions and a normal vector for triangle1.
            std::array<Vector3<T>, 3> E1
            {
                triangle1.v[1] - triangle1.v[0],
                triangle1.v[2] - triangle1.v[1],
                triangle1.v[0] - triangle1.v[2]
            };
            Vector3<T> N1 = Cross(E1[0], E1[1]);

            // Scale-project triangle0 onto the normal line of triangle1 and
            // test for separation.
            T const projT0V0 = Dot(N1, triangle1.v[0] - triangle0.v[0]);
            tExtreme0 = { projT0V0, projT0V0 };
            tExtreme1 = ScaleProjectOntoLine(triangle0, N1);
            if (tExtreme0[1] < tExtreme1[0] || tExtreme1[1] < tExtreme0[0])
            {
                return result;
            }

            // At this time, neither normal line is a separation axis for the
            // triangles. If Cross(N0,N1) != (0,0,0), the planes of the
            // triangles are not parallel and must intersect in a line. If
            // Cross(N0,N1) = (0,0,0), the planes are parallel. In fact they
            // are coplanar; for if they were not coplanar, one of the two
            // previous separating axis tests would have determined this and
            // returned from the function call.

            // The potential separating axes are origin+t*direction, where
            // origin is inTriangle.v[0]. In the translated configuration,
            // the potential separating axes are t*direction.
            Vector3<T> direction{ zero, zero, zero };

            Vector3<T> N0xN1 = Cross(N0, N1);
            T sqrLengthN0xN1 = Dot(N0xN1, N0xN1);
            if (sqrLengthN0xN1 > zero)
            {
                // The triangles are not parallel. Test for separation by
                // using directions that are cross products of a pair of
                // triangle edges, one edge from triangle0 and one edge from
                // triangle1.
                for (size_t i1 = 0; i1 < 3; ++i1)
                {
                    for (size_t i0 = 0; i0 < 3; ++i0)
                    {
                        direction = Cross(E0[i0], E1[i1]);
                        tExtreme0 = ScaleProjectOntoLine(triangle0, direction);
                        tExtreme1 = ScaleProjectOntoLine(triangle1, direction);
                        if (tExtreme0[1] < tExtreme1[0] || tExtreme1[1] < tExtreme0[0])
                        {
                            return result;
                        }
                    }
                }
            }
            else
            {
                // The triangles are coplanar. Test for separation by using
                // directions that are cross products of a pair of vectors,
                // one vector a normal of a triangle and the other vector an
                // edge from the other triangle.

                // Directions N0xE0[i0].
                for (size_t i0 = 0; i0 < 3; ++i0)
                {
                    direction = Cross(N0, E0[i0]);
                    tExtreme0 = ScaleProjectOntoLine(triangle0, direction);
                    tExtreme1 = ScaleProjectOntoLine(triangle1, direction);
                    if (tExtreme0[1] < tExtreme1[0] || tExtreme1[1] < tExtreme0[0])
                    {
                        return result;
                    }
                }

                // Directions N1xE1[i1].
                for (size_t i1 = 0; i1 < 3; ++i1)
                {
                    direction = Cross(N1, E1[i1]);
                    tExtreme0 = ScaleProjectOntoLine(triangle0, direction);
                    tExtreme1 = ScaleProjectOntoLine(triangle1, direction);
                    if (tExtreme0[1] < tExtreme1[0] || tExtreme1[1] < tExtreme0[0])
                    {
                        return result;
                    }
                }
            }

            result.intersect = true;
            return result;
        }

        // The query is for triangles moving with constant linear velocity
        // during the time interval [0,tMax].
        Result operator()(T const& tMax,
            Triangle3<T> const& inTriangle0, Vector3<T> const& velocity0,
            Triangle3<T> const& inTriangle1, Vector3<T> const& velocity1)
        {
            Result result{};

            // The query determines the interval [tFirst,tLast] over which
            // the triangle are intersecting. Start with time interval
            // [0,+infinity).
            T tFirst = static_cast<T>(0);
            T tLast = std::numeric_limits<T>::max();

            // Compute the velocity of inTriangle1 relative to inTriangle0.
            Vector3<T> relVelocity = velocity1 - velocity0;

            // Translate the triangles so that triangle0.v[0] becomes (0,0,0).
            T const zero = static_cast<T>(0);
            Vector3<T> const& origin = inTriangle0.v[0];
            Triangle3<T> triangle0(
                Vector3<T>{ zero, zero, zero },
                inTriangle0.v[1] - origin,
                inTriangle0.v[2] - origin);

            Triangle3<T> triangle1(
                inTriangle1.v[0] - origin,
                inTriangle1.v[1] - origin,
                inTriangle1.v[2] - origin);

            // Get edge directions and a normal vector for triangle0.
            std::array<Vector3<T>, 3> E0
            {
                triangle0.v[1] - triangle0.v[0],
                triangle0.v[2] - triangle0.v[1],
                triangle0.v[0] - triangle0.v[2]
            };
            Vector3<T> N0 = Cross(E0[0], E0[1]);

            // Test for overlap using the separating axis test in the N0
            // direction.
            if (!TestOverlap(triangle0, triangle1, N0, tMax, relVelocity,
                tFirst, tLast))
            {
                return result;
            }

            // Get edge directions and a normal vector for triangle1.
            std::array<Vector3<T>, 3> E1
            {
                triangle1.v[1] - triangle1.v[0],
                triangle1.v[2] - triangle1.v[1],
                triangle1.v[0] - triangle1.v[2]
            };
            Vector3<T> N1 = Cross(E1[0], E1[1]);

            if (std::fabs(Dot(N0, N1)) < static_cast<T>(1))
            {
                // The triangles are not parallel.

                // Test for overlap using the separating axis test in the
                // N1 direction.
                if (!TestOverlap(triangle0, triangle1, N1, tMax,
                    relVelocity, tFirst, tLast))
                {
                    return result;
                }

                // Test for overlap using the separating axis test in the
                // directions E0[i0]xE1[i1].
                for (size_t i1 = 0; i1 < 3; ++i1)
                {
                    for (size_t i0 = 0; i0 < 3; ++i0)
                    {
                        Vector3<T> direction = UnitCross(E0[i0], E1[i1]);
                        if (!TestOverlap(triangle0, triangle1, direction,
                            tMax, relVelocity, tFirst, tLast))
                        {
                            return result;
                        }
                    }
                }
            }
            else
            {
                // The triangles are coplanar.

                // Test for overlap using the separating axis test in the
                // directions N0xE0[i0].
                for (size_t i0 = 0; i0 < 3; ++i0)
                {
                    Vector3<T> direction = UnitCross(N0, E0[i0]);
                    if (!TestOverlap(triangle0, triangle1, direction,
                        tMax, relVelocity, tFirst, tLast))
                    {
                        return result;
                    }
                }

                // Test for overlap using the separating axis test in the
                // directions N1xE1[i1].
                for (size_t i1 = 0; i1 < 3; ++i1)
                {
                    Vector3<T> direction = UnitCross(N1, E1[i1]);
                    if (!TestOverlap(triangle0, triangle1, direction,
                        tMax, relVelocity, tFirst, tLast))
                    {
                        return result;
                    }
                }
            }

            result.intersect = true;
            result.contactTime = tFirst;
            return result;
        }

    private:
        // The triangle is <V[0],V[1],V[2]>. The line is t*direction, where
        // the origin is (0,0,0) and the 'direction' is not zero but not
        // necessarily unit length. The projections of the triangle vertices
        // onto the line are t[i] = Dot(direction, V[i]). Return the extremes
        // tmin = min(t[0],t[1],t[2]) and tmax = max(t[0],t[1],t[2]) as
        // elements of a std::array<T,2>.
        static std::array<T, 2> ScaleProjectOntoLine(Triangle3<T> const& triangle,
            Vector3<T> const& direction)
        {
            T t = Dot(direction, triangle.v[0]);
            std::array<T, 2> tExtreme{ t, t };
            for (size_t i = 1; i < 3; ++i)
            {
                t = Dot(direction, triangle.v[i]);
                if (t < tExtreme[0])
                {
                    tExtreme[0] = t;
                }
                else if (t > tExtreme[1])
                {
                    tExtreme[1] = t;
                }
            }
            return tExtreme;
        }

        // This is the constant velocity separating axis test.
        static bool TestOverlap(T const& tMax, T const& speed,
            std::array<T, 2> const& extreme0, std::array<T, 2> const& extreme1,
            T& tFirst, T& tLast)
        {
            T const zero = static_cast<T>(0);

            if (extreme1[1] < extreme0[0])
            {
                // The interval extreme1 is on the left of the interval
                // extreme0.
                if (speed <= zero)
                {
                    // The interval extreme1 is moving away from the interval
                    // extreme0.
                    return false;
                }

                // Compute the first time of contact on this axis.
                T t = (extreme0[0] - extreme1[1]) / speed;
                if (t > tFirst)
                {
                    tFirst = t;
                }

                if (tFirst > tMax)
                {
                    // The intersection occurs after the specified maximum
                    // time.
                    return false;
                }

                // Compute the last time of contact on this axis.
                t = (extreme0[1] - extreme1[0]) / speed;
                if (t < tLast)
                {
                    tLast = t;
                }

                if (tFirst > tLast)
                {
                    // The time interval is invalid, so the objects are not
                    // intersecting.
                    return false;
                }
            }
            else if (extreme0[1] < extreme1[0])
            {
                // The interval extreme1 is on the right of the interval
                // extreme0.
                if (speed >= zero)
                {
                    // The interval extreme1 is moving away from the interval
                    // extreme0.
                    return false;
                }

                // Compute the first time of contact on this axis.
                T t = (extreme0[1] - extreme1[0]) / speed;
                if (t > tFirst)
                {
                    tFirst = t;
                }

                if (tFirst > tMax)
                {
                    // The intersection occurs after the specified maximum
                    // time.
                    return false;
                }

                // Compute the last time of contact on this axis.
                t = (extreme0[0] - extreme1[1]) / speed;
                if (t < tLast)
                {
                    tLast = t;
                }

                if (tFirst > tLast)
                {
                    // The time interval is invalid, so the objects are not
                    // intersecting.
                    return false;
                }

            }
            else
            {
                // The intervals extreme0 and extreme1 are currently
                // overlapping.
                if (speed > zero)
                {
                    // Compute the last time of contact on this axis.
                    T t = (extreme0[1] - extreme1[0]) / speed;
                    if (t < tLast)
                    {
                        tLast = t;
                    }

                    if (tFirst > tLast)
                    {
                        // The time interval is invalid, so the objects are
                        // not intersecting.
                        return false;
                    }
                }
                else if (speed < zero)
                {
                    // Compute the last time of contact on this axis.
                    T t = (extreme0[0] - extreme1[1]) / speed;
                    if (t < tLast)
                    {
                        tLast = t;
                    }

                    if (tFirst > tLast)
                    {
                        // The time interval is invalid, so the objects are
                        // not intersecting.
                        return false;
                    }
                }
            }
            return true;
        }

        // A projection wrapper to set up for the separating axis test.
        static bool TestOverlap(Triangle3<T> const& triangle0,
            Triangle3<T> const& triangle1, Vector3<T> const& direction,
            T const& tMax, Vector3<T> const& velocity, T& tFirst, T& tLast)
        {
            auto extreme0 = ScaleProjectOntoLine(triangle0, direction);
            auto extreme1 = ScaleProjectOntoLine(triangle1, direction);
            T speed = Dot(direction, velocity);
            return TestOverlap(tMax, speed, extreme0, extreme1, tFirst, tLast);
        }
    };

    template <typename T>
    class FIQuery<T, Triangle3<T>, Triangle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                contactTime(static_cast<T>(0)),
                intersection{}
            {
            }

            // The contact time is 0 for stationary triangles. It is
            // nonnegative for moving triangles.
            bool intersect;
            T contactTime;
            std::vector<Vector3<T>> intersection;
        };

        // The query is for stationary triangles.
        Result operator()(Triangle3<T> const& inTriangle0, Triangle3<T> const& inTriangle1)
        {
            Result result{};

            // Translate the triangles so that triangle0.v[0] becomes (0,0,0).
            T const zero = static_cast<T>(0);
            Vector3<T> const& origin = inTriangle0.v[0];
            Triangle3<T> triangle0(
                Vector3<T>{ zero, zero, zero },
                inTriangle0.v[1] - origin,
                inTriangle0.v[2] - origin);

            Triangle3<T> triangle1(
                inTriangle1.v[0] - origin,
                inTriangle1.v[1] - origin,
                inTriangle1.v[2] - origin);

            // Compute a normal vector for the plane containing triangle0.
            Vector3<T> normal = Cross(triangle0.v[1], triangle0.v[2]);

            // Determine where the vertices of triangle1 live relative to the
            // plane of triangle0. The 'distance' values are actually signed
            // and scaled distances, the latter because 'normal' is not
            // necessarily unit length.
            size_t numPositive = 0, numNegative = 0, numZero = 0;
            std::array<T, 3> distance{};
            std::array<int32_t, 3> sign{};
            for (size_t i = 0; i < 3; ++i)
            {
                distance[i] = Dot(normal, triangle1.v[i]);
                if (distance[i] > zero)
                {
                    sign[i] = 1;
                    ++numPositive;
                }
                else if (distance[i] < zero)
                {
                    sign[i] = -1;
                    ++numNegative;
                }
                else
                {
                    sign[i] = 0;
                    ++numZero;
                }
            }

            if (numZero == 0 )
            {
                if (numPositive > 0 && numNegative > 0)
                {
                    // (n,p,z) is (1,2,0) or (2,1,0).
                    int32_t signCompare = (numPositive == 1 ? 1 : -1);
                    for (size_t i0 = 1, i1 = 2, i2 = 0; i2 < 3; i0 = i1, i1 = i2++)
                    {
                        if (sign[i2] == signCompare)
                        {
                            Segment3<T> segment{};
                            Vector3<T> const& Vi2 = triangle1.v[i2];
                            T t0 = distance[i2] / (distance[i2] - distance[i0]);
                            Vector3<T> diffVi0Vi2 = triangle1.v[i0] - triangle1.v[i2];
                            segment.p[0] = Vi2 + t0 * diffVi0Vi2;
                            Vector3<T> diffVi1Vi2 = triangle1.v[i1] - triangle1.v[i2];
                            T t1 = distance[i2] / (distance[i2] - distance[i1]);
                            segment.p[1] = Vi2 + t1 * diffVi1Vi2;
                            IntersectsSegment(normal, triangle0, segment, result);
                            break;
                        }
                    }
                }
                // else: (n,p,z) is (0,3,0) or (3,0,0) and triangle1 is
                // strictly on one side of the plane of triangle0, so no
                // intersection.
            }
            else if (numZero == 1)
            {
                if (numPositive == 1)
                {
                    // (n,p,z) is (1,1,1). A single vertex of triangle1 is
                    // in the plane of triangle0 and the opposing edge of
                    // triangle1 intersects the plane transversely.
                    for (size_t i0 = 1, i1 = 2, i2 = 0; i2 < 3; i0 = i1, i1 = i2++)
                    {
                        if (sign[i2] == 0)
                        {
                            Segment3<T> segment{};
                            segment.p[0] = triangle1.v[i2];
                            Vector3<T> const& Vi1 = triangle1.v[i1];
                            T t = distance[i1] / (distance[i1] - distance[i0]);
                            Vector3<T> diffVi0Vi1 = triangle1.v[i0] - triangle1.v[i1];
                            segment.p[1] = Vi1 + t * diffVi0Vi1;
                            IntersectsSegment(normal, triangle0, segment, result);
                            break;
                        }
                    }
                }
                else
                {
                    // (n,p,z) is (2,0,1) or (0,2,1). A single vertex of
                    // triangle1 is in the plane of triangle0.
                    for (size_t i = 0; i < 3; ++i)
                    {
                        if (sign[i] == 0)
                        {
                            ContainsPoint(normal, triangle0, triangle1.v[i], result);
                            break;
                        }
                    }
                }
            }
            else if (numZero == 2)
            {
                // (n,p,z) is (0,1,2) or (1,0,2). Two vertices are on the
                // plane of triangle0, so the segment connecting the vertices
                // is on the plane.
                for (size_t i0 = 1, i1 = 2, i2 = 0; i2 < 3; i0 = i1, i1 = i2++)
                {
                    if (sign[i2] != 0)
                    {
                        Segment3<T> segment(triangle1.v[i0], triangle1.v[i1]);
                        IntersectsSegment(normal, triangle0, segment, result);
                        break;
                    }
                }
            }
            else // numZero == 3
            {
                // (n,p,z) is (0,0,3). Triangle1 is contained in the plane of
                // triangle0.
                GetCoplanarIntersection(normal, triangle0, triangle1, result);
            }

            if (result.intersect)
            {
                // Translate the intersection set back to the original
                // coordinate system.
                for (auto& point : result.intersection)
                {
                    point += origin;
                }
            }
            return result;
        }

        // The query is for triangles moving with constant linear velocity
        // during the time interval [0,tMax].
        Result operator()(T const& tMax,
            Triangle3<T> const& inTriangle0, Vector3<T> const& velocity0,
            Triangle3<T> const& inTriangle1, Vector3<T> const& velocity1)
        {
            Result result{};

            // The query determines the interval [tFirst,tLast] over which
            // the triangle are intersecting. Start with time interval
            // [0,+infinity).
            T tFirst = static_cast<T>(0);
            T tLast = std::numeric_limits<T>::max();

            // Compute the velocity of inTriangle1 relative to inTriangle0.
            Vector3<T> relVelocity = velocity1 - velocity0;

            // Translate the triangles so that triangle0.v[0] becomes (0,0,0).
            T const zero = static_cast<T>(0);
            Vector3<T> const& origin = inTriangle0.v[0];
            Triangle3<T> triangle0(
                Vector3<T>{ zero, zero, zero },
                inTriangle0.v[1] - origin,
                inTriangle0.v[2] - origin);

            Triangle3<T> triangle1(
                inTriangle1.v[0] - origin,
                inTriangle1.v[1] - origin,
                inTriangle1.v[2] - origin);

            // Get edge directions and a normal vector for triangle0.
            std::array<Vector3<T>, 3> E0
            {
                triangle0.v[1] - triangle0.v[0],
                triangle0.v[2] - triangle0.v[1],
                triangle0.v[0] - triangle0.v[2]
            };
            Vector3<T> N0 = Cross(E0[0], E0[1]);

            // Find overlap using the separating axis test in the N0
            // direction.
            ContactSide side = ContactSide::CS_NONE;
            Configuration tcfg0{}, tcfg1{};
            if (!FindOverlap(triangle0, triangle1, N0, tMax, relVelocity,
                side, tcfg0, tcfg1, tFirst, tLast))
            {
                return result;
            }

            // Get edge directions and a normal vector for triangle1.
            std::array<Vector3<T>, 3> E1
            {
                triangle1.v[1] - triangle1.v[0],
                triangle1.v[2] - triangle1.v[1],
                triangle1.v[0] - triangle1.v[2]
            };
            Vector3<T> N1 = Cross(E1[0], E1[1]);

            if (std::fabs(Dot(N0, N1)) < static_cast<T>(1))
            {
                // The triangles are not parallel.

                // Test for overlap using the separating axis test in the
                // N1 direction.
                if (!FindOverlap(triangle0, triangle1, N1, tMax,
                    relVelocity, side, tcfg0, tcfg1, tFirst, tLast))
                {
                    return result;
                }

                // Test for overlap using the separating axis test in the
                // directions E0[i0]xE1[i1].
                for (size_t i1 = 0; i1 < 3; ++i1)
                {
                    for (size_t i0 = 0; i0 < 3; ++i0)
                    {
                        Vector3<T> direction = UnitCross(E0[i0], E1[i1]);
                        if (!FindOverlap(triangle0, triangle1, direction,
                            tMax, relVelocity, side, tcfg0, tcfg1, tFirst, tLast))
                        {
                            return result;
                        }
                    }
                }
            }
            else
            {
                // The triangles are coplanar.

                // Test for overlap using the separating axis test in the
                // directions N0xE0[i0].
                for (size_t i0 = 0; i0 < 3; ++i0)
                {
                    Vector3<T> direction = UnitCross(N0, E0[i0]);
                    if (!FindOverlap(triangle0, triangle1, direction,
                        tMax, relVelocity, side, tcfg0, tcfg1, tFirst, tLast))
                    {
                        return result;
                    }
                }

                // Test for overlap using the separating axis test in the
                // directions N1xE1[i1].
                for (size_t i1 = 0; i1 < 3; ++i1)
                {
                    Vector3<T> direction = UnitCross(N1, E1[i1]);
                    if (!FindOverlap(triangle0, triangle1, direction,
                        tMax, relVelocity, side, tcfg0, tcfg1, tFirst, tLast))
                    {
                        return result;
                    }
                }
            }

            // Move the triangles to the first contact before computing
            // the contact set.
            for (size_t i = 0; i < 3; ++i)
            {
                triangle0.v[i] = inTriangle0.v[i] + tFirst * velocity0;
                triangle1.v[i] = inTriangle1.v[i] + tFirst * velocity1;
            }

            auto stationary = this->operator()(triangle0, triangle1);
            result.intersect = true;
            result.contactTime = tFirst;
            result.intersection = std::move(stationary.intersection);
            return result;
        }

    private:
        // Compute the point, segment or polygon of intersection of coplanar
        // triangles. The intersection is computed by projecting the triangles
        // onto the plane and using a find-intersection query for two
        // triangles in 2D. The intersection can be empty.
        static void GetCoplanarIntersection(Vector3<T> const& normal,
            Triangle3<T> const& triangle0, Triangle3<T> const& triangle1,
            Result& result)
        {
            // Project the triangles onto the coordinate plane most aligned
            // with the plane normal.
            int32_t maxIndex = 0;
            T cmax = std::fabs(normal[0]);
            T cvalue = std::fabs(normal[1]);
            if (cvalue > cmax)
            {
                maxIndex = 1;
                cmax = cvalue;
            }
            cvalue = std::fabs(normal[2]);
            if (cvalue > cmax)
            {
                maxIndex = 2;
            }

            std::array<int32_t, 3> lookup{};
            if (maxIndex == 0)
            {
                // Project onto the yz-plane.
                lookup = { 1, 2, 0 };
            }
            else if (maxIndex == 1)
            {
                // Project onto the xz-plane.
                lookup = { 0, 2, 1 };
            }
            else // maxIndex = 2
            {
                // Project onto the xy-plane.
                lookup = { 0, 1, 2 };
            }

            Triangle2<T> projTriangle0{}, projTriangle1{};
            for (size_t i = 0; i < 3; ++i)
            {
                projTriangle0.v[i][0] = triangle0.v[i][lookup[0]];
                projTriangle0.v[i][1] = triangle0.v[i][lookup[1]];
                projTriangle1.v[i][0] = triangle1.v[i][lookup[0]];
                projTriangle1.v[i][1] = triangle1.v[i][lookup[1]];
            }

            // 2D triangle intersection queries require counterclockwise
            // ordering of vertices.
            T const zero = static_cast<T>(0);
            if (normal[maxIndex] < zero)
            {
                // Triangle0 is clockwise; reorder it.
                std::swap(projTriangle0.v[1], projTriangle0.v[2]);
            }

            Vector2<T> edge0 = projTriangle1.v[1] - projTriangle1.v[0];
            Vector2<T> edge1 = projTriangle1.v[2] - projTriangle1.v[0];
            if (DotPerp(edge0, edge1) < zero)
            {
                // Triangle1 is clockwise; reorder it.
                std::swap(projTriangle1.v[1], projTriangle1.v[2]);
            }

            FIQuery<T, Triangle2<T>, Triangle2<T>> ttQuery{};
            auto ttResult = ttQuery(projTriangle0, projTriangle1);
            size_t const numVertices = ttResult.intersection.size();
            if (numVertices == 0)
            {
                result.intersect = false;
                result.intersection.clear();
                return;
            }

            // Lift the 2D polygon of intersection to the 3D triangle space.
            result.intersect = true;
            result.intersection.resize(numVertices);
            auto const& src = ttResult.intersection;
            auto& trg = result.intersection;
            for (size_t i = 0; i < trg.size(); ++i)
            {
                trg[i][lookup[0]] = src[i][0];
                trg[i][lookup[1]] = src[i][1];
                trg[i][lookup[2]] = -(normal[lookup[0]] * trg[i][lookup[0]] +
                    normal[lookup[1]] * trg[i][lookup[1]]) / normal[lookup[2]];
            }
        }

        // Compute the point or segment of intersection of the 'triangle' with
        // 'normal' vector. The input segment is an edge of the other triangle.
        // The intersection can be empty.
        static void IntersectsSegment(Vector3<T> const& normal,
            Triangle3<T> const& triangle, Segment3<T> const& segment, Result& result)
        {
            // Project the triangle and segment onto the coordinate plane most
            // aligned with the plane normal.
            int32_t maxIndex = 0;
            T cmax = std::fabs(normal[0]);
            T cvalue = std::fabs(normal[1]);
            if (cvalue > cmax)
            {
                maxIndex = 1;
                cmax = cvalue;
            }
            cvalue = std::fabs(normal[2]);
            if (cvalue > cmax)
            {
                maxIndex = 2;
            }

            std::array<int32_t, 3> lookup{};
            if (maxIndex == 0)
            {
                // Project onto the yz-plane.
                lookup = { 1, 2, 0 };
            }
            else if (maxIndex == 1)
            {
                // Project onto the xz-plane.
                lookup = { 0, 2, 1 };
            }
            else // maxIndex = 2
            {
                // Project onto the xy-plane.
                lookup = { 0, 1, 2 };
            }

            Triangle2<T> projTriangle{};
            for (size_t i = 0; i < 3; ++i)
            {
                projTriangle.v[i][0] = triangle.v[i][lookup[0]];
                projTriangle.v[i][1] = triangle.v[i][lookup[1]];
            }

            Segment2<T> projSegment{};
            for (size_t i = 0; i < 2; ++i)
            {
                projSegment.p[i][0] = segment.p[i][lookup[0]];
                projSegment.p[i][1] = segment.p[i][lookup[1]];
            }

            // Compute the intersection with the coincident edge and the
            // triangle.
            FIQuery<T, Segment2<T>, Triangle2<T>> stQuery{};
            auto stResult = stQuery(projSegment, projTriangle);
            if (stResult.intersect)
            {
                result.intersect = true;
                result.intersection.resize(stResult.numIntersections);

                // Lift the 2D intersection points to the 3D triangle space.
                auto const& src = stResult.point;
                auto& trg = result.intersection;
                for (size_t i = 0; i < trg.size(); ++i)
                {
                    trg[i][lookup[0]] = src[i][0];
                    trg[i][lookup[1]] = src[i][1];
                    trg[i][lookup[2]] = -(normal[lookup[0]] * trg[i][lookup[0]] +
                        normal[lookup[1]] * trg[i][lookup[1]]) / normal[lookup[2]];
                }
            }
        }
        
        // Determine whether the point is inside or strictly outside the
        // triangle.
        static void ContainsPoint(Vector3<T> const& normal,
            Triangle3<T> const& triangle, Vector3<T> const& point, Result& result)
        {
            // Project the triangle and point onto the coordinate plane most
            // aligned with the plane normal.
            int32_t maxIndex = 0;
            T cmax = std::fabs(normal[0]);
            T cvalue = std::fabs(normal[1]);
            if (cvalue > cmax)
            {
                maxIndex = 1;
                cmax = cvalue;
            }
            cvalue = std::fabs(normal[2]);
            if (cvalue > cmax)
            {
                maxIndex = 2;
            }

            std::array<int32_t, 3> lookup{};
            if (maxIndex == 0)
            {
                // Project onto the yz-plane.
                lookup = { 1, 2, 0 };
            }
            else if (maxIndex == 1)
            {
                // Project onto the xz-plane.
                lookup = { 0, 2, 1 };
            }
            else // maxIndex = 2
            {
                // Project onto the xy-plane.
                lookup = { 0, 1, 2 };
            }

            Triangle2<T> projTriangle{};
            for (size_t i = 0; i < 3; ++i)
            {
                projTriangle.v[i][0] = triangle.v[i][lookup[0]];
                projTriangle.v[i][1] = triangle.v[i][lookup[1]];
            }

            Vector2<T> projPoint{ point[lookup[0]], point[lookup[1]] };

            // Determine whether the point is inside or strictly outside the
            // triangle. The triangle is counterclockwise ordered when sign
            // is +1 or clockwise ordered when sign is -1.
            T const zero = static_cast<T>(0);
            T const sign = (normal[maxIndex] > zero ? static_cast<T>(1) : static_cast<T>(-1));
            for (size_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                Vector2<T> diffPV0 = projPoint - projTriangle.v[i0];
                Vector2<T> diffV1V0 = projTriangle.v[i1] - projTriangle.v[i0];
                if (sign * DotPerp(diffPV0, diffV1V0) > zero)
                {
                    // The point is strictly outside edge <V[i0],V[i1]>.
                    result.intersect = false;
                    result.intersection.clear();
                    return;
                }
            }

            // Lift the 2D point of intersection to the 3D triangle space.
            result.intersect = true;
            result.intersection.resize(1);
            auto const& src = projPoint;
            auto& trg = result.intersection[0];
            trg[lookup[0]] = src[0];
            trg[lookup[1]] = src[1];
            trg[lookup[2]] = -(normal[lookup[0]] * trg[lookup[0]] +
                normal[lookup[1]] * trg[lookup[1]]) / normal[lookup[2]];
        }


        // Support for the query for moving triangles.
        enum class ProjectionMap
        {
            // Initial value for construction of Configuration.
            PM_INVALID,

            // 3 vertices project to the same point (min = max).
            PM_3,

            // 2 vertices project to a point (min) and 1 vertex projects to
            // a point (max).
            PM_21,

            // 1 vertex projects to a point (min) and 2 vertices project to
            // a point (max).
            PM_12,

            // 1 vertex projects to a point (min), 1 vertex projects to a
            // point (max) and 1 vertex projects to a point strictly between
            // the min and max points.
            PM_111
        };

        struct Configuration
        {
            Configuration()
                :
                map(ProjectionMap::PM_INVALID),
                index{ 0, 0, 0, 0, 0, 0, 0, 0 },
                min(static_cast<T>(0)),
                max(static_cast<T>(0))
            {
            }

            // This is how the vertices map to the projection interval.
            ProjectionMap map;

            // The sorted indices of the vertices.
            std::array<int32_t, 8> index;

            // The projection interval [min,max].
            T min, max;
        };

        enum class ContactSide
        {
            CS_LEFT,
            CS_RIGHT,
            CS_NONE
        };

        // The triangle is <V[0],V[1],V[2]>. The line is t*direction, where
        // the origin is (0,0,0) and the 'direction' is not zero but not
        // necessarily unit length. The projections of the triangle vertices
        // onto the line are t[i] = Dot(direction, V[i]). Return the
        // configuration of the triangle that lead to the extreme interval.
        static void ScaleProjectOntoLine(Triangle3<T> const& triangle,
            Vector3<T> const& direction, Configuration& cfg)
        {
            // Find the projections of the triangle vertices onto the
            // potential separating axis.
            T d0 = Dot(direction, triangle.v[0]);
            T d1 = Dot(direction, triangle.v[1]);
            T d2 = Dot(direction, triangle.v[2]);

            // Explicit sort of vertices to construct a Configuration object.
            if (d0 <= d1)
            {
                if (d1 <= d2) // d0 <= d1 <= d2
                {
                    if (d0 != d1)
                    {
                        if (d1 != d2)
                        {
                            cfg.map = ProjectionMap::PM_111;
                        }
                        else
                        {
                            cfg.map = ProjectionMap::PM_12;
                        }
                    }
                    else // d0 = d1
                    {
                        if (d1 != d2)
                        {
                            cfg.map = ProjectionMap::PM_21;
                        }
                        else
                        {
                            cfg.map = ProjectionMap::PM_3;
                        }
                    }
                    cfg.index[0] = 0;
                    cfg.index[1] = 1;
                    cfg.index[2] = 2;
                    cfg.min = d0;
                    cfg.max = d2;
                }
                else if (d0 <= d2) // d0 <= d2 < d1
                {
                    if (d0 != d2)
                    {
                        cfg.map = ProjectionMap::PM_111;
                        cfg.index[0] = 0;
                        cfg.index[1] = 2;
                        cfg.index[2] = 1;
                    }
                    else
                    {
                        cfg.map = ProjectionMap::PM_21;
                        cfg.index[0] = 2;
                        cfg.index[1] = 0;
                        cfg.index[2] = 1;
                    }
                    cfg.min = d0;
                    cfg.max = d1;
                }
                else // d2 < d0 <= d1
                {
                    if (d0 != d1)
                    {
                        cfg.map = ProjectionMap::PM_111;
                    }
                    else
                    {
                        cfg.map = ProjectionMap::PM_12;
                    }

                    cfg.index[0] = 2;
                    cfg.index[1] = 0;
                    cfg.index[2] = 1;
                    cfg.min = d2;
                    cfg.max = d1;
                }
            }
            else if (d2 <= d1) // d2 <= d1 < d0
            {
                if (d2 != d1)
                {
                    cfg.map = ProjectionMap::PM_111;
                    cfg.index[0] = 2;
                    cfg.index[1] = 1;
                    cfg.index[2] = 0;
                }
                else
                {
                    cfg.map = ProjectionMap::PM_21;
                    cfg.index[0] = 1;
                    cfg.index[1] = 2;
                    cfg.index[2] = 0;

                }
                cfg.min = d2;
                cfg.max = d0;
            }
            else if (d2 <= d0) // d1 < d2 <= d0
            {
                if (d2 != d0)
                {
                    cfg.map = ProjectionMap::PM_111;
                }
                else
                {
                    cfg.map = ProjectionMap::PM_12;
                }

                cfg.index[0] = 1;
                cfg.index[1] = 2;
                cfg.index[2] = 0;
                cfg.min = d1;
                cfg.max = d0;
            }
            else // d1 < d0 < d2
            {
                cfg.map = ProjectionMap::PM_111;
                cfg.index[0] = 1;
                cfg.index[1] = 0;
                cfg.index[2] = 2;
                cfg.min = d1;
                cfg.max = d2;
            }
        }

        // This is the constant velocity separating axis test. The cfg0 and
        // cfg1 inputs are the configurations for the triangles at time 0.
        // The tcfg0 and tcfg1 are the configurations at contact time.
        static bool FindOverlap(T const& tMax, T const& speed,
            Configuration const& cfg0, Configuration const& cfg1,
            ContactSide& side, Configuration& tcfg0, Configuration& tcfg1,
            T& tFirst, T& tLast)
        {
            T const zero = static_cast<T>(0);

            if (cfg1.max < cfg0.min)
            {
                // The cfg1 interval is on the left of the cfg0 interval.
                if (speed <= zero)
                {
                    // The cfg1 interval is moving away from the cfg0
                    // interval.
                    return false;
                }

                // Compute the first time of contact on this axis.
                T t = (cfg0.min - cfg1.max) / speed;

                if (t > tFirst)
                {
                    // Time t is the new maximum first time of contact.
                    tFirst = t;
                    side = ContactSide::CS_LEFT;
                    tcfg0 = cfg0;
                    tcfg1 = cfg1;
                }

                if (tFirst > tMax)
                {
                    // The intersection occurs after the specified maximum
                    // time.
                    return false;
                }

                // Compute the last time of contact on this axis.
                t = (cfg0.max - cfg1.min) / speed;
                if (t < tLast)
                {
                    tLast = t;
                }

                if (tFirst > tLast)
                {
                    // The time interval is invalid, so the objects are
                    // not intersecting.
                    return false;
                }
            }
            else if (cfg0.max < cfg1.min)
            {
                // The cfg1 interval is on the right of the cfg0 interval.
                if (speed >= zero)
                {
                    // The cfg1 interval is moving away from the cfg0
                    // interval.
                    return false;
                }

                // Compute the first time of contact on this axis.
                T t = (cfg0.max - cfg1.min) / speed;

                if (t > tFirst)
                {
                    // Time t is the new maximum first time of contact.
                    tFirst = t;
                    side = ContactSide::CS_RIGHT;
                    tcfg0 = cfg0;
                    tcfg1 = cfg1;
                }

                if (tFirst > tMax)
                {
                    // The intersection occurs after the specified maximum
                    // time.
                    return false;
                }

                // Compute the last time of contact on this axis.
                t = (cfg0.min - cfg1.max) / speed;
                if (t < tLast)
                {
                    tLast = t;
                }

                if (tFirst > tLast)
                {
                    // The time interval is invalid, so the objects are
                    // not intersecting.
                    return false;
                }
            }
            else
            {
                // The intervals for cfg0 and cfg1 are currently
                // overlapping.
                if (speed > zero)
                {
                    // Compute the last time of contact on this axis.
                    T t = (cfg0.max - cfg1.min) / speed;
                    if (t < tLast)
                    {
                        tLast = t;
                    }

                    if (tFirst > tLast)
                    {
                        // The time interval is invalid, so the objects are
                        // not intersecting.
                        return false;
                    }
                }
                else if (speed < zero)
                {
                    // Compute the last time of contact on this axis.
                    T t = (cfg0.min - cfg1.max) / speed;
                    if (t < tLast)
                    {
                        tLast = t;
                    }

                    if (tFirst > tLast)
                    {
                        // The time interval is invalid, so the objects are
                        // not intersecting.
                        return false;
                    }
                }
            }
            return true;
        }

        // A projection wrapper to set up for the separating axis test.
        static bool FindOverlap(Triangle3<T> const& triangle0,
            Triangle3<T> const& triangle1, Vector3<T> const& direction,
            T const& tMax, Vector3<T> const& velocity, ContactSide& side,
            Configuration& tcfg0, Configuration& tcfg1, T& tFirst, T& tLast)
        {
            Configuration cfg0{}, cfg1{};
            ScaleProjectOntoLine(triangle0, direction, cfg0);
            ScaleProjectOntoLine(triangle1, direction, cfg1);
            T speed = Dot(direction, velocity);
            return FindOverlap(tMax, speed, cfg0, cfg1, side, tcfg0, tcfg1,
                tFirst, tLast);
        }
    };
}
