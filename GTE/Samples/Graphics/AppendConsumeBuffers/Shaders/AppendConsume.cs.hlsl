// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

struct Particle
{
    int2 location;
};

ConsumeStructuredBuffer<Particle> currentState;
AppendStructuredBuffer<Particle> nextState;

// The test code uses Dispatch(1,1,1), so 'id' is (x,0,0) with 0 <= x < 32.
[numthreads(32, 1, 1)]
void CSMain (uint3 id : SV_GroupThreadID)
{
    // Append only half the current state (the even-indexed ones.)
    Particle p = currentState.Consume();
    if ((p.location[0] & 1) == 0)
    {
        nextState.Append(p);
    }
}
