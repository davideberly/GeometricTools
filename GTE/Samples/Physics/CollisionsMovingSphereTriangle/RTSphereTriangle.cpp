// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2023.08.08

#include <Mathematics/Logger.h>
#include "RTSphereTriangle.h"
#include <limits>

RTSphereTriangle::Sphere::Sphere()
    :
    center(Vector3<float>::Zero()),
    radius(0.0f),
    radiusSqr(0.0f)
{
}

void RTSphereTriangle::Sphere::ComputeDerived()
{
    radiusSqr = radius * radius;
}

RTSphereTriangle::Triangle::Triangle()
    :
    position{
        Vector3<float>::Zero(),
        Vector3<float>::Zero(),
        Vector3<float>::Zero() },
    edge{
        Vector3<float>::Zero(),
        Vector3<float>::Zero(),
        Vector3<float>::Zero() },
    halfLength{ 0.0f, 0.0f, 0.0f },
    midPoint{
        Vector3<float>::Zero(),
        Vector3<float>::Zero(),
        Vector3<float>::Zero() },
    normal(Vector3<float>::Zero()),
    edgeNormal{
        Vector3<float>::Zero(),
        Vector3<float>::Zero(),
        Vector3<float>::Zero() }
{
}

void RTSphereTriangle::Triangle::ComputeDerived()
{
    edge[0] = position[1] - position[0];
    edge[1] = position[2] - position[1];
    edge[2] = position[0] - position[2];
    halfLength[0] = 0.5f * Normalize(edge[0]);
    halfLength[1] = 0.5f * Normalize(edge[1]);
    halfLength[2] = 0.5f * Normalize(edge[2]);
    midPoint[0] = position[0] + halfLength[0] * edge[0];
    midPoint[1] = position[1] + halfLength[1] * edge[1];
    midPoint[2] = position[2] + halfLength[2] * edge[2];
    normal = UnitCross(edge[0], edge[1]);
    edgeNormal[0] = Cross(edge[0], normal);
    edgeNormal[1] = Cross(edge[1], normal);
    edgeNormal[2] = Cross(edge[2], normal);

}

RTSphereTriangle::ContactType RTSphereTriangle::Collide(
    Sphere const& sphere, Vector3<float> const& sphereVelocity,
    Triangle const& triangle, Vector3<float> const& triangleVelocity,
    float tMax, float& contactTime, Vector3<float>& contactPoint)
{
    // Test the sphere-triangle relationship at time zero.
    float sqrDistance = SqrDistance(sphere.center, triangle, contactPoint);
    if (sqrDistance < sphere.radiusSqr)
    {
        contactTime = 0.0f;
        return ContactType::OVERLAPPING;
    }
    else if (sqrDistance == sphere.radiusSqr)
    {
        contactTime = 0.0f;
        return ContactType::CONTACT;
    }

    // The sphere and triangle are initially separated. Compute the velocity
    // of the sphere relative to triangle, so the triangle is then stationary.
    Vector3<float> relativeVelocity = sphereVelocity - triangleVelocity;
    if (relativeVelocity == Vector3<float>::Zero())
    {
        // The objects are stationary relative to each other.
        return ContactType::SEPARATED;
    }

    float t0Intr = std::numeric_limits<float>::max();
    float t1Intr = -std::numeric_limits<float>::max();
    float t0{}, t1{};

    if (IntersectLinePolyhedron(sphere, relativeVelocity, triangle, tMax, t0, t1))
    {
        if (t0 < t0Intr)
        {
            t0Intr = t0;
        }
        if (t1 > t1Intr)
        {
            t1Intr = t1;
        }
    }

    for (size_t i = 0; i < 3; ++i)
    {
        if (IntersectLineCylinder(sphere, relativeVelocity, triangle.midPoint[i],
            triangle.normal, triangle.edgeNormal[i], triangle.edge[i],
            triangle.halfLength[i], tMax, t0, t1))
        {
            if (t0 < t0Intr)
            {
                t0Intr = t0;
            }
            if (t1 > t1Intr)
            {
                t1Intr = t1;
            }
        }
    }

    for (size_t i = 0; i < 3; ++i)
    {
        if (IntersectLineSphere(sphere, relativeVelocity, triangle.position[i],
            tMax, t0, t1))
        {
            if (t0 < t0Intr)
            {
                t0Intr = t0;
            }
            if (t1 > t1Intr)
            {
                t1Intr = t1;
            }
        }
    }

    if (t0Intr <= t1Intr)
    {
        contactTime = t0Intr;
        Vector3<float> center = sphere.center + contactTime * relativeVelocity;
        sqrDistance = SqrDistance(center, triangle, contactPoint);
        contactPoint += contactTime * triangleVelocity;
        return ContactType::CONTACT;
    }
    else
    {
        return ContactType::SEPARATED;
    }
}

