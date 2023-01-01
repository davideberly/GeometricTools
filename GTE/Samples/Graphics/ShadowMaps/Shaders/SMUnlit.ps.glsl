// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

uniform Screen
{
    vec4 screen;  // (depthBias, txSize, tySize, unused)
};

uniform sampler2D shadowSampler;

layout(location = 0) in vec4 projTCoord;
layout(location = 1) in float depth;
layout(location = 0) out vec4 pixelColor;

void main()
{
    // Generate the texture coordinates for the specified depth-map size.
    float txSize = screen.y;
    float tySize = screen.z;
    vec4 tcoords[9];
    tcoords[0] = projTCoord;
    tcoords[1] = projTCoord + vec4(-txSize,    0.0f, 0.0f, 0.0f);
    tcoords[2] = projTCoord + vec4(+txSize,    0.0f, 0.0f, 0.0f);
    tcoords[3] = projTCoord + vec4(0.0f,    -tySize, 0.0f, 0.0f);
    tcoords[4] = projTCoord + vec4(-txSize, -tySize, 0.0f, 0.0f);
    tcoords[5] = projTCoord + vec4(+txSize, -tySize, 0.0f, 0.0f);
    tcoords[6] = projTCoord + vec4(0.0f,    +tySize, 0.0f, 0.0f);
    tcoords[7] = projTCoord + vec4(-txSize, +tySize, 0.0f, 0.0f);
    tcoords[8] = projTCoord + vec4(+txSize, +tySize, 0.0f, 0.0f);
    float w = projTCoord.w;

    // Sample each of them, checking whether or not the pixel is shadowed.
    float depthBias = screen.x;
    float diff = depth - depthBias;
    float shadowTerm = 0.0f;
    for (int i = 0; i < 9; ++i)
    {
        tcoords[i] /= w;
        float rvalue = texture(shadowSampler, tcoords[i].xy).r;
        if (rvalue >= diff)
        {
            // The pixel is not in shadow.
            shadowTerm += 1.0f;
        }
    }
    shadowTerm /= 9.0f;
    pixelColor = vec4(shadowTerm, shadowTerm, shadowTerm, shadowTerm);
}
