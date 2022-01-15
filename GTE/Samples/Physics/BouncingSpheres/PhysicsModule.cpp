// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.14

#include "PhysicsModule.h"

PhysicsModule::PhysicsModule(size_t numSpheres, double xMin, double xMax,
    double yMin, double yMax, double zMin, double zMax)
    :
    mRigidSphere(numSpheres),
    mSphere(numSpheres),
    mMoved(numSpheres),
    mRigidPlane{},
    mPlane{},
    mPlaneContact{}
{
    // Initialize the immovable planes.
    mPlane[0] = Plane3<double>({ 1.0, 0.0, 0.0 }, xMin);
    mPlane[1] = Plane3<double>({ 0.0, 1.0, 0.0 }, yMin);
    mPlane[2] = Plane3<double>({ 0.0, 0.0, 1.0 }, zMin);
    mPlane[3] = Plane3<double>({ -1.0, 0.0, 0.0 }, -xMax);
    mPlane[4] = Plane3<double>({ 0.0, -1.0, 0.0 }, -yMax);
    mPlane[5] = Plane3<double>({ 0.0, 0.0, -1.0 }, -zMax);
    for (size_t i = 0; i < 6; ++i)
    {
        // Calling SetMass(0) indicates the object is immovable. Internally,
        // the mass is set to infinity and the inverse mass is set to zero.
        mRigidPlane[i] = std::make_shared<RigidBody<double>>();
        mRigidPlane[i]->SetMass(0.0);
        mRigidPlane[i]->SetPosition(mPlane[i].origin);
    }
}

void PhysicsModule::InitializeSphere(size_t i, double radius, double mass,
    Vector3<double> const& position, Vector3<double> const& linearMomentum)
{
    mRigidSphere[i] = std::make_shared<RigidBody<double>>();
    mRigidSphere[i]->SetMass(mass);
    mRigidSphere[i]->SetPosition(position);
    mRigidSphere[i]->SetLinearMomentum(linearMomentum);
    mRigidSphere[i]->mForce = Force;
    mRigidSphere[i]->mTorque = Torque;

    mSphere[i].center = position;
    mSphere[i].radius = radius;

    mMoved[i] = false;
}

void PhysicsModule::DoTick(double time, double deltaTime)
{
    DoCollisionDetection();
    DoCollisionResponse(time, deltaTime);
}

void PhysicsModule::SetPlaneContact(size_t sIndex, size_t bIndex,
    Vector3<double> const& spherePosition, double radius, Contact& contact)
{
    contact.B = mRigidSphere[sIndex];
    contact.A = mRigidPlane[bIndex];
    contact.N = mPlane[bIndex].normal;
    contact.P = spherePosition;
    mPlaneContact.push_back(contact);

    Vector3<double> planePosition = mRigidPlane[bIndex]->GetPosition();
    Vector3<double> contactPosition = contact.B->GetPosition();
    double offset = (bIndex < 3 ? +radius : -radius);
    int32_t axis = static_cast<int32_t>(bIndex % 3);
    contactPosition[axis] = planePosition[axis] + offset;
    contact.B->SetPosition(contactPosition);
    mMoved[sIndex] = true;
}

void PhysicsModule::DoCollisionDetection()
{
    mPlaneContact.clear();

    // Collisions with boundaries.
    size_t const numSpheres = mRigidSphere.size();
    Contact contact{};
    for (size_t i = 0; i < numSpheres; ++i)
    {
        Vector3<double> position = mRigidSphere[i]->GetPosition();
        double radius = mSphere[i].radius;
        mMoved[i] = false;

        // These checks are done in pairs under the assumption that the ball 
        // radii are smaller than the separation of opposite boundaries. Only
        // one of each opposite pair of boundaries may be touched at any time.

        // x-boundaries
        if (position[0] < mPlane[0].constant + radius)
        {
            SetPlaneContact(i, 0, position, radius, contact);
        }
        else if (position[0] > -mPlane[3].constant - radius)
        {
            SetPlaneContact(i, 3, position, radius, contact);
        }

        // y-boundaries
        if (position[1] < mPlane[1].constant + radius)
        {
            SetPlaneContact(i, 1, position, radius, contact);
        }
        else if (position[1] > -mPlane[4].constant - radius)
        {
            SetPlaneContact(i, 4, position, radius, contact);
        }

        // z-boundaries
        if (position[2] < mPlane[2].constant + radius)
        {
            SetPlaneContact(i, 2, position, radius, contact);
        }
        else if (position[2] > -mPlane[5].constant - radius)
        {
            SetPlaneContact(i, 5, position, radius, contact);
        }
    }

    // Collisions between balls.
    for (size_t i0 = 0; i0 + 1 < numSpheres; ++i0)
    {
        for (size_t i1 = i0 + 1; i1 < numSpheres; ++i1)
        {
            Vector3<double> diff = mRigidSphere[i1]->GetPosition() -
                mRigidSphere[i0]->GetPosition();
            double diffLen = Length(diff);
            double radius0 = mSphere[i0].radius;
            double radius1 = mSphere[i1].radius;
            double magnitude = diffLen - radius0 - radius1;
            if (magnitude < 0.0)
            {
                contact.A = mRigidSphere[i0];
                contact.B = mRigidSphere[i1];
                contact.N = diff / diffLen;
                Vector3<double> deltaPos = magnitude * contact.N;

                if (mMoved[i0] && !mMoved[i1])
                {
                    // Ball i0 moved but ball i1 did not.
                    mSphere[i1].center = mRigidSphere[i1]->GetPosition() - deltaPos;
                    mRigidSphere[i1]->SetPosition(mSphere[i1].center);
                }
                else if (!mMoved[i0] && mMoved[i1])
                {
                    // Ball j moved but ball i0 did not.
                    mSphere[i0].center = mRigidSphere[i0]->GetPosition() + deltaPos;
                    mRigidSphere[i0]->SetPosition(mSphere[i0].center);
                }
                else
                {
                    // Neither ball moved or both balls moved already.
                    deltaPos *= 0.5;

                    mSphere[i1].center = mRigidSphere[i1]->GetPosition() - deltaPos;
                    mRigidSphere[i1]->SetPosition(mSphere[i1].center);

                    mSphere[i0].center = mRigidSphere[i0]->GetPosition() + deltaPos;
                    mRigidSphere[i0]->SetPosition(mSphere[i0].center);
                }

                contact.P = mRigidSphere[i0]->GetPosition() + radius0 * contact.N;
                mPlaneContact.push_back(contact);
            }
        }
    }
}

