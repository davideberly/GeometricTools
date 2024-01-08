// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The find-intersection query is based on the document
// https://www.geometrictools.com/Documentation/IntersectionMovingSphereBox.pdf
// and also uses the method of separating axes.
// https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf

#include <Mathematics/DistPointAlignedBox.h>
#include <Mathematics/IntrRay3AlignedBox3.h>
#include <Mathematics/Hypersphere.h>
#include <array>
#include <cmath>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, AlignedBox3<T>, Sphere3<T>>
    {
    public:
        // The intersection query considers the box and sphere to be solids;
        // that is, the sphere object includes the region inside the spherical
        // boundary and the box object includes the region inside the cuboid
        // boundary.  If the sphere object and box object object overlap, the
        // objects intersect.
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(AlignedBox3<T> const& box, Sphere3<T> const& sphere)
        {
            DCPQuery<T, Vector3<T>, AlignedBox3<T>> pbQuery;
            auto pbResult = pbQuery(sphere.center, box);
            Result result{};
            result.intersect = (pbResult.sqrDistance <= sphere.radius * sphere.radius);
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, AlignedBox3<T>, Sphere3<T>>
    {
    public:
        // Currently, only a dynamic query is supported.  A static query will
        // need to compute the intersection set of (solid) box and sphere.
        struct Result
        {
            Result()
                :
                intersectionType(0),
                contactTime(static_cast<T>(0)),
                contactPoint(Vector3<T>::Zero())
            {
            }

            // The cases are
            // 1. Objects initially overlapping.  The contactPoint is only one
            //    of infinitely many points in the overlap.
            //      intersectionType = -1
            //      contactTime = 0
            //      contactPoint = sphere.center
            // 2. Objects initially separated but do not intersect later.  The
            //      contactTime and contactPoint are invalid.
            //      intersectionType = 0
            //      contactTime = 0
            //      contactPoint = (0,0,0)
            // 3. Objects initially separated but intersect later.
            //      intersectionType = +1
            //      contactTime = first time T > 0
            //      contactPoint = corresponding first contact
            int32_t intersectionType;
            T contactTime;
            Vector3<T> contactPoint;

            // TODO: To support arbitrary precision for the contactTime,
            // return q0, q1 and q2 where contactTime = (q0 - sqrt(q1)) / q2.
            // The caller can compute contactTime to desired number of digits
            // of precision.  These are valid when intersectionType is +1 but
            // are set to zero (invalid) in the other cases.  Do the same for
            // the contactPoint.
        };

        Result operator()(AlignedBox3<T> const& box, Vector3<T> const& boxVelocity,
            Sphere3<T> const& sphere, Vector3<T> const& sphereVelocity)
        {
            Result result{};

            // Translate the sphere and box so that the box center becomes
            // the origin.  Compute the velocity of the sphere relative to
            // the box.
            Vector3<T> boxCenter = (box.max + box.min) * (T)0.5;
            Vector3<T> extent = (box.max - box.min) * (T)0.5;
            Vector3<T> C = sphere.center - boxCenter;
            Vector3<T> V = sphereVelocity - boxVelocity;

            // Test for no-intersection that leads to an early exit.  The test
            // is fast, using the method of separating axes.
            AlignedBox3<T> superBox;
            for (int32_t i = 0; i < 3; ++i)
            {
                superBox.max[i] = extent[i] + sphere.radius;
                superBox.min[i] = -superBox.max[i];
            }
            TIQuery<T, Ray3<T>, AlignedBox3<T>> rbQuery;
            auto rbResult = rbQuery(Ray3<T>(C, V), superBox);
            if (rbResult.intersect)
            {
                DoQuery(extent, C, sphere.radius, V, result);

                // Translate the contact point back to the coordinate system
                // of the original sphere and box.
                result.contactPoint += boxCenter;
            }
            return result;
        }

    protected:
        // The query assumes the box is axis-aligned with center at the
        // origin. Callers need to convert the results back to the original
        // coordinate system of the query.
        void DoQuery(Vector3<T> const& K, Vector3<T> const& inC,
            T radius, Vector3<T> const& inV, Result& result)
        {
            // Change signs on components, if necessary, to transform C to the
            // first quadrant. Adjust the velocity accordingly.
            Vector3<T> C = inC, V = inV;
            std::array<T, 3> sign = { (T)0, (T)0, (T)0 };
            for (int32_t i = 0; i < 3; ++i)
            {
                if (C[i] >= (T)0)
                {
                    sign[i] = (T)1;
                }
                else
                {
                    C[i] = -C[i];
                    V[i] = -V[i];
                    sign[i] = (T)-1;
                }
            }

            Vector3<T> delta = C - K;
            if (delta[2] <= radius)
            {
                if (delta[1] <= radius)
                {
                    if (delta[0] <= radius)
                    {
                        if (delta[2] <= (T)0)
                        {
                            if (delta[1] <= (T)0)
                            {
                                if (delta[0] <= (T)0)
                                {
                                    InteriorOverlap(C, result);
                                }
                                else
                                {
                                    // x-face
                                    FaceOverlap(0, 1, 2, K, C, radius, delta, result);
                                }
                            }
                            else
                            {
                                if (delta[0] <= (T)0)
                                {
                                    // y-face
                                    FaceOverlap(1, 2, 0, K, C, radius, delta, result);
                                }
                                else
                                {
                                    // xy-edge
                                    if (delta[0] * delta[0] + delta[1] * delta[1] <= radius * radius)
                                    {
                                        EdgeOverlap(0, 1, 2, K, C, radius, delta, result);
                                    }
                                    else
                                    {
                                        EdgeSeparated(0, 1, 2, K, C, radius, delta, V, result);
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (delta[1] <= (T)0)
                            {
                                if (delta[0] <= (T)0)
                                {
                                    // z-face
                                    FaceOverlap(2, 0, 1, K, C, radius, delta, result);
                                }
                                else
                                {
                                    // xz-edge
                                    if (delta[0] * delta[0] + delta[2] * delta[2] <= radius * radius)
                                    {
                                        EdgeOverlap(2, 0, 1, K, C, radius, delta, result);
                                    }
                                    else
                                    {
                                        EdgeSeparated(2, 0, 1, K, C, radius, delta, V, result);
                                    }
                                }
                            }
                            else
                            {
                                if (delta[0] <= (T)0)
                                {
                                    // yz-edge
                                    if (delta[1] * delta[1] + delta[2] * delta[2] <= radius * radius)
                                    {
                                        EdgeOverlap(1, 2, 0, K, C, radius, delta, result);
                                    }
                                    else
                                    {
                                        EdgeSeparated(1, 2, 0, K, C, radius, delta, V, result);
                                    }
                                }
                                else
                                {
                                    // xyz-vertex
                                    if (Dot(delta, delta) <= radius * radius)
                                    {
                                        VertexOverlap(K, radius, delta, result);
                                    }
                                    else
                                    {
                                        VertexSeparated(K, radius, delta, V, result);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        // x-face
                        FaceUnbounded(0, 1, 2, K, C, radius, delta, V, result);
                    }
                }
                else
                {
                    if (delta[0] <= radius)
                    {
                        // y-face
                        FaceUnbounded(1, 2, 0, K, C, radius, delta, V, result);
                    }
                    else
                    {
                        // xy-edge
                        EdgeUnbounded(0, 1, 2, K, C, radius, delta, V, result);
                    }
                }
            }
            else
            {
                if (delta[1] <= radius)
                {
                    if (delta[0] <= radius)
                    {
                        // z-face
                        FaceUnbounded(2, 0, 1, K, C, radius, delta, V, result);
                    }
                    else
                    {
                        // xz-edge
                        EdgeUnbounded(2, 0, 1, K, C, radius, delta, V, result);
                    }
                }
                else
                {
                    if (delta[0] <= radius)
                    {
                        // yz-edge
                        EdgeUnbounded(1, 2, 0, K, C, radius, delta, V, result);
                    }
                    else
                    {
                        // xyz-vertex
                        VertexUnbounded(K, C, radius, delta, V, result);
                    }
                }
            }

            if (result.intersectionType != 0)
            {
                // Translate back to the coordinate system of the
                // tranlated box and sphere.
                for (int32_t i = 0; i < 3; ++i)
                {
                    if (sign[i] < (T)0)
                    {
                        result.contactPoint[i] = -result.contactPoint[i];
                    }
                }
            }
        }

    private:
        void InteriorOverlap(Vector3<T> const& C, Result& result)
        {
            result.intersectionType = -1;
            result.contactTime = (T)0;
            result.contactPoint = C;
        }

        void VertexOverlap(Vector3<T> const& K, T radius,
            Vector3<T> const& delta, Result& result)
        {
            result.intersectionType = (Dot(delta, delta) < radius * radius ? -1 : 1);
            result.contactTime = (T)0;
            result.contactPoint = K;
        }

        void EdgeOverlap(int32_t i0, int32_t i1, int32_t i2, Vector3<T> const& K,
            Vector3<T> const& C, T radius, Vector3<T> const& delta,
            Result& result)
        {
            result.intersectionType = (delta[i0] * delta[i0] + delta[i1] * delta[i1] < radius * radius ? -1 : 1);
            result.contactTime = (T)0;
            result.contactPoint[i0] = K[i0];
            result.contactPoint[i1] = K[i1];
            result.contactPoint[i2] = C[i2];
        }

        void FaceOverlap(int32_t i0, int32_t i1, int32_t i2, Vector3<T> const& K,
            Vector3<T> const& C, T radius, Vector3<T> const& delta,
            Result& result)
        {
            result.intersectionType = (delta[i0] < radius ? -1 : 1);
            result.contactTime = (T)0;
            result.contactPoint[i0] = K[i0];
            result.contactPoint[i1] = C[i1];
            result.contactPoint[i2] = C[i2];
        }

        void VertexSeparated(Vector3<T> const& K, T radius,
            Vector3<T> const& delta, Vector3<T> const& V, Result& result)
        {
            if (V[0] < (T)0 || V[1] < (T)0 || V[2] < (T)0)
            {
                DoQueryRayRoundedVertex(K, radius, delta, V, result);
            }
        }

        void EdgeSeparated(int32_t i0, int32_t i1, int32_t i2, Vector3<T> const& K,
            Vector3<T> const& C, T radius, Vector3<T> const& delta,
            Vector3<T> const& V, Result& result)
        {
            if (V[i0] < (T)0 || V[i1] < (T)0)
            {
                DoQueryRayRoundedEdge(i0, i1, i2, K, C, radius, delta, V, result);
            }
        }

        void VertexUnbounded(Vector3<T> const& K, Vector3<T> const& C, T radius,
            Vector3<T> const& delta, Vector3<T> const& V, Result& result)
        {
            if (V[0] < (T)0 && V[1] < (T)0 && V[2] < (T)0)
            {
                // Determine the face of the rounded box that is intersected
                // by the ray C+T*V.
                T tmax = (radius - delta[0]) / V[0];
                int32_t j0 = 0;
                T temp = (radius - delta[1]) / V[1];
                if (temp > tmax)
                {
                    tmax = temp;
                    j0 = 1;
                }
                temp = (radius - delta[2]) / V[2];
                if (temp > tmax)
                {
                    tmax = temp;
                    j0 = 2;
                }

                // The j0-rounded face is the candidate for intersection.
                int32_t j1 = (j0 + 1) % 3;
                int32_t j2 = (j1 + 1) % 3;
                DoQueryRayRoundedFace(j0, j1, j2, K, C, radius, delta, V, result);
            }
        }

        void EdgeUnbounded(int32_t i0, int32_t i1, int32_t /* i2 */, Vector3<T> const& K,
            Vector3<T> const& C, T radius, Vector3<T> const& delta,
            Vector3<T> const& V, Result& result)
        {
            if (V[i0] < (T)0 && V[i1] < (T)0)
            {
                // Determine the face of the rounded box that is intersected
                // by the ray C+T*V.
                T tmax = (radius - delta[i0]) / V[i0];
                int32_t j0 = i0;
                T temp = (radius - delta[i1]) / V[i1];
                if (temp > tmax)
                {
                    tmax = temp;
                    j0 = i1;
                }

                // The j0-rounded face is the candidate for intersection.
                int32_t j1 = (j0 + 1) % 3;
                int32_t j2 = (j1 + 1) % 3;
                DoQueryRayRoundedFace(j0, j1, j2, K, C, radius, delta, V, result);
            }
        }

        void FaceUnbounded(int32_t i0, int32_t i1, int32_t i2, Vector3<T> const& K,
            Vector3<T> const& C, T radius, Vector3<T> const& delta,
            Vector3<T> const& V, Result& result)
        {
            if (V[i0] < (T)0)
            {
                DoQueryRayRoundedFace(i0, i1, i2, K, C, radius, delta, V, result);
            }
        }

        void DoQueryRayRoundedVertex(Vector3<T> const& K, T radius,
            Vector3<T> const& delta, Vector3<T> const& V, Result& result)
        {
            T a1 = Dot(V, delta);
            if (a1 < (T)0)
            {
                // The caller must ensure that a0 > 0 and a2 > 0.
                T a0 = Dot(delta, delta) - radius * radius;
                T a2 = Dot(V, V);
                T adiscr = a1 * a1 - a2 * a0;
                if (adiscr >= (T)0)
                {
                    // The ray intersects the rounded vertex, so the sphere-box
                    // contact point is the vertex.
                    result.intersectionType = 1;
                    result.contactTime = -(a1 + std::sqrt(adiscr)) / a2;
                    result.contactPoint = K;
                }
            }
        }

        void DoQueryRayRoundedEdge(int32_t i0, int32_t i1, int32_t i2, Vector3<T> const& K,
            Vector3<T> const& C, T radius, Vector3<T> const& delta,
            Vector3<T> const& V, Result& result)
        {
            T b1 = V[i0] * delta[i0] + V[i1] * delta[i1];
            if (b1 < (T)0)
            {
                // The caller must ensure that b0 > 0 and b2 > 0.
                T b0 = delta[i0] * delta[i0] + delta[i1] * delta[i1] - radius * radius;
                T b2 = V[i0] * V[i0] + V[i1] * V[i1];
                T bdiscr = b1 * b1 - b2 * b0;
                if (bdiscr >= (T)0)
                {
                    T tmax = -(b1 + std::sqrt(bdiscr)) / b2;
                    T p2 = C[i2] + tmax * V[i2];
                    if (-K[i2] <= p2)
                    {
                        if (p2 <= K[i2])
                        {
                            // The ray intersects the finite cylinder of the
                            // rounded edge, so the sphere-box contact point
                            // is on the corresponding box edge.
                            result.intersectionType = 1;
                            result.contactTime = tmax;
                            result.contactPoint[i0] = K[i0];
                            result.contactPoint[i1] = K[i1];
                            result.contactPoint[i2] = p2;
                        }
                        else
                        {
                            // The ray intersects the infinite cylinder but
                            // not the finite cylinder of the rounded edge.
                            // It is possible the ray intersects the rounded
                            // vertex for K.
                            DoQueryRayRoundedVertex(K, radius, delta, V, result);
                        }
                    }
                    else
                    {
                        // The ray intersects the infinite cylinder but
                        // not the finite cylinder of the rounded edge.
                        // It is possible the ray intersects the rounded
                        // vertex for otherK.
                        Vector3<T> otherK, otherDelta;
                        otherK[i0] = K[i0];
                        otherK[i1] = K[i1];
                        otherK[i2] = -K[i2];
                        otherDelta[i0] = C[i0] - otherK[i0];
                        otherDelta[i1] = C[i1] - otherK[i1];
                        otherDelta[i2] = C[i2] - otherK[i2];
                        DoQueryRayRoundedVertex(otherK, radius, otherDelta, V, result);
                    }
                }
            }
        }

        void DoQueryRayRoundedFace(int32_t i0, int32_t i1, int32_t i2, Vector3<T> const& K,
            Vector3<T> const& C, T radius, Vector3<T> const& delta,
            Vector3<T> const& V, Result& result)
        {
            Vector3<T> otherK, otherDelta;

            T tmax = (radius - delta[i0]) / V[i0];
            T p1 = C[i1] + tmax * V[i1];
            T p2 = C[i2] + tmax * V[i2];

            if (p1 < -K[i1])
            {
                // The ray potentially intersects the rounded (i0,i1)-edge
                // whose top-most vertex is otherK.
                otherK[i0] = K[i0];
                otherK[i1] = -K[i1];
                otherK[i2] = K[i2];
                otherDelta[i0] = C[i0] - otherK[i0];
                otherDelta[i1] = C[i1] - otherK[i1];
                otherDelta[i2] = C[i2] - otherK[i2];
                DoQueryRayRoundedEdge(i0, i1, i2, otherK, C, radius, otherDelta, V, result);
                if (result.intersectionType == 0)
                {
                    if (p2 < -K[i2])
                    {
                        // The ray potentially intersects the rounded
                        // (i2,i0)-edge whose right-most vertex is otherK.
                        otherK[i0] = K[i0];
                        otherK[i1] = K[i1];
                        otherK[i2] = -K[i2];
                        otherDelta[i0] = C[i0] - otherK[i0];
                        otherDelta[i1] = C[i1] - otherK[i1];
                        otherDelta[i2] = C[i2] - otherK[i2];
                        DoQueryRayRoundedEdge(i2, i0, i1, otherK, C, radius, otherDelta, V, result);
                    }
                    else if (p2 > K[i2])
                    {
                        // The ray potentially intersects the rounded
                        // (i2,i0)-edge whose right-most vertex is K.
                        DoQueryRayRoundedEdge(i2, i0, i1, K, C, radius, delta, V, result);
                    }
                }
            }
            else if (p1 <= K[i1])
            {
                if (p2 < -K[i2])
                {
                    // The ray potentially intersects the rounded
                    // (i2,i0)-edge whose right-most vertex is otherK.
                    otherK[i0] = K[i0];
                    otherK[i1] = K[i1];
                    otherK[i2] = -K[i2];
                    otherDelta[i0] = C[i0] - otherK[i0];
                    otherDelta[i1] = C[i1] - otherK[i1];
                    otherDelta[i2] = C[i2] - otherK[i2];
                    DoQueryRayRoundedEdge(i2, i0, i1, otherK, C, radius, otherDelta, V, result);
                }
                else if (p2 <= K[i2])
                {
                    // The ray intersects the i0-face of the rounded box, so
                    // the sphere-box contact point is on the corresponding
                    // box face.
                    result.intersectionType = 1;
                    result.contactTime = tmax;
                    result.contactPoint[i0] = K[i0];
                    result.contactPoint[i1] = p1;
                    result.contactPoint[i2] = p2;
                }
                else  // p2 > K[i2]
                {
                    // The ray potentially intersects the rounded
                    // (i2,i0)-edge whose right-most vertex is K.
                    DoQueryRayRoundedEdge(i2, i0, i1, K, C, radius, delta, V, result);
                }
            }
            else // p1 > K[i1]
            {
                // The ray potentially intersects the rounded (i0,i1)-edge
                // whose top-most vertex is K.
                DoQueryRayRoundedEdge(i0, i1, i2, K, C, radius, delta, V, result);
                if (result.intersectionType == 0)
                {
                    if (p2 < -K[i2])
                    {
                        // The ray potentially intersects the rounded
                        // (i2,i0)-edge whose right-most vertex is otherK.
                        otherK[i0] = K[i0];
                        otherK[i1] = K[i1];
                        otherK[i2] = -K[i2];
                        otherDelta[i0] = C[i0] - otherK[i0];
                        otherDelta[i1] = C[i1] - otherK[i1];
                        otherDelta[i2] = C[i2] - otherK[i2];
                        DoQueryRayRoundedEdge(i2, i0, i1, otherK, C, radius, otherDelta, V, result);
                    }
                    else if (p2 > K[i2])
                    {
                        // The ray potentially intersects the rounded
                        // (i2,i0)-edge whose right-most vertex is K.
                        DoQueryRayRoundedEdge(i2, i0, i1, K, C, radius, delta, V, result);
                    }
                }
            }
        }
    };
}
