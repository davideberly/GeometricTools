// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06

#include "PhysicsModule.h"
#include <Mathematics/LinearSystem.h>

PhysicsModule::PhysicsModule(size_t numSpheres, double xMin, double xMax,
    double yMin, double yMax, double zMin, double zMax)
    :
    mRigidSphere(numSpheres),
    mRigidPlane{},
    mContacts{},
    mRestitution(0.8)  // selected arbitrarily
{
    // Create the immovable planes.
    mRigidPlane[0] = std::make_shared<RigidPlane>(Plane3<double>({ +1.0,  0.0,  0.0 }, +xMin));
    mRigidPlane[1] = std::make_shared<RigidPlane>(Plane3<double>({  0.0, +1.0,  0.0 }, +yMin));
    mRigidPlane[2] = std::make_shared<RigidPlane>(Plane3<double>({  0.0,  0.0, +1.0 }, +zMin));
    mRigidPlane[3] = std::make_shared<RigidPlane>(Plane3<double>({ -1.0,  0.0,  0.0 }, -xMax));
    mRigidPlane[4] = std::make_shared<RigidPlane>(Plane3<double>({  0.0, -1.0,  0.0 }, -yMax));
    mRigidPlane[5] = std::make_shared<RigidPlane>(Plane3<double>({  0.0,  0.0, -1.0 }, -zMax));
}

void PhysicsModule::InitializeSphere(size_t i, double radius, double massDensity,
    Vector3<double> const& center, Vector3<double> const& linearVelocity,
    Quaternion<double> const& qOrientation, Vector3<double> const& angularVelocity)
{
    mRigidSphere[i] = std::make_shared<RigidSphere>(
        Sphere3<double>(center, radius), massDensity);

    // This sets the initial linear velocity. It also sets the initial linear
    // momentum.
    mRigidSphere[i]->SetLinearVelocity(linearVelocity);

    // This sets the initial orientation. It also synchronizes the world
    // inertia tensor with the current orientation.
    mRigidSphere[i]->SetQOrientation(qOrientation, true);

    // SetAngularVelocity requires a current world inertia tensor, so it must
    // be called after SetQOrientation.
    mRigidSphere[i]->SetAngularVelocity(angularVelocity);

    mRigidSphere[i]->Force = [this, i](double, RigidBodyState<double> const& state)
    {
        // The only external force is gravity.
        double constexpr gravityConstant = 9.81;   // m/sec^2
        Vector3<double> gravityDirection{ 0.0, 0.0, -1.0 };
        Vector3<double> gravityForce =
            (state.GetMass() * gravityConstant) * gravityDirection;

        // Take into account friction when the spheres are sliding on the
        // floor.
        double constexpr epsilon = 1e-03;
        Vector3<double> frictionForce{ 0.0, 0.0, 0.0 };
        double z = state.GetPosition()[2];
        double radius = mRigidSphere[i]->GetRadius();
        if (z - radius <= epsilon)
        {
            double constexpr viscosity = 1000.0;
            Vector3<double> linearVelocity = state.GetLinearVelocity();
            Normalize(linearVelocity);
            frictionForce = -viscosity * linearVelocity;
            frictionForce[2] = 0.0;
        }

        return gravityForce + frictionForce;
    };

    mRigidSphere[i]->Torque = [this, i](double, RigidBodyState<double> const& state)
    {
        // No external torque is applied. However, take into account friction
        // when the spheres are spinning on the floor.
        double constexpr epsilon = 1e-03;
        Vector3<double> torque{ 0.0, 0.0, 0.0 };
        double z = state.GetPosition()[2];
        double radius = mRigidSphere[i]->GetRadius();
        if (z - radius <= epsilon)
        {
            double constexpr viscosity = 1000.0;
            Vector3<double> angularVelocity = state.GetAngularVelocity();
            Normalize(angularVelocity);
            Vector3<double> newAngularVelocity = -viscosity * angularVelocity;
            Vector3<double> newAngularMomentum = state.GetWorldInertia() * newAngularVelocity;
            torque = newAngularMomentum;
        }
        return torque;
    };
}

void PhysicsModule::DoTick(double time, double deltaTime)
{
    DoCollisionDetection();
    DoCollisionResponse(time, deltaTime);
}