float RTSphereTriangle::SqrDistance(Vector3<float> const& center,
    Triangle const& triangle, Vector3<float>& closest)
{
    Vector3<float> CmP0 = center - triangle.position[0];
    Vector3<float> CmP1 = center - triangle.position[1];
    Vector3<float> CmP2 = center - triangle.position[2];
    float dot0 = Dot(triangle.edgeNormal[0], CmP0);
    float dot1 = Dot(triangle.edgeNormal[1], CmP1);
    float dot2 = Dot(triangle.edgeNormal[2], CmP2);
    float E0dCmP0{}, E0dCmP1{}, E1dCmP1{}, E1dCmP2{}, E2dCmP2{}, E2dCmP0{};

    if (dot0 > 0.0f)
    {
        if (dot1 > 0.0f)
        {
            if (dot2 > 0.0f)
            {
                // +++: It is not theoretically possible to reach this case
                // because not all three edges can be/ visible from outside
                // the triangle. With numerical rounding errors, it might be
                // possible for this case to happen, so handle it anyway.
                closest = (triangle.position[0] + triangle.position[1] +
                    triangle.position[2]) / 3.0f;
                Vector3<float> diff = center - closest;
                return Dot(diff, diff);
            }
            else
            {
                // ++-: Edges E0 and E1 are visible.
                E0dCmP0 = Dot(triangle.edge[0], CmP0);
                if (E0dCmP0 <= 0.0f)
                {
                    // Vertex P0 is the closest feature.
                    closest = triangle.position[0];
                    return Dot(CmP0, CmP0);
                }

                E0dCmP1 = Dot(triangle.edge[0], CmP1);
                if (E0dCmP1 < 0.0f)
                {
                    // Edge E0 is the closest feature.
                    closest = triangle.position[0] + E0dCmP0 * triangle.edge[0];
                    return std::fabs(Dot(CmP0, CmP0) - E0dCmP0 * E0dCmP0);
                }

                E1dCmP1 = Dot(triangle.edge[1], CmP1);
                if (E1dCmP1 <= 0.0f)
                {
                    // Vertex P1 is the closest feature.
                    closest = triangle.position[1];
                    return Dot(CmP1, CmP1);
                }

                E1dCmP2 = Dot(triangle.edge[1], CmP2);
                if (E1dCmP2 < 0.0f)
                {
                    // Edge E1 is the closest feature.
                    closest = triangle.position[1] + E1dCmP1 * triangle.edge[1];
                    return std::fabs(Dot(CmP1, CmP1) - E1dCmP1 * E1dCmP1);
                }

                // Vertex P2 is closest the feature.
                closest = triangle.position[2];
                return Dot(CmP2, CmP2);
            }
        }
        else
        {
            if (dot2 > 0.0f)
            {
                // +-+: Edges E2 and E0 are visible.
                E2dCmP2 = Dot(triangle.edge[2], CmP2);
                if (E2dCmP2 <= 0.0f)
                {
                    // Vertex P2 is the closest feature.
                    closest = triangle.position[2];
                    return Dot(CmP2, CmP2);
                }

                E2dCmP0 = Dot(triangle.edge[2], CmP0);
                if (E2dCmP0 < 0.0f)
                {
                    // Edge E2 is the closest feature.
                    closest = triangle.position[2] + E2dCmP2 * triangle.edge[2];
                    return std::fabs(Dot(CmP2, CmP2) - E2dCmP2 * E2dCmP2);
                }

                E0dCmP0 = Dot(triangle.edge[0], CmP0);
                if (E0dCmP0 <= 0.0f)
                {
                    // Vertex P0 is the closest feature.
                    closest = triangle.position[0];
                    return Dot(CmP0, CmP0);
                }

                E0dCmP1 = Dot(triangle.edge[0], CmP1);
                if (E0dCmP1 < 0.0f)
                {
                    // Edge E0 is the closest feature.
                    closest = triangle.position[0] + E0dCmP0 * triangle.edge[0];
                    return std::fabs(Dot(CmP0, CmP0) - E0dCmP0 * E0dCmP0);
                }

                // Vertex P1 is the closest feature.
                closest = triangle.position[1];
                return Dot(CmP1, CmP1);
            }
            else
            {
                // +--: Edge E0 is visible.
                E0dCmP0 = Dot(triangle.edge[0], CmP0);
                if (E0dCmP0 <= 0.0f)
                {
                    // Vertex P0 is the closest feature.
                    closest = triangle.position[0];
                    return Dot(CmP0, CmP0);
                }

                E0dCmP1 = Dot(triangle.edge[0], CmP1);
                if (E0dCmP1 < 0.0f)
                {
                    // Edge E0 is the closest feature.
                    closest = triangle.position[0] + E0dCmP0 * triangle.edge[0];
                    return std::fabs(Dot(CmP0, CmP0) - E0dCmP0 * E0dCmP0);
                }

                // Vertex P1 is the closest feature.
                closest = triangle.position[1];
                return Dot(CmP1, CmP1);
            }
        }
    }
    else
    {
        if (dot1 > 0.0f)
        {
            if (dot2 > 0.0f)
            {
                // -++: Edges E1 and E2 are visible.
                E1dCmP1 = Dot(triangle.edge[1], CmP1);
                if (E1dCmP1 <= 0.0f)
                {
                    // Vertex P1 is the closest feature.
                    closest = triangle.position[1];
                    return Dot(CmP1, CmP1);
                }

                E1dCmP2 = Dot(triangle.edge[1], CmP2);
                if (E1dCmP2 < 0.0f)
                {
                    // Edge E1 is the closest feature.
                    closest = triangle.position[1] + E1dCmP1 * triangle.edge[1];
                    return std::fabs(Dot(CmP1, CmP1) - E1dCmP1 * E1dCmP1);
                }

                E2dCmP2 = Dot(triangle.edge[2], CmP2);
                if (E2dCmP2 <= 0.0f)
                {
                    // Vertex P2 is the closest feature.
                    closest = triangle.position[2];
                    return Dot(CmP2, CmP2);
                }

                E2dCmP0 = Dot(triangle.edge[2], CmP0);
                if (E2dCmP0 < 0.0f)
                {
                    // Edge E2 is the closest feature.
                    closest = triangle.position[2] + E2dCmP2 * triangle.edge[2];
                    return std::fabs(Dot(CmP2, CmP2) - E2dCmP2 * E2dCmP2);
                }

                // Vertex P0 is the closest feature.
                closest = triangle.position[0];
                return Dot(CmP0, CmP0);
            }
            else
            {
                // -+-: Edge E1 is visible.
                E1dCmP1 = Dot(triangle.edge[1], CmP1);
                if (E1dCmP1 <= 0.0f)
                {
                    // Vertex P1 is the closest feature.
                    closest = triangle.position[1];
                    return Dot(CmP1, CmP1);
                }

                E1dCmP2 = Dot(triangle.edge[1], CmP2);
                if (E1dCmP2 < 0.0f)
                {
                    // Edge E1 is the closest feature.
                    closest = triangle.position[1] + E1dCmP1 * triangle.edge[1];
                    return std::fabs(Dot(CmP1, CmP1) - E1dCmP1 * E1dCmP1);
                }

                // Vertex P2 is the closest feature.
                closest = triangle.position[2];
                return Dot(CmP2, CmP2);
            }
        }
        else
        {
            if (dot2 > 0.0f)
            {
                // --+: Edge E2 is visible.
                E2dCmP2 = Dot(CmP2, triangle.edge[2]);
                if (E2dCmP2 <= 0.0f)
                {
                    // Vertex P2 is the closest feature.
                    closest = triangle.position[2];
                    return Dot(CmP2, CmP2);
                }

                E2dCmP0 = Dot(triangle.edge[2], CmP0);
                if (E2dCmP0 < 0.0f)
                {
                    // Edge E2 is the closest feature.
                    closest = triangle.position[2] + E2dCmP2 * triangle.edge[2];
                    return std::fabs(Dot(CmP2, CmP2) - E2dCmP2 * E2dCmP2);
                }

                // Vertex P0 is the closest feature.
                closest = triangle.position[0];
                return Dot(CmP0, CmP0);
            }
            else
            {
                // ---: The projection of the point is inside the triangle.
                float NdCmP0 = Dot(triangle.normal, CmP0);
                closest = center - NdCmP0 * triangle.normal;
                return std::fabs(NdCmP0);
            }
        }
    }
}

