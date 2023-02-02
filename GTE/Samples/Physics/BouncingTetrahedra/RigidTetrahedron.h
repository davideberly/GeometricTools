// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06

#include <Mathematics/RigidBody.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Tetrahedron3.h>
#include <Mathematics/PolyhedralMassProperties.h>
#include <array>
using namespace gte;

class RigidTetrahedron : public RigidBody<double>
{
public:
    RigidTetrahedron(Tetrahedron3<double> const& bodyTetrahedron,
        Vector3<double> const& position, double massDensity);

    virtual ~RigidTetrahedron() = default;

    inline Tetrahedron3<double> const& GetBodyTetrahedron() const
    {
        return mBodyTetrahedron;
    }

    inline Tetrahedron3<double> const& GetWorldTetrahedron() const
    {
        return mWorldTetrahedron;
    }

    inline Vector3<double> const& GetWorldCentroid() const
    {
        return mWorldCentroid;
    }

    inline Sphere3<double> const& GetWorldSphere() const
    {
        return mWorldSphere;
    }

    inline double GetRadius() const
    {
        return mWorldSphere.radius;
    }

    void UpdateWorldQuantities();

private:

    Tetrahedron3<double> mBodyTetrahedron;
    Tetrahedron3<double> mWorldTetrahedron;
    Vector3<double> mWorldCentroid;
    Sphere3<double> mWorldSphere;
};
