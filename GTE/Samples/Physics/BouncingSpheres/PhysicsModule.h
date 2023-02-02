// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06

#include "RigidPlane.h"
#include "RigidSphere.h"
#include <memory>
#include <vector>
using namespace gte;

// The PhysicsModule is an implementation of the collision detection and
// impulse-based collision response described in "Game Physics, 2nd edition."
// The code comments have relevant equation numbers from the book. However,
// the DoCollisionResponse function uses a variation for computing impulses,
// described in
//   https://www.geometrictools.com/Documentation/ComputingImpulsiveForces.pdf

class PhysicsModule
{
public:
    PhysicsModule(size_t numSpheres, double xMin, double xMax, double yMin,
        double yMax, double zMin, double zMax);

    // This function must be called for each of the numSpheres sphere objects
    // before starting the simulation.
    void InitializeSphere(size_t i, double radius, double massDensity,
        Vector3<double> const& position, Vector3<double> const& linearVelocity,
        Quaternion<double> const& qOrientation, Vector3<double> const& angularVelocity);

    inline size_t GetNumSpheres() const
    {
        return mRigidSphere.size();
    }

    // The input must satisfy 0 <= i < 6 where the extremes were passed to the
    // constructor. The normals are directed into the interior of the
    // simulation region. The planes are immovable.
    //   plane[0]: back wall, Dot((1,0,0),(x,y,z)) = xMin
    //   plane[1]: side1 wall, Dot((0,1,0),(x,y,z)) = yMin
    //   plane[2]: floor, Dot((0,0,1),(x,y,z)) = zMin
    //   plane[3]: front wall, Dot((-1,0,0),(x,y,z)) = -xMax
    //   plane[4]: side2 wall, Dot((0,-1,0),(x,y,z)) = -yMax
    //   plane[5]: ceiling, Dot((0,0,-1),(x,y,z)) = -zMax
    inline Plane3<double> GetPlane(size_t i) const
    {
        return mRigidPlane[i]->GetPlane();
    }

    // The input must satisfy 0 <= i < numSpheres where the upper bound was
    // passed to the constructor.
    inline Sphere3<double> GetWorldSphere(size_t i) const
    {
        return mRigidSphere[i]->GetWorldSphere();
    }

    inline Matrix3x3<double> const& GetOrientation(size_t i) const
    {
        return mRigidSphere[i]->GetROrientation();
    }

    // Execute the physics simulation. The caller of this function maintains
    // the physics clock.
    void DoTick(double time, double deltaTime);

private:
    using Contact = RigidBodyContact<double>;

    void DoCollisionDetection();
    void DoCollisionResponse(double time, double deltaTime);

    bool SetSpherePlaneContact(std::shared_ptr<RigidSphere> const& rigidSphere,
        std::shared_ptr<RigidPlane> const& rigidPlane, double overlap);

    void UndoSphereOverlap(
        std::shared_ptr<RigidSphere> const& rigidSphere0,
        std::shared_ptr<RigidSphere> const& rigidSphere1,
        double overlap, bool moved0, bool moved1);

    // Physical representations of solid spheres.
    std::vector<std::shared_ptr<RigidSphere>> mRigidSphere;

    // Physical representation of planar boundaries.
    std::array<std::shared_ptr<RigidPlane>, 6> mRigidPlane;

    // Contact points during one pass of the physical simulation.
    std::vector<Contact> mContacts;
    double mRestitution;
};
