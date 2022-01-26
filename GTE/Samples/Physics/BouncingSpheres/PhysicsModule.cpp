// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.26

#include "PhysicsModule.h"
#include <Mathematics/LinearSystem.h>

PhysicsModule::PhysicsModule(size_t numSpheres, double xMin, double xMax,
    double yMin, double yMax, double zMin, double zMax)
    :
    mRigidSphere(numSpheres),
    mSphere(numSpheres),
    mMoved(numSpheres),
    mRigidPlane{},
    mPlane{},
    mContacts{}
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
        // Setting the mass to zero indicates the object is immovable.
        mRigidPlane[i] = std::make_shared<RigidBody<double>>();
        mRigidPlane[i]->SetMass(0.0);
        mRigidPlane[i]->SetBodyInertia(Matrix3x3<double>::Zero());
        mRigidPlane[i]->SetPosition(mPlane[i].origin);
    }
}

void PhysicsModule::InitializeSphere(size_t i, double radius, double massDensity,
    Vector3<double> const& position, Vector3<double> const& linearVelocity,
    Quaternion<double> const& qOrientation, Vector3<double> const& angularVelocity)
{
    mRigidSphere[i] = std::make_shared<RigidBody<double>>();
    double mass = massDensity * (4.0 * GTE_C_PI / 3.0) * radius * radius * radius;
    Matrix3x3<double> bodyInertia = massDensity * Matrix3x3<double>::Identity();
    mRigidSphere[i]->SetMass(mass);
    mRigidSphere[i]->SetBodyInertia(bodyInertia);
    mRigidSphere[i]->SetPosition(position);
    mRigidSphere[i]->SetLinearVelocity(linearVelocity);

    // This sets the initial orientation. It also synchronizes the world
    // inertia tensor with the current orientation.
    mRigidSphere[i]->SetQOrientation(qOrientation, true);

    // SetAngularMomentum requires a current world inertia tensor, so it
    // must be called after SetQOrientation.
    mRigidSphere[i]->SetAngularVelocity(angularVelocity);

    // Synchronize the graphics sphere with the physics sphere.
    mSphere[i].center = position;
    mSphere[i].radius = radius;

    mMoved[i] = false;

    mRigidSphere[i]->Force = [this, i](double, RigidBodyState<double> const& state)
    {
        // The only external force is gravity.
        double constexpr gravityConstant = 9.81;   // m/sec^2
        Vector3<double> gravityDirection{ 0.0, 0.0, -1.0 };
        Vector3<double> gravityForce =
            (state.GetMass() * gravityConstant) * gravityDirection;

        // Take into account friction when the spheres are sliding on the
        // flooor.
        double constexpr epsilon = 1e-03;
        Vector3<double> frictionForce{ 0.0, 0.0, 0.0 };
        if (state.GetPosition()[2] - mSphere[i].radius <= epsilon)
        {
            double constexpr viscosity = 1.0;
            auto const& linearVelocity = state.GetLinearVelocity();
            double speed = Length(linearVelocity);
            frictionForce = -viscosity * speed * linearVelocity;
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
        if (state.GetPosition()[2] - mSphere[i].radius <= epsilon)
        {
            double constexpr viscosity = 1.0;
            auto const& angularVelocity = state.GetAngularVelocity();
            double speed = Length(angularVelocity);
            Vector3<double> newAngularVelocity = -viscosity * speed * angularVelocity;
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
    for (size_t i = 0; i < numSpheres; ++i)
    {
        Vector3<double> center = mRigidSphere[i]->GetPosition();
        double radius = mSphere[i].radius;
        mMoved[i] = false;

        // Test for the sphere intersecting or occurring outside an
        // x-constant plane.
        double xminCenter = mPlane[0].constant + radius;
        if (center[0] < xminCenter)
        {
            Contact contact{};
            contact.A = mRigidSphere[i];
            contact.B = mRigidPlane[0];
            contact.P = { xminCenter, center[1], center[2] };
            contact.N = mPlane[0].normal;
            mContacts.push_back(contact);
            mRigidSphere[i]->SetPosition(contact.P);
            mMoved[i] = true;

            // Synchronize the graphics sphere with the physics sphere.
            mSphere[i].center = contact.P;
        }
        else
        {
            double xmaxCenter = -mPlane[3].constant - radius;
            if (center[0] > xmaxCenter)
            {
                Contact contact{};
                contact.A = mRigidSphere[i];
                contact.B = mRigidPlane[3];
                contact.P = { xmaxCenter, center[1], center[2] };
                contact.N = mPlane[3].normal;
                mContacts.push_back(contact);
                mRigidSphere[i]->SetPosition(contact.P);
                mMoved[i] = true;

                // Synchronize the graphics sphere with the physics sphere.
                mSphere[i].center = contact.P;
            }
        }

        // Test for the sphere intersection or occurring outside a
        // y-constant plane.
        double yminCenter = mPlane[1].constant + radius;
        if (center[1] < yminCenter)
        {
            Contact contact{};
            contact.A = mRigidSphere[i];
            contact.B = mRigidPlane[1];
            contact.P = { center[0], yminCenter, center[2] };
            contact.N = mPlane[1].normal;
            mContacts.push_back(contact);
            mRigidSphere[i]->SetPosition(contact.P);
            mMoved[i] = true;

            // Synchronize the graphics sphere with the physics sphere.
            mSphere[i].center = contact.P;
        }
        else
        {
            double ymaxCenter = -mPlane[4].constant - radius;
            if (center[1] > ymaxCenter)
            {
                Contact contact{};
                contact.A = mRigidSphere[i];
                contact.B = mRigidPlane[4];
                contact.P = { center[0], ymaxCenter, center[2] };
                contact.N = mPlane[4].normal;
                mContacts.push_back(contact);
                mRigidSphere[i]->SetPosition(contact.P);
                mMoved[i] = true;

                // Synchronize the graphics sphere with the physics sphere.
                mSphere[i].center = contact.P;
            }
        }

        // Test for the sphere intersecting or occurring outside a
        // z-constant plane.
        double zminCenter = mPlane[2].constant + radius;
        if (center[2] < zminCenter)
        {
            Contact contact{};
            contact.A = mRigidSphere[i];
            contact.B = mRigidPlane[2];
            contact.P = { center[0], center[1], zminCenter };
            contact.N = mPlane[2].normal;
            mContacts.push_back(contact);
            mRigidSphere[i]->SetPosition(contact.P);
            mMoved[i] = true;

            // Synchronize the graphics sphere with the physics sphere.
            mSphere[i].center = contact.P;
        }
        else
        {
            double zmaxCenter = -mPlane[5].constant - radius;
            if (center[2] > zmaxCenter)
            {
                Contact contact{};
                contact.A = mRigidSphere[i];
                contact.B = mRigidPlane[5];
                contact.P = { center[0], center[1], zmaxCenter };
                contact.N = mPlane[5].normal;
                mContacts.push_back(contact);
                mRigidSphere[i]->SetPosition(contact.P);
                mMoved[i] = true;

                // Synchronize the graphics sphere with the physics sphere.
                mSphere[i].center = contact.P;
            }
        }
    }

    // Test for sphere-sphere collisions.
    for (size_t i0 = 0; i0 + 1 < numSpheres; ++i0)
    {
        auto const& center0 = mRigidSphere[i0]->GetPosition();
        double radius0 = mSphere[i0].radius;

        for (size_t i1 = i0 + 1; i1 < numSpheres; ++i1)
        {
            auto const& center1 = mRigidSphere[i1]->GetPosition();
            double radius1 = mSphere[i1].radius;

            // Test for overlap of sphere i0 and sphere i1.
            Vector3<double> delta = center1 - center0;
            double lengthDelta = Length(delta);
            double overlap = lengthDelta - radius0 - radius1;
            if (overlap < 0.0)
            {
                Contact contact{};
                contact.A = mRigidSphere[i0];
                contact.B = mRigidSphere[i1];
                contact.N = delta / lengthDelta;
                Vector3<double> offset = overlap * contact.N;

                if (mMoved[i0] && !mMoved[i1])
                {
                    // Sphere i0 moved but sphere i1 did not.
                    auto newCenter1 = center1 - offset;
                    mRigidSphere[i1]->SetPosition(newCenter1);

                    // Synchronize the graphics sphere with the physics
                    // sphere.
                    mSphere[i1].center = newCenter1;
                }
                else if (!mMoved[i0] && mMoved[i1])
                {
                    // Sphere i1 moved but sphere i0 did not.
                    auto newCenter0 = center0 + offset;
                    mRigidSphere[i0]->SetPosition(newCenter0);

                    // Synchronize the graphics sphere with the physics
                    // sphere.
                    mSphere[i0].center = newCenter0;
                }
                else
                {
                    // Neither sphere moved or both spheres moved. Avoid bias
                    // by moving both spheres half the offset.
                    offset *= 0.5;
                    auto newCenter1 = center1 - offset;
                    auto newCenter0 = center0 + offset;
                    mRigidSphere[i1]->SetPosition(newCenter1);
                    mRigidSphere[i0]->SetPosition(newCenter0);

                    // Synchronize the graphics spheres with the physics
                    // spheres.
                    mSphere[i1].center = newCenter1;
                    mSphere[i0].center = newCenter0;
                }

                contact.P = mRigidSphere[i0]->GetPosition() + radius0 * contact.N;
                mContacts.push_back(contact);
            }
        }
    }
}

void PhysicsModule::DoCollisionResponse(double time, double deltaTime)
{
    // For a description of the construction of impulse forces, see
    // https://www.geometrictools.com/Documentation/ComputingImpulsiveForces.pdf

    // The coefficient of restitution, arbitrarily selected.
    double constexpr restitution = 0.8;

    // Apply the instantaneous impulse forces at the current time.
    for (size_t i = 0; i < mContacts.size(); ++i)
    {
        auto const& contact = mContacts[i];

        // The contact point and contact normal.
        auto const& P0 = contact.P;
        auto const& N0 = contact.N;

        // The positions of the centers of mass.
        auto const& XA = contact.A->GetPosition();
        auto const& XB = contact.B->GetPosition();

        // The location of the contact points relative to the centers of mass.
        auto rA = P0 - XA;
        auto rB = P0 - XB;

        // The preimpulse linear velocities of the centers of mass.
        auto const& linvelANeg = contact.A->GetLinearVelocity();
        auto const& linvelBNeg = contact.B->GetLinearVelocity();

        // The preimpulse angular velocities about the centers of mass.
        auto const& angvelANeg = contact.A->GetAngularVelocity();
        auto const& angvelBNeg = contact.B->GetAngularVelocity();

        // The preimpulse velocities of P0.
        auto velANeg = linvelANeg + Cross(angvelANeg, rA);
        auto velBNeg = linvelBNeg + Cross(angvelBNeg, rB);
        auto velDiffNeg = velANeg - velBNeg;

        // The preimpulse linear momenta of the centers of mass.
        Vector3<double> linmomANeg = contact.A->GetLinearMomentum();
        Vector3<double> linmomBNeg = contact.B->GetLinearMomentum();

        // The preimpulse angular momenta about the centers of mass.
        Vector3<double> angmomANeg = contact.A->GetAngularMomentum();
        Vector3<double> angmomBNeg = contact.B->GetAngularMomentum();

        // The inverse masses, inverse world inertia tensors and quadratic
        // forms associated with these tensors.
        double const& invMA = contact.A->GetInverseMass();
        double const& invMB = contact.B->GetInverseMass();
        double sumInvMasses = invMA + invMB;
        auto const& invJA = contact.A->GetWorldInverseInertia();
        auto const& invJB = contact.B->GetWorldInverseInertia();

        Vector3<double> T0 = velDiffNeg - Dot(N0, velDiffNeg) * N0;
        Normalize(T0);
        if (T0 != Vector3<double>::Zero())
        {
            // T0 is tangent at the contact point, unit length and
            // perpendicular to N0.
            Vector3<double> T1 = Cross(N0, T0);
            auto rAxN0 = Cross(rA, N0);
            auto rAxT0 = Cross(rA, T0);
            auto rAxT1 = Cross(rA, T1);
            auto rBxN0 = Cross(rB, N0);
            auto rBxT0 = Cross(rB, T0);
            auto rBxT1 = Cross(rB, T1);

            // The matrix constructed here is positive definite. This ensures
            // the linear system always has a solution, so the bool return
            // value from LinearSystem<double>::Solve is ignored.
            Matrix3x3<double> sysMatrix{};
            sysMatrix(0, 0) = sumInvMasses + Dot(rAxN0, invJA * rAxN0) + Dot(rBxN0, invJB * rBxN0);
            sysMatrix(1, 1) = sumInvMasses + Dot(rAxT0, invJA * rAxT0) + Dot(rBxT0, invJB * rBxT0);
            sysMatrix(2, 2) = sumInvMasses + Dot(rAxT1, invJA * rAxT1) + Dot(rBxT1, invJB * rBxT1);
            sysMatrix(0, 1) = Dot(rAxN0, invJA * rAxT0) + Dot(rBxN0, invJB * rBxT0);
            sysMatrix(0, 2) = Dot(rAxN0, invJA * rAxT1) + Dot(rBxN0, invJB * rBxT1);
            sysMatrix(1, 2) = Dot(rAxT0, invJA * rAxT1) + Dot(rBxT0, invJB * rBxT1);
            sysMatrix(1, 0) = sysMatrix(0, 1);
            sysMatrix(2, 0) = sysMatrix(0, 2);
            sysMatrix(2, 1) = sysMatrix(1, 2);
            Vector3<double> sysInput{};
            sysInput[0] = -(1.0 + restitution) * Dot(N0, velDiffNeg);
            sysInput[1] = 0.0;
            sysInput[2] = 0.0;
            Vector3<double> sysOutput{};
            (void)LinearSystem<double>::Solve(sysMatrix, sysInput, sysOutput);

            // Apply the impulsive force to the bodies to change linear and
            // angular momentum.
            auto impulse = sysOutput[0] * N0 + sysOutput[1] * T0 + sysOutput[2] * T1;
            contact.A->SetLinearMomentum(linmomANeg + impulse);
            contact.B->SetLinearMomentum(linmomBNeg - impulse);
            contact.A->SetAngularMomentum(angmomANeg + Cross(rA, impulse));
            contact.B->SetAngularMomentum(angmomBNeg - Cross(rB, impulse));
        }
        else
        {
            // Fall back to the impulse force f*N0 when the relative
            // velocity at the contact P0 is parallel to N0.
            auto rAxN0 = Cross(rA, N0);
            auto rBxN0 = Cross(rB, N0);
            double quadformA = Dot(rAxN0, invJA * rAxN0);
            double quadformB = Dot(rBxN0, invJB * rBxN0);

            // The magnitude of the impulse force.
            double numer = -(1.0 + restitution) * Dot(N0, velDiffNeg);
            double denom = sumInvMasses + quadformA + quadformB;
            double f = numer / denom;

            // Apply the impulsive force to the bodies to change linear and
            // angular momentum.
            auto impulse = f * N0;
            contact.A->SetLinearMomentum(linmomANeg + impulse);
            contact.B->SetLinearMomentum(linmomBNeg - impulse);
            contact.A->SetAngularMomentum(angmomANeg + Cross(rA, impulse));
            contact.B->SetAngularMomentum(angmomBNeg - Cross(rB, impulse));
        }
    }

    for (size_t i = 0; i < mRigidSphere.size(); ++i)
    {
        mRigidSphere[i]->Update(time, deltaTime);

        // Synchronize the graphics sphere with the physics sphere.
        mSphere[i].center = mRigidSphere[i]->GetPosition();
    }
}