void PhysicsModule::DoCollisionResponse(double time, double deltaTime)
{
    size_t const numContacts = mPlaneContact.size();
    if (numContacts > 0)
    {
        std::vector<double> preRelVelocities(numContacts);
        std::vector<double> impulseMagnitudes(numContacts);

        ComputePreimpulseVelocity(preRelVelocities);
        ComputeImpulseMagnitude(preRelVelocities, impulseMagnitudes);

        for (size_t i = 0; i < numContacts; ++i)
        {
            auto& contact = mPlaneContact[i];
            Vector3<double> impulse = impulseMagnitudes[i] * contact.N;
            contact.A->SetLinearMomentum(contact.A->GetLinearMomentum() + impulse);
            contact.B->SetLinearMomentum(contact.B->GetLinearMomentum() - impulse);
        }
    }

    for (size_t i = 0; i < mRigidSphere.size(); ++i)
    {
        mRigidSphere[i]->Update(time, deltaTime);
        mSphere[i].center = mRigidSphere[i]->GetPosition();
    }
}

void PhysicsModule::ComputePreimpulseVelocity(
    std::vector<double>& preRelVelocities)
{
    for (size_t i = 0; i < mPlaneContact.size(); ++i)
    {
        auto const& contact = mPlaneContact[i];

        Vector3<double> relA = contact.P - contact.A->GetPosition();
        Vector3<double> relB = contact.P - contact.B->GetPosition();

        Vector3<double> velA = contact.A->GetLinearVelocity() +
            Cross(contact.A->GetAngularVelocity(), relA);

        Vector3<double> velB = contact.B->GetLinearVelocity() +
            Cross(contact.B->GetAngularVelocity(), relB);

        preRelVelocities[i] = Dot(contact.N, velB - velA);
    }
}

void PhysicsModule::ComputeImpulseMagnitude(
    std::vector<double>& preRelVelocities,
    std::vector<double>& impulseMagnitudes)
{
    // The coefficient of restitution, arbitrarily selected.
    double constexpr restitution = 0.8f;

    Vector3<double> linVelDiff{}, relA{}, relB{};
    Vector3<double> AxN{}, BxN{}, JInvAxN{}, JInvBxN{};

    for (size_t i = 0; i < mPlaneContact.size(); ++i)
    {
        const Contact& contact = mPlaneContact[i];

        if (preRelVelocities[i] < 0.0f)
        {
            linVelDiff =
                contact.A->GetLinearVelocity() - contact.B->GetLinearVelocity();
            relA = contact.P - contact.A->GetPosition();
            relB = contact.P - contact.B->GetPosition();
            AxN = Cross(relA, contact.N);
            BxN = Cross(relB, contact.N);
            JInvAxN = contact.A->GetWorldInverseInertia() * AxN;
            JInvBxN = contact.B->GetWorldInverseInertia() * BxN;

            double numer = -(1.0f + restitution) * (
                Dot(contact.N, linVelDiff) +
                Dot(contact.A->GetAngularVelocity(), AxN) -
                Dot(contact.B->GetAngularVelocity(), BxN));

            double denom =
                contact.A->GetInverseMass() +
                contact.B->GetInverseMass() +
                Dot(AxN, JInvAxN) +
                Dot(BxN, JInvBxN);

            impulseMagnitudes[i] = numer / denom;
        }
        else
        {
            impulseMagnitudes[i] = 0.0;
        }
    }
}

Vector3<double> PhysicsModule::Force(
    double,
    double mass,
    Vector3<double> const&,
    Quaternion<double> const&,
    Vector3<double> const&,
    Vector3<double> const&,
    Matrix3x3<double> const&,
    Vector3<double> const&,
    Vector3<double> const&)
{
    double constexpr gravityConstant = 9.81;   // m/sec^2
    Vector3<double> gravityDirection{ 0.0, 0.0, -1.0f };
    return (mass * gravityConstant) * gravityDirection;
}

Vector3<double> PhysicsModule::Torque(
    double,
    double,
    Vector3<double> const&,
    Quaternion<double> const&,
    Vector3<double> const&,
    Vector3<double> const&,
    Matrix3x3<double> const&,
    Vector3<double> const&,
    Vector3<double> const&)
{
    // No torque is used in this application.
    return Vector3<double>::Zero();
}
