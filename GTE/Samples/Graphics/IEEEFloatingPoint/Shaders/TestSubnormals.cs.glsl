// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

buffer inBuffer { REAL data[]; } inBufferSB;  // Two subnormal numbers.
buffer outBuffer { REAL data[]; } outBufferSB;  // The sum of inputs, supposed to be subnormal.

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    outBufferSB.data[0] = inBufferSB.data[0] + inBufferSB.data[1];
}
