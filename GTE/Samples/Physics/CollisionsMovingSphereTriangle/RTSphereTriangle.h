// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.21

#pragma once

#include <Mathematics/Vector3.h>
using namespace gte;

// Compute the first time of contact and the corresponding first point of
// contact for a moving sphere and a moving triangle.

class RTSphereTriangle
{
public:
    struct Sphere
    {
        Sphere();

        // Call this after 'radius' is modified.
        void ComputeDerived();

        Vector3<float> center;
        float radius;
        float radiusSqr;
    };

    struct Triangle
    {
        Triangle();

        // Call this after any of position[] have been modified.
        void ComputeDerived();

        // The positions of the triangle vertices.
        std::array<Vector3<float>, 3> position;

        // The edge directions, all unit length.
        std::array<Vector3<float>, 3> edge;

        // The edge half-lengths.
        std::array<float, 3> halfLength;

        // The midpoints of the edges.
        std::array<Vector3<float>, 3> midPoint;

        // The triangle normal direction, unit length.
        Vector3<float> normal;

        // The edge normals in the plane of tre triangle, unit length and
        // outer pointing.
        std::array<Vector3<float>, 3> edgeNormal;
    };

    enum class ContactType
    {
        SEPARATED,
        CONTACT,
        OVERLAPPING
    };

    // The time interval of the query is [0,tMax]. On return,
    //   type = SEPARATED:  No intersection during the time interval, so
    //       contactTime and contactPoint are invalid.
    //   type = CONTACT:  A single time and point of contact during the
    //       time interval, so contactTime and contactPoint are valid.
    //   type = OVERLAPPING:  The sphere and triangle overlap at time zero.
    //       The set of intersection has infinitely many points. The
    //       contactTime is zero. The contactPoint stores the triangle
    //       point that is closest to the sphere center.
    static ContactType Collide(
        Sphere const& sphere,
        Vector3<float> const& sphereVelocity,
        Triangle const& triangle,
        Vector3<float> const& triangleVelocity,
        float tMax,
        float& contactTime,
        Vector3<float>& contactPoint);

private:
    // Compute the squared distance from C to the triangle.
    static float SqrDistance(Vector3<float> const& center,
        Triangle const& triangle, Vector3<float>& closest);

    // Clip C+t*V with [t0,t1] against the plane Dot(N,X-P) = 0, discarding
    // that portion of the interval on the side of the plane to which N is
    // directed. The return value is 'true' when a nonempty interval exists
    // after clipping.
    static bool ClipAgainstPlane(Vector3<float> const& center,
        Vector3<float> const& velocity, Vector3<float> const& normal,
        Vector3<float> const& position, float& t0, float& t1);

    // Compute the intersection of the segment C+t*V, [0,tMax], with the
    // sphere |X-P| = r. The function returns 'true' iff [t0,t1] is a
    // nonempty interval.
    static bool IntersectLineSphere(Sphere const& sphere,
        Vector3<float> const& velocity, Vector3<float> const& position,
        float tMax, float& t0, float& t1);

    // Compute the intersection of the segment C+t*V, [0,tMax], with a finite
    // cylinder. The cylinder has center P, radius r, height h and axis
    // direction U2. The set {U0,U1,U2} is orthonormal and right-handed. In
    // the coordinate system of the cylinder, a point A = P+x*U0+y*U1+z*U2. To
    // be inside the cylinder, x*x + y*y <= r*r and |z| <= h/2. The function
    // returns 'true' iff [t0,t1] is a nonempty interval.
    static bool IntersectLineCylinder(Sphere const& sphere,
        Vector3<float> const& velocity, Vector3<float> const& position,
        Vector3<float> const& U0, Vector3<float> const& U1,
        Vector3<float> const& U2, float halfHeight, float tMax,
        float& t0, float& t1);

    // Compute the intersection of the segment C+t*V, [0,tMax], with the
    // extruded triangle whose faces are Dot(N,X-P0) = r, Dot(-N,X-P0) = r,
    // Dot(EN0,X-P0) = 0, Dot(EN1,X-P1) = 0 and Dot(EN2,X-P2) = 0. The
    // function returns 'true' iff [t0,t1] is a nonempty interval.
    static bool IntersectLinePolyhedron(Sphere const& sphere,
        Vector3<float> const& velocity, Triangle const& triangle,
        float tMax, float& t0, float& t1);
};
