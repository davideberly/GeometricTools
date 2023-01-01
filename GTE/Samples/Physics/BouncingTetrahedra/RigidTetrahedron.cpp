// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06

#include "RigidTetrahedron.h"

RigidTetrahedron::RigidTetrahedron(Tetrahedron3<double> const& bodyTetrahedron,
    Vector3<double> const& position, double massDensity)
    :
    RigidBody<double>{},
    mBodyTetrahedron(bodyTetrahedron),
    mWorldTetrahedron(bodyTetrahedron),
    mWorldCentroid{ 0.0, 0.0, 0.0 },
    mWorldSphere({ 0.0, 0.0, 0.0 }, 0.0)
{
    // The computation of mass and body inertia tensor assumes the mass
    // density is 1. The output mass and body inertia tensor must be
    // multiplied by the mass density to obtain the correct values.
    std::array<size_t, 12> faceIndices = Tetrahedron3<double>::GetAllFaceIndices();
    std::array<int32_t, 12> indices{};
    for (size_t i = 0; i < 12; ++i)
    {
        indices[i] = static_cast<int32_t>(faceIndices[i]);
    }
    double mass{};
    Vector3<double> centroid{};
    Matrix3x3<double> bodyInertia{};
    ComputeMassProperties(mBodyTetrahedron.v.data(), 4, indices.data(), true,
        mass, centroid, bodyInertia);
    mass *= massDensity;
    bodyInertia *= massDensity;

    // Compute a bounding sphere for the tetrahedon. The center of the
    // bounding sphere is the center of mass. The radius is the maximum
    // distance from the center of mass to the vertices.
    mWorldSphere.center = centroid;
    mWorldSphere.radius = 0.0;
    for (size_t j = 0; j < 4; ++j)
    {
        double length = Length(mBodyTetrahedron.v[j] - centroid);
        if (length > mWorldSphere.radius)
        {
            mWorldSphere.radius = length;
        }
    }

    SetMass(mass);
    SetBodyInertia(bodyInertia);
    SetPosition(position);
    UpdateWorldQuantities();
}

void RigidTetrahedron::UpdateWorldQuantities()
{
    auto const& rotate = GetROrientation();
    auto const& translate = GetPosition();
    for (size_t j = 0; j < 4; ++j)
    {
        mWorldTetrahedron.v[j] = rotate * mBodyTetrahedron.v[j] + translate;
    }
    mWorldCentroid = mWorldTetrahedron.ComputeCentroid();
    mWorldSphere.center = translate;
}