bool RTSphereTriangle::ClipAgainstPlane(Vector3<float> const& center,
    Vector3<float> const& velocity, Vector3<float> const& normal,
    Vector3<float> const& position, float& t0, float& t1)
{
    // Define f(t) = Dot(N, C + t * V - P)
    //             = Dot(N, C - P) + t * Dot(N, V)
    //             = a0 + t * a1
    // Evaluate at the endpoints of the time interval.
    float a0 = Dot(normal, center - position);
    float a1 = Dot(normal, velocity);
    float f0 = a0 + t0 * a1;
    float f1 = a0 + t1 * a1;

    // Clip [t0,t1] against the plane. There are nine cases to consider,
    // depending on the signs of f0 and f1.
    if (f0 > 0.0f)
    {
        if (f1 > 0.0f)
        {
            // The segment is strictly outside the plane.
            return false;
        }
        else if (f1 < 0.0f)
        {
            // The segment intersects the plane at an edge-interior point.
            // T = -a0/a1 is the time of intersection, so discard [t0, T].
            t0 = -a0 / a1;
        }
        else  // f1 == 0.0f
        {
            // The segment is outside the plane but touches at the
            // t1-endpoint, so discard [t0, t1] (degenerate to a point).
            t0 = t1;
        }
    }
    else if (f0 < 0.0f)
    {
        if (f1 > 0.0f)
        {
            // The segment intersects the plane at an edge-interior point.
            // T = -a0/a1 is the time of intersection, so discard [T, t1].
            t1 = -a0 / a1;
        }
    }
    else  // f0 == 0.0f
    {
        if (f1 > 0.0f)
        {
            // The segment is outside the plane but touches at the
            // t0-endpoint, so discard [t0, t1] (degenerate to a point).
            t1 = t0;
        }
    }

    return true;
}

