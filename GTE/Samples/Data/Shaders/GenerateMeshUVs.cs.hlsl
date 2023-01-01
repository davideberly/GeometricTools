// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer Bounds
{
    int2 bound;
    int numBoundaryEdges;
    int numInputs;
};

struct VertexGraphData
{
    int adjacent;
    Real weight;
};

StructuredBuffer<int3> vertexGraph;
StructuredBuffer<VertexGraphData> vertexGraphData;
StructuredBuffer<int> orderedVertices;
StructuredBuffer<Real2> inTCoords;
RWStructuredBuffer<Real2> outTCoords;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 t : SV_DispatchThreadID)
{
    int index = t.x + bound.x * t.y;
    if (step(index, numInputs-1))
    {
        int v = orderedVertices[numBoundaryEdges + index];
        int2 range = vertexGraph[v].yz;
        Real2 tcoord = Real2(0, 0);
        Real weightSum = 0;
        for (int j = 0; j < range.y; ++j)
        {
            VertexGraphData data = vertexGraphData[range.x + j];
            weightSum += data.weight;
            tcoord += data.weight * inTCoords[data.adjacent];
        }
        tcoord /= weightSum;
        outTCoords[v] = tcoord;
    }
}
