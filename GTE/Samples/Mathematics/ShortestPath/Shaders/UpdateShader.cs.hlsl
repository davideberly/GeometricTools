// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer Segment
{
    int x, y, numPixels;
};

Texture2D<float4> weights;
RWTexture2D<float> distance;
RWTexture2D<int2> previous;

[numthreads(ISIZE, 1, 1)]
void CSMain(int gt : SV_GroupThreadID)
{
    if (gt < numPixels)
    {
        int2 curr = int2(x + gt, y - gt);

        int2 prev1 = curr - int2(1, 0);
        float dmin = distance[prev1] + weights[prev1].y;
        int2 prevmin = prev1;
        int2 prev2 = curr - int2(0, 1);
        float d = distance[prev2] + weights[prev2].z;
        if (d < dmin)
        {
            dmin = d;
            prevmin = prev2;
        }
        int2 prev3 = curr - int2(1, 1);
        d = distance[prev3] + weights[prev3].w;
        if (d < dmin)
        {
            dmin = d;
            prevmin = prev3;
        }

        distance[curr] = dmin;
        previous[curr] = prevmin;
    }
}
