// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "TriangleIntersection.h"

bool TriangleIntersection::operator() (Vector3<float> U[3], Vector3<float> V[3]) const
{
    Vector3<float> S0[2], S1[2];
    if (Intersects(V, U, S0) && Intersects(U, V, S1))
    {
        // Theoretically, the segments lie on the same line.  A direction D
        // of the line is the Cross(NormalOf(U),NormalOf(V)).  We choose the
        // average A of the segment endpoints as the line origin.
        Vector3<float> uNormal = Cross(U[1] - U[0], U[2] - U[0]);
        Vector3<float> vNormal = Cross(V[1] - V[0], V[2] - V[0]);
        Vector3<float> D = UnitCross(uNormal, vNormal);
        Vector3<float> A = 0.25f*(S0[0] + S0[1] + S1[0] + S1[1]);

        // Each segment endpoint is of the form A + t*D.  Compute the
        // t-values to obtain I0 = [t0min,t0max] for S0 and I1 = [t1min,t1max]
        // for S1.  The segments intersect when I0 overlaps I1.  Although this
        // application acts as a "test intersection" query, in fact the
        // construction here is a "find intersection" query.
        float t00 = Dot(D, S0[0] - A), t01 = Dot(D, S0[1] - A);
        float t10 = Dot(D, S1[0] - A), t11 = Dot(D, S1[1] - A);
        auto I0 = std::minmax(t00, t01);
        auto I1 = std::minmax(t10, t11);
        return (I0.second > I1.first && I0.first < I1.second);
    }
    return false;
}

bool TriangleIntersection::Intersects(Vector3<float> U[3],
    Vector3<float> V[3], Vector3<float> segment[2]) const
{
    // Compute the plane normal for triangle U.
    Vector3<float> edge1 = U[1] - U[0];
    Vector3<float> edge2 = U[2] - U[0];
    Vector3<float> normal = UnitCross(edge1, edge2);

    // Test whether the edges of triangle V transversely intersect the
    // plane of triangle U.
    float d[3];
    int32_t positive = 0, negative = 0, zero = 0;
    for (int32_t i = 0; i < 3; ++i)
    {
        d[i] = Dot(normal, V[i] - U[0]);
        if (d[i] > 0.0f)
        {
            ++positive;
        }
        else if (d[i] < 0.0f)
        {
            ++negative;
        }
        else
        {
            ++zero;
        }
    }
    // positive + negative + zero == 3

    if (positive > 0 && negative > 0)
    {
        if (positive == 2)  // and negative == 1
        {
            if (d[0] < 0.0f)
            {
                segment[0] = (d[1] * V[0] - d[0] * V[1]) / (d[1] - d[0]);
                segment[1] = (d[2] * V[0] - d[0] * V[2]) / (d[2] - d[0]);
            }
            else if (d[1] < 0.0f)
            {
                segment[0] = (d[0] * V[1] - d[1] * V[0]) / (d[0] - d[1]);
                segment[1] = (d[2] * V[1] - d[1] * V[2]) / (d[2] - d[1]);
            }
            else  // d[2] < 0.0f
            {
                segment[0] = (d[0] * V[2] - d[2] * V[0]) / (d[0] - d[2]);
                segment[1] = (d[1] * V[2] - d[2] * V[1]) / (d[1] - d[2]);
            }
        }
        else if (negative == 2)  // and positive == 1
        {
            if (d[0] > 0.0f)
            {
                segment[0] = (d[1] * V[0] - d[0] * V[1]) / (d[1] - d[0]);
                segment[1] = (d[2] * V[0] - d[0] * V[2]) / (d[2] - d[0]);
            }
            else if (d[1] > 0.0f)
            {
                segment[0] = (d[0] * V[1] - d[1] * V[0]) / (d[0] - d[1]);
                segment[1] = (d[2] * V[1] - d[1] * V[2]) / (d[2] - d[1]);
            }
            else  // d[2] > 0.0f
            {
                segment[0] = (d[0] * V[2] - d[2] * V[0]) / (d[0] - d[2]);
                segment[1] = (d[1] * V[2] - d[2] * V[1]) / (d[1] - d[2]);
            }
        }
        else  // positive == 1, negative == 1, zero == 1
        {
            if (d[0] == 0.0f)
            {
                segment[0] = V[0];
                segment[1] = (d[2] * V[1] - d[1] * V[2]) / (d[2] - d[1]);
            }
            else if (d[1] == 0.0f)
            {
                segment[0] = V[1];
                segment[1] = (d[0] * V[2] - d[2] * V[0]) / (d[0] - d[2]);
            }
            else  // d[2] == 0.0f
            {
                segment[0] = V[2];
                segment[1] = (d[1] * V[0] - d[0] * V[1]) / (d[1] - d[0]);
            }
        }
        return true;
    }

    // Triangle V does not transversely intersect triangle U, although it is
    // possible a vertex or edge of V is just touching U.  In this case, we
    // do not call this an intersection.
    return false;
}
