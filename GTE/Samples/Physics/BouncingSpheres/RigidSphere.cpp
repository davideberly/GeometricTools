// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06

#include "RigidSphere.h"
using namespace gte;

RigidSphere::RigidSphere(Sphere3<double> const& sphere, double massDensity)
    :
    RigidBody<double>{},
    mWorldSphere({ 0.0, 0.0, 0.0 }, sphere.radius)
{
    double rCubed = sphere.radius * sphere.radius * sphere.radius;
    double volume = 4.0 * GTE_C_PI * rCubed / 3.0;
    double mass = massDensity * volume;
    Matrix3x3<double> bodyInertia = massDensity * Matrix3x3<double>::Identity();
    SetMass(mass);
    SetBodyInertia(bodyInertia);
    SetPosition(sphere.center);
    UpdateWorldQuantities();
}

void RigidSphere::UpdateWorldQuantities()
{
    mWorldSphere.center = GetPosition();
}