bool RTSphereTriangle::IntersectLineSphere(Sphere const& sphere,
    Vector3<float> const& velocity, Vector3<float> const& position,
    float tMax, float& t0, float& t1)
{
    t0 = 0.0f;
    t1 = tMax;

    // Compute the coefficients for the quadratic equation
    // Q(t) = |C+t * V - P|^2 - r^2 = q0 + 2 * q1 * t + q2 * t^2.
    Vector3<float> CmP = sphere.center - position;
    float q2 = Dot(velocity, velocity);  // not zero in this application
    float q1 = Dot(velocity, CmP);
    float q0 = Dot(CmP, CmP) - sphere.radiusSqr;
    float discr = q1 * q1 - q0 * q2;
    if (discr >= 0.0f)
    {
        // Q(t) has two distinct real-valued roots (discr > 0) or one repeated
        // real-valued root (discr == 0).
        float invQ2 = 1.0f / q2;
        float rootDiscr = sqrtf(discr);
        float root0 = (-q1 - rootDiscr) * invQ2;
        float root1 = (-q1 + rootDiscr) * invQ2;

        // Compute the intersection of [0, tMax] with [root0,root1].
        if (t1 < root0 || t0 > root1)
        {
            // The intersection is empty.
            return false;
        }
        if (t1 == root0)
        {
            // The intersection is a single point.
            t0 = root0;
            t1 = root0;
            return true;
        }
        if (t0 == root1)
        {
            // The intersection is a single point.
            t0 = root1;
            t1 = root1;
            return true;
        }

        // Here we know that t1 > root0 and t0 < root1.
        if (t0 < root0)
        {
            t0 = root0;
        }
        if (t1 > root1)
        {
            t1 = root1;
        }
        return true;
    }
    else
    {
        // Q(t) has no real-valued roots.
        return false;
    }
}

