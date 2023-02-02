// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform Parameters
{
    mat4 wMatrix0, wMatrix1;
    uint numTriangles0, numTriangles1;
};

// Each buffer stores triples to represent a triangle.  The color0 values
// are initialized to 0 and the color1 values are initialized to 1 before
// the compute shader is called.  On output the color0 values are 0 or 2
// and the color1 values are 1 or 3.
buffer vertices0 { vec4 data[]; } vertices0SB;
buffer vertices1 { vec4 data[]; } vertices1SB;
buffer color0 { uint data[]; } color0SB;
buffer color1 { uint data[]; } color1SB;

bool Intersects(vec3 U[3], vec3 V[3], out vec3 segment[2])
{
    // Compute the plane normal for triangle U.
    vec3 edge1 = U[1] - U[0];
    vec3 edge2 = U[2] - U[0];
    vec3 normal = cross(edge1, edge2);
    normalize(normal);

    // Test whether the edges of triangle V transversely intersect the
    // plane of triangle U.
    float d[3];
    int positive = 0, negative = 0, zero = 0;
    for (int i = 0; i < 3; ++i)
    {
        d[i] = dot(normal, V[i] - U[0]);
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
    else
    {
        // Triangle V does not transversely intersect triangle U, although it
        // is possible a vertex or edge of V is just touching U.  In this case,
        // we do not call this an intersection.
        return false;
    }
}

bool TrianglesIntersect(vec3 U[3], vec3 V[3])
{
    vec3 S0[2], S1[2];
    if (Intersects(V, U, S0) && Intersects(U, V, S1))
    {
        // Theoretically, the segments lie on the same line.  A direction D
        // of the line is the Cross(NormalOf(U),NormalOf(V)).  We choose the
        // average A of the segment endpoints as the line origin.
        vec3 uNormal = cross(U[1] - U[0], U[2] - U[0]);
        vec3 vNormal = cross(V[1] - V[0], V[2] - V[0]);
        vec3 A = 0.25f*(S0[0] + S0[1] + S1[0] + S1[1]);
        vec3 D = cross(uNormal, vNormal);
        normalize(D);

        // Each segment endpoint is of the form A + t*D.  Compute the
        // t-values to obtain I0 = [t0min,t0max] for S0 and I1 = [t1min,t1max]
        // for S1.  The segments intersect when I0 overlaps I1.  Although this
        // application acts as a "test intersection" query, in fact the
        // construction here is a "find intersection" query.
        float t00 = dot(D, S0[0] - A), t01 = dot(D, S0[1] - A);
        float t10 = dot(D, S1[0] - A), t11 = dot(D, S1[1] - A);
        float i0min = min(t00, t01);
        float i0max = max(t00, t01);
        float i1min = min(t10, t11);
        float i1max = max(t10, t11);
        return (i0max > i1min && i0min < i1max);
    }
    return false;
}

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 dt = ivec2(gl_GlobalInvocationID.xy);

    if (dt.x < numTriangles0 && dt.y < numTriangles1)
    {
        vec3 V0[3], V1[3];
#if GTE_USE_MAT_VEC
        for (int j = 0; j < 3; ++j)
        {
            V0[j] = (wMatrix0 * vertices0SB.data[3 * dt.x + j]).xyz;
            V1[j] = (wMatrix1 * vertices1SB.data[3 * dt.y + j]).xyz;
        }
#else
        for (int j = 0; j < 3; ++j)
        {
            V0[j] = (vertices0SB.data[3 * dt.x + j] * wMatrix0).xyz;
            V1[j] = (vertices1SB.data[3 * dt.y + j] * wMatrix1).xyz;
        }
#endif

        if (TrianglesIntersect(V0, V1))
        {
            for (int j = 0; j < 3; ++j)
            {
                atomicCompSwap(color0SB.data[3 * dt.x + j], 0, 2);
                atomicCompSwap(color1SB.data[3 * dt.y + j], 1, 3);
            }
        }
    }
}