void PhysicsModule::DoCollisionDetection()
{
    mContacts.clear();

    // Test for sphere-plane collisions. These checks are done in pairs with
    // the assumption that the sphere diameters are smaller than the distance
    // between parallel planar boundaries. In this case, only one of each
    // parallel pair of planes can be intersected at any time. Each pair of
    // parallel planes is tested in order to handle the case when a sphere
    // intersects two planes meeting at a region edge or three planes meeting
    // at a region corner. When the sphere is partially or fully outside a
    // plane, the interpenetration is removed to push the sphere back into
    // the simulation region.
    size_t const numSpheres = mRigidSphere.size();
    std::vector<bool> moved(numSpheres, false);
    double overlap{};
    for (size_t i = 0; i < numSpheres; ++i)
    {
        mRigidSphere[i]->UpdateWorldQuantities();
        auto const& sphere = mRigidSphere[i]->GetWorldSphere();

        // Test for the sphere intersecting or occurring outside an
        // x-constant plane.
        overlap = sphere.radius - mRigidPlane[0]->GetSignedDistance(sphere.center);
        if (overlap > 0.0)
        {
            moved[i] = SetSpherePlaneContact(
                mRigidSphere[i], mRigidPlane[0], overlap);
        }
        else
        {
            overlap = sphere.radius - mRigidPlane[3]->GetSignedDistance(sphere.center);
            if (overlap > 0.0)
            {
                moved[i] = SetSpherePlaneContact(
                    mRigidSphere[i], mRigidPlane[3], overlap);
            }
        }

        // Test for the sphere intersection or occurring outside a
        // y-constant plane.
        overlap = sphere.radius - mRigidPlane[1]->GetSignedDistance(sphere.center);
        if (overlap > 0.0)
        {
            moved[i] = SetSpherePlaneContact(
                mRigidSphere[i], mRigidPlane[1], overlap);
        }
        else
        {
            overlap = sphere.radius - mRigidPlane[4]->GetSignedDistance(sphere.center);
            if (overlap > 0.0)
            {
                moved[i] = SetSpherePlaneContact(
                    mRigidSphere[i], mRigidPlane[4], overlap);
            }
        }

        // Test for the sphere intersecting or occurring outside a
        // z-constant plane.
        overlap = sphere.radius - mRigidPlane[2]->GetSignedDistance(sphere.center);
        if (overlap > 0.0)
        {
            moved[i] = SetSpherePlaneContact(
                mRigidSphere[i], mRigidPlane[2], overlap);
        }
        else
        {
            overlap = sphere.radius - mRigidPlane[5]->GetSignedDistance(sphere.center);
            if (overlap > 0.0)
            {
                moved[i] = SetSpherePlaneContact(
                    mRigidSphere[i], mRigidPlane[5], overlap);
            }
        }
    }

    // Test for sphere-sphere collisions.
    for (size_t i0 = 0; i0 + 1 < numSpheres; ++i0)
    {
        auto const& sphere0 = mRigidSphere[i0]->GetWorldSphere();

        for (size_t i1 = i0 + 1; i1 < numSpheres; ++i1)
        {
            auto const& sphere1 = mRigidSphere[i1]->GetWorldSphere();

            // Test for overlap of sphere i0 and sphere i1.
            auto delta = sphere1.center - sphere0.center;
            double lengthDelta = Length(delta);
            overlap = sphere0.radius + sphere1.radius - lengthDelta;
            if (overlap > 0.0)
            {
                UndoSphereOverlap(mRigidSphere[i0], mRigidSphere[i1],
                    overlap, moved[i0], moved[i1]);
            }
        }
    }
}

void PhysicsModule::DoCollisionResponse(double time, double deltaTime)
{
    // Apply the instantaneous impulse forces at the current time.
    for (auto& contact : mContacts)
    {
        contact.ApplyImpulse();
    }

    for (size_t i = 0; i < mRigidSphere.size(); ++i)
    {
        // Solve the equations of motion.
        mRigidSphere[i]->Update(time, deltaTime);
    }
}

bool PhysicsModule::SetSpherePlaneContact(
    std::shared_ptr<RigidSphere> const& rigidSphere,
    std::shared_ptr<RigidPlane> const& rigidPlane,
    double overlap)
{
    auto const& sphere = rigidSphere->GetWorldSphere();
    auto const& plane = rigidPlane->GetPlane();

    Contact contact{};
    contact.A = rigidSphere;
    contact.B = rigidPlane;
    contact.P = sphere.center + overlap * plane.normal;
    contact.N = plane.normal;
    contact.restitution = mRestitution;
    mContacts.push_back(contact);

    // Move the intersecting sphere to be just touching the plane.
    rigidSphere->SetPosition(contact.P);
    rigidSphere->UpdateWorldQuantities();
    return true;
}

void PhysicsModule::UndoSphereOverlap(
    std::shared_ptr<RigidSphere> const& rigidSphere0,
    std::shared_ptr<RigidSphere> const& rigidSphere1,
    double overlap, bool moved0, bool moved1)
{
    auto const& sphere0 = rigidSphere0->GetWorldSphere();
    auto const& sphere1 = rigidSphere1->GetWorldSphere();
    auto normal = sphere1.center - sphere0.center;
    Normalize(normal);

    Contact contact{};
    contact.A = rigidSphere0;
    contact.B = rigidSphere1;
    contact.N = normal;
    contact.restitution = mRestitution;
    auto offset = overlap * contact.N;

    if (moved0 && !moved1)
    {
        // Sphere i0 moved but sphere i1 did not.
        rigidSphere1->SetPosition(rigidSphere1->GetPosition() + offset);
        rigidSphere1->UpdateWorldQuantities();
    }
    else if (!moved0 && moved1)
    {
        // Sphere i1 moved but sphere i0 did not.
        rigidSphere0->SetPosition(rigidSphere0->GetPosition() - offset);
        rigidSphere0->UpdateWorldQuantities();
    }
    else
    {
        // Neither sphere moved or both spheres moved. Avoid bias
        // by moving both spheres half the offset.
        offset *= 0.5;
        rigidSphere1->SetPosition(rigidSphere1->GetPosition() + offset);
        rigidSphere1->UpdateWorldQuantities();
        rigidSphere0->SetPosition(rigidSphere0->GetPosition() - offset);
        rigidSphere0->UpdateWorldQuantities();
    }

    contact.P = rigidSphere0->GetPosition() + sphere0.radius * contact.N;
    mContacts.push_back(contact);
}