bool RTSphereTriangle::IntersectLineCylinder(Sphere const& sphere,
    Vector3<float> const& velocity, Vector3<float> const& position,
    Vector3<float> const& U0, Vector3<float> const& U1,
    Vector3<float> const& U2, float halfHeight, float tMax,
    float& t0, float& t1)
{
    t0 = 0.0f;
    t1 = tMax;

    // Clip against the plane caps.
    if (!ClipAgainstPlane(sphere.center, velocity, U2,
            position + halfHeight * U2, t0, t1) ||
        !ClipAgainstPlane(sphere.center, velocity, -U2,
            position - halfHeight * U2, t0, t1))
    {
        return false;
    }

    // In cylinder coordinates, C+t*V = P + x(t)*U0 + y(t)*U1 + z(t)*U2,
    // x(t) = Dot(U0,C+t*V-P) = a0 + t*b0, y(t) = Dot(U1,C+t*V-P) = a1 + t*b1
    Vector3<float> CmP = sphere.center - position;
    float a0 = Dot(U0, CmP), b0 = Dot(U0, velocity);
    float a1 = Dot(U1, CmP), b1 = Dot(U1, velocity);

    // Clip the segment [t0, t1] against the cylinder wall.
    float x0 = a0 + t0 * b0, y0 = a1 + t0 * b1, r0Sqr = x0 * x0 + y0 * y0;
    float x1 = a0 + t1 * b0, y1 = a1 + t1 * b1, r1Sqr = x1 * x1 + y1 * y1;
    float rSqr = sphere.radiusSqr;

    // Some case require computing intersections of the segment with the
    // circle of radius r. This amounts to finding roots for the quadratic
    // Q(t) = x(t)*x(t) + y(t)*y(t) - r*r = q2*t^2 + 2*q1*t + q0, where
    // q0 = a0*a0+b0*b0-r*r, q1 = a0*a1+b0*b1, and q2 = a1*a1+b1*b1. Compute
    // the coefficients only when needed.
    float q0{}, q1{}, q2{}, T{};

    if (r0Sqr > rSqr)
    {
        if (r1Sqr > rSqr)
        {
            q2 = b0 * b0 + b1 * b1;
            if (q2 > 0.0f)
            {
                q0 = a0 * a0 + a1 * a1 - rSqr;
                q1 = a0 * b0 + a1 * b1;
                float discr = q1 * q1 - q0 * q2;
                if (discr < 0.0f)
                {
                    // The quadratic has no real-valued roots, so the
                    // segment is outside the cylinder.
                    return false;
                }

                float rootDiscr = sqrtf(discr);
                float invQ2 = 1.0f / q2;
                float root0 = (-q1 - rootDiscr) * invQ2;
                float root1 = (-q1 + rootDiscr) * invQ2;

                // We know that (x0,y0) and (x1,y1) are outside the cylinder,
                // so Q(t0) > 0 and Q(t1) > 0. This reduces the number of
                // cases to analyze for intersection of [t0, t1] and
                // [root0, root1].
                if (t1 < root0 || t0 > root1)
                {
                    // The segment is strictly outside the cylinder.
                    return false;
                }
                else
                {
                    // [t0, t1] strictly contains [root0, root1].
                    t0 = root0;
                    t1 = root1;
                }
            }
            else  // q2 == 0.0f and q1 = 0.0f; that is, Q(t) = q0
            {
                // The segment is degenerate, a point that is outside the
                // cylinder.
                return false;
            }
        }
        else if (r1Sqr < rSqr)
        {
            // Solve nondegenerate quadratic and clip. There must be a single
            // root T in [t0, t1]. Discard [t0, T].
            q0 = a0 * a0 + a1 * a1 - rSqr;
            q1 = a0 * b0 + a1 * b1;
            q2 = b0 * b0 + b1 * b1;
            t0 = (-q1 - std::sqrt(std::fabs(q1 * q1 - q0 * q2))) / q2;
        }
        else // r1Sqr == rSqr
        {
            // The segment intersects the circle at t1. The other root is
            // necessarily T = -t1-2*q1/q2. Use it only when T <= t1, in
            // which case discard [t0, T].
            q1 = a0 * b0 + a1 * b1;
            q2 = b0 * b0 + b1 * b1;
            T = -t1 - 2.0f * q1 / q2;
            t0 = (T < t1 ? T : t1);
        }
    }
    else if (r0Sqr < rSqr)
    {
        if (r1Sqr > rSqr)
        {
            // Solve nondegenerate quadratic and clip. There must be a single
            // root T in [t0, t1]. Discard [T, t1].
            q0 = a0 * a0 + a1 * a1 - rSqr;
            q1 = a0 * b0 + a1 * b1;
            q2 = b0 * b0 + b1 * b1;
            t1 = (-q1 + std::sqrt(std::fabs(q1 * q1 - q0 * q2))) / q2;
        }
        // else: The segment is inside the cylinder.
    }
    else // r0Sqr == rSqr
    {
        if (r1Sqr > rSqr)
        {
            // The segment intersects the circle at t0. The other root is
            // necessarily T = -t0-2*q1/q2. Use it only when T >= t0, in
            // which case discard [T, t1].
            q1 = a0 * b0 + a1 * b1;
            q2 = b0 * b0 + b1 * b1;
            T = -t0 - 2.0f * q1 / q2;
            t1 = (T > t0 ? T : t0);
        }
        // else: The segment is inside the cylinder.
    }

    return true;
}

bool RTSphereTriangle::IntersectLinePolyhedron(Sphere const& sphere,
    Vector3<float> const& velocity, Triangle const& triangle, float tMax,
    float& t0, float& t1)
{
    auto const& C = sphere.center;
    auto const& r = sphere.radius;
    auto const& V = velocity;
    auto const& N = triangle.normal;
    auto const& P = triangle.position;
    auto const& EN = triangle.edgeNormal;

    t0 = 0.0f;
    t1 = tMax;

    return
        ClipAgainstPlane(C, V, N, P[0] + r * N, t0, t1) &&
        ClipAgainstPlane(C, V, -N, P[0] - r * N, t0, t1) &&
        ClipAgainstPlane(C, V, EN[0], P[0], t0, t1) &&
        ClipAgainstPlane(C, V, EN[1], P[1], t0, t1) &&
        ClipAgainstPlane(C, V, EN[2], P[2], t0, t1);
}
