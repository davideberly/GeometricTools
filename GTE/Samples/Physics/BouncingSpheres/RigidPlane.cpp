// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06

#include "RigidPlane.h"

RigidPlane::RigidPlane(Plane3<double> const& plane)
    :
    RigidBody<double>{},
    mPlane(plane)
{
    SetMass(0.0);
    SetBodyInertia(Matrix3x3<double>::Zero());
    SetPosition(mPlane.origin);
}
