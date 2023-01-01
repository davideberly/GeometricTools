// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform Bounds
{
    ivec2 bound;
    int numBoundaryEdges;
    int numInputs;
};

struct VertexGraphData
{
    int adjacent;
    Real weight;
};

buffer vertexGraph { ivec3 data[]; } vertexGraphSB;
buffer vertexGraphData { VertexGraphData data[]; } vertexGraphDataSB;
buffer orderedVertices { int data[]; } orderedVerticesSB;
buffer inTCoords { Real2 data[]; } inTCoordsSB;
buffer outTCoords { Real2 data[]; } outTCoordsSB;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 t = ivec2(gl_GlobalInvocationID.xy);
    int index = t.x + bound.x * t.y;
    if (step(index, numInputs-1) == 1)
    {
        int v = orderedVerticesSB.data[numBoundaryEdges + index];
        ivec2 range = vertexGraphSB.data[v].yz;
        Real2 tcoord = Real2(0, 0);
        Real weightSum = 0;
        for (int j = 0; j < range.y; ++j)
        {
            VertexGraphData vgd = vertexGraphDataSB.data[range.x + j];
            weightSum += vgd.weight;
            tcoord += vgd.weight * inTCoordsSB.data[vgd.adjacent];
        }
        tcoord /= weightSum;
        outTCoordsSB.data[v] = tcoord;
    }
}
