// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Mathematics/RigidBody.h>
#include <Mathematics/Hypersphere.h>
using namespace gte;

class RigidSphere : public RigidBody<double>
{
public:
    RigidSphere(Sphere3<double> const& sphere, double massDensity);
    virtual ~RigidSphere() = default;

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
    Sphere3<double> mWorldSphere;
};


