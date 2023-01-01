// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

// The application must provide the function body via a ProgramDefines,
// and it must ensure the body compiles.
float Function(float z)
{
    return FUNCTION_BODY;
}

// The maximum number of elements must be large enough to hold all root
// bounds.  An analysis of the function might help with this, but because
// of numerical round-off errors, you should be conservative and provide
// extra storage.  For example, if a root r is exactly represented as a
// floating-point number, you get two bounds [r_prev,r] and [r,r_next],
// where {r_prev, r, r_next} are three consecutive floating-point numbers
// because Function(r_prev) < 0 = Function(r) < Function(r_next).
AppendStructuredBuffer<float4> rootBounds;

[numthreads(8, 8, 1)]
void CSMain(uint2 t : SV_DispatchThreadID)
{
    uint trailing = t.x + 4096 * t.y;
    for (uint biased = 0; biased < 255; ++biased)
    {
        uint exponent = (biased << 23);
        uint encoding0 = exponent | trailing;
        float z0 = asfloat(encoding0);
        uint encoding1 = encoding0 + 1;
        float z1 = asfloat(encoding1);

        float f0 = Function(z0);
        float f1 = Function(z1);
        if (sign(f0) * sign(f1) <= 0.0f)
        {
            rootBounds.Append(float4(z0, f0, z1, f1));
        }

        z0 = -z0;
        z1 = -z1;
        f0 = Function(z0);
        f1 = Function(z1);
        if (sign(f0) * sign(f1) <= 0.0f)
        {
            rootBounds.Append(float4(z1, f1, z0, f0));
        }
    }
}
