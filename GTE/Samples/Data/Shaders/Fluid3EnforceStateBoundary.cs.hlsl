// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#if USE_COPY_X_EDGE
Texture2D<float4> state;
RWTexture1D<float> xMin;
RWTexture1D<float> xMax;

[numthreads(1, NUM_Y_THREADS, 1)]
void CopyXEdge(uint2 c : SV_DispatchThreadID)
{
    uint2 dim;
    state.GetDimensions(dim.x, dim.y);
    xMin[c.y] = state[uint2(1, c.y)].y;
    xMax[c.y] = state[uint2(dim.x - 2, c.y)].y;
}
#endif

#if USE_WRITE_X_EDGE
Texture1D<float> xMin;
Texture1D<float> xMax;
RWTexture2D<float4> state;

[numthreads(1, NUM_Y_THREADS, 1)]
void WriteXEdge(uint2 c : SV_DispatchThreadID)
{
    uint2 dim;
    state.GetDimensions(dim.x, dim.y);
    state[uint2(0, c.y)] = float4(0.0f, xMin[c.y], 0.0f, 0.0f);
    state[uint2(dim.x - 1, c.y)] = float4(0.0f, xMax[c.y], 0.0f, 0.0f);
}
#endif

#if USE_COPY_Y_EDGE
Texture2D<float4> state;
RWTexture1D<float> yMin;
RWTexture1D<float> yMax;

[numthreads(NUM_X_THREADS, 1, 1)]
void CopyYEdge(uint2 c : SV_DispatchThreadID)
{
    uint2 dim;
    state.GetDimensions(dim.x, dim.y);
    yMin[c.x] = state[uint2(c.x, 1)].x;
    yMax[c.x] = state[uint2(c.x, dim.y - 2)].x;
}
#endif

#if USE_WRITE_Y_EDGE
Texture1D<float> yMin;
Texture1D<float> yMax;
RWTexture2D<float4> state;

[numthreads(NUM_X_THREADS, 1, 1)]
void WriteYEdge(uint2 c : SV_DispatchThreadID)
{
    uint2 dim;
    state.GetDimensions(dim.x, dim.y);
    state[uint2(c.x, 0)] = float4(yMin[c.x], 0.0f, 0.0f, 0.0f);
    state[uint2(c.x, dim.y - 1)] = float4(yMax[c.x], 0.0f, 0.0f, 0.0f);
}
#endif
