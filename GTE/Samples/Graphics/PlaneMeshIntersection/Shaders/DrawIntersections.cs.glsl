// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(rgba32f) uniform readonly image2D colorImage;
layout(rgba32f) uniform readonly image2D planeConstantImage;
layout(rgba32f) uniform writeonly image2D outputImage;

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    ivec2 t = ivec2(gl_GlobalInvocationID.xy);

    vec4 color = imageLoad(colorImage, t);
    vec2 planeConstant = imageLoad(planeConstantImage, t).xy;

    uint value = floatBitsToInt(color.w);
    if (value == 0x7F7FFFFF)
    {
        imageStore(outputImage, t, vec4(color.rgb, 1.0f));
        return;
    }

    float signChange = 0.0f;
    vec2 intValue = floor(planeConstant);
    vec2 diff0 = planeConstant - intValue;
    vec2 diff1 = vec2(0.0f);
    if (diff0.x > 0.0f || diff0.y > 0.0f)
    {
        ivec2 nbr;
        for (int dy = -1; dy <= 1; ++dy)
        {
            nbr.y = t.y + dy;
            for (int dx = -1; dx <= 1; ++dx)
            {
                nbr.x = t.x + dx;
                diff1 = intValue - imageLoad(planeConstantImage, nbr).xy;
                if (diff1.x >= 0.0f || diff1.y >= 0.0f)
                {
                    signChange = 1.0f;
                }
            }
        }
    }
    else
    {
        signChange = 1.0f;
    }

    vec4 outputValue = signChange * vec4(0.0f, 0.0f, 0.0f, 1.0f) +
        (1.0f - signChange) * vec4(color.rgb, 1.0f);
    imageStore(outputImage, t, outputValue);
}
