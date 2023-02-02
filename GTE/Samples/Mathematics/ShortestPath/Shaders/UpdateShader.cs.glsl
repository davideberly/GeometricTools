// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform Segment
{
    int x, y, numPixels;
};

layout(rgba32f) uniform readonly image2D weights;
layout(r32f) restrict uniform image2D distance;
layout(rg32i) uniform writeonly iimage2D previous;

layout (local_size_x = ISIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
    int gt = int(gl_GlobalInvocationID.xy);
    if (gt < numPixels)
    {
        ivec2 curr = ivec2(x + gt, y - gt);

        ivec2 prev1 = curr - ivec2(1, 0);
        float dmin = imageLoad(distance, prev1).x + imageLoad(weights, prev1).y;
        ivec2 prevmin = prev1;
        ivec2 prev2 = curr - ivec2(0, 1);
        float d = imageLoad(distance, prev2).x + imageLoad(weights, prev2).z;
        if (d < dmin)
        {
            dmin = d;
            prevmin = prev2;
        }
        ivec2 prev3 = curr - ivec2(1, 1);
        d = imageLoad(distance, prev3).x + imageLoad(weights, prev3).w;
        if (d < dmin)
        {
            dmin = d;
            prevmin = prev3;
        }

        imageStore(distance, curr, vec4(dmin, 0.0f, 0.0f, 0.0f));
        imageStore(previous, curr, ivec4(prevmin, 0, 0));
    }
}
