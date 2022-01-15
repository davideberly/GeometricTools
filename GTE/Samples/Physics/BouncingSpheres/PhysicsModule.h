// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.14

#include <Mathematics/Hyperplane.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/RigidBody.h>
#include <memory>
#include <vector>
using namespace gte;

class PhysicsModule
{
public:
    PhysicsModule(size_t numSpheres, double xMin, double xMax, double yMin,
        double yMax, double zMin, double zMax);

    // This function must be called for each of the numSpheres sphere objects
    // before starting the simulation.
    void InitializeSphere(size_t i, double radius, double mass,
        Vector3<double> const& position, Vector3<double> const& linearMomentum);

    inline size_t GetNumSpheres() const
    {
        return mSphere.size();
    }

    // The input must satisfy 0 <= i < 6 where the extremes were passed to the
    // constructor.
    //   plane[0]: back wall (x = xMin)
    //   plane[1]: side1 wall (y = yMin)
    //   plane[2]: floor (z = zMin)
    //   plane[3]: front wall (-x = -xMax)
    //   plane[4]: side2 wall (-y = -yMax)
    //   plane[5]: ceiling (-z = -zMax)
    inline Plane3<double> const& GetPlane(size_t i) const
    {
        return mPlane[i];
    }

    // The input must satisfy 0 <= i < numSpheres where the upper bound was
    // passed to the constructor.
    inline Sphere3<double> const& GetSphere(size_t i) const
    {
        return mSphere[i];
    }

    // Execute the physics simulation. The caller of this function maintains
    // the physics clock.
    void DoTick(double time, double deltaTime);

private:
    // Support for the physics simulation including the external forces
    // (Force function) and torques (Torque function). TODO: Make the force
    // and torque/ functions of type std::function? This requires modifying
    // the RigidBody class.
    struct Contact
    {
        Contact()
            :
            A{},
            B{},
            P(Vector3<double>::Zero()),
            N(Vector3<double>::Zero())
        {
        }

        // ball containing face
        std::shared_ptr<RigidBody<double>> A;

        // ball containing vertex
        std::shared_ptr<RigidBody<double>> B;

        // contact point
        Vector3<double> P;

        // outward unit-length normal to face
        Vector3<double> N;
    };

    void SetPlaneContact(size_t sIndex, size_t bIndex,
        Vector3<double> const& spherePosition, double radius,
        Contact& contact);

    void DoCollisionDetection();

    void DoCollisionResponse(double time, double deltaTime);

    void ComputePreimpulseVelocity(std::vector<double>& preRelVelocities);

    void ComputeImpulseMagnitude(std::vector<double>& preRelVelocities,
        std::vector<double>& impulseMagnitudes);

    static Vector3<double> Force(
        double time,
        double mass,
        Vector3<double> const& position,
        Quaternion<double> const& quatOrient,
        Vector3<double> const& linearMomentum,
        Vector3<double> const& angularMomentum,
        Matrix3x3<double> const& rotOrient,
        Vector3<double> const& linearVelocity,
        Vector3<double> const& angularVelocity);

    static Vector3<double> Torque(
        double time,
        double mass,
        Vector3<double> const& position,
        Quaternion<double> const& quatOrient,
        Vector3<double> const& linearMomentum,
        Vector3<double> const& angularMomentum,
        Matrix3x3<double> const& rotOrient,
        Vector3<double> const& linearVelocity,
        Vector3<double> const& angularVelocity);


    // Physical representations of solid spheres. The rigid spheres are the
    // rigid bodies that move.
    std::vector<std::shared_ptr<RigidBody<double>>> mRigidSphere;
    std::vector<Sphere3<double>> mSphere;
    std::vector<bool> mMoved;

    // Physical representation of planar boundaries (floor, ceiling, walls).
    // The floor is z = zMin and the ceiling is z = zMax. The side walls are
    // y = yMin and y = yMax. The front wall is x = xMax and the back wall is
    // xMinx = 1. The assumption is that the observer is on the +x axis
    // viewing the simulation in the direction (-1,0,0) and having up-vector
    // (0,0,1).
    //
    // The plane order is
    //   { x = xMin, y = yMin, z = zMin, x = xMax, y = yMax, z = zMax }
    // The plane normals are directed into the interior of the region. The
    // planes are immovable.
    std::array<std::shared_ptr<RigidBody<double>>, 6> mRigidPlane;
    std::array<Plane3<double>, 6> mPlane;

    // Contact points during one pass of the physical simulation.
    std::vector<Contact> mPlaneContact;
};
