// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Vector3.h>
using namespace gte;

class TriangleIntersection
{
public:
    bool operator() (Vector3<float> U[3], Vector3<float> V[3]) const;

private:
    // The first input is the plane (determined by triangle U) and the
    // second input is triangle.
    bool Intersects(Vector3<float> U[3], Vector3<float> V[3],
        Vector3<float> segment[2]) const;
};
