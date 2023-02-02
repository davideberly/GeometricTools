// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06

#include "PhysicsModule.h"
#include <Mathematics/DistPointHyperplane.h>
#include <Mathematics/DistTetrahedron3Tetrahedron3.h>
#include <Mathematics/IntrTetrahedron3Tetrahedron3.h>
#include <Mathematics/PolyhedralMassProperties.h>
#include <Mathematics/RootsBisection1.h>

PhysicsModule::PhysicsModule(size_t numTetra, double xMin,
    double xMax, double yMin, double yMax, double zMin, double zMax)
    :
    mRigidTetra(numTetra),
    mRigidPlane{},
    mContacts{},
    mRestitution(0.25),  // selected arbitrarily
    mNormalFunction{}
{
    // Create the immovable planes.
    mRigidPlane[0] = std::make_shared<RigidPlane>(Plane3<double>({ +1.0,  0.0,  0.0 }, +xMin));
    mRigidPlane[1] = std::make_shared<RigidPlane>(Plane3<double>({  0.0, +1.0,  0.0 }, +yMin));
    mRigidPlane[2] = std::make_shared<RigidPlane>(Plane3<double>({  0.0,  0.0, +1.0 }, +zMin));
    mRigidPlane[3] = std::make_shared<RigidPlane>(Plane3<double>({ -1.0,  0.0,  0.0 }, -xMax));
    mRigidPlane[4] = std::make_shared<RigidPlane>(Plane3<double>({  0.0, -1.0,  0.0 }, -yMax));
    mRigidPlane[5] = std::make_shared<RigidPlane>(Plane3<double>({  0.0,  0.0, -1.0 }, -zMax));

    mNormalFunction[0] = &PhysicsModule::ComputeNothing;
    mNormalFunction[1] = &PhysicsModule::ComputeNormal0;
    mNormalFunction[2] = &PhysicsModule::ComputeNormal1;
    mNormalFunction[3] = &PhysicsModule::ComputeNormal01;
    mNormalFunction[4] = &PhysicsModule::ComputeNormal2;
    mNormalFunction[5] = &PhysicsModule::ComputeNormal02;
    mNormalFunction[6] = &PhysicsModule::ComputeNormal12;
    mNormalFunction[7] = &PhysicsModule::ComputeNormal021;
    mNormalFunction[8] = &PhysicsModule::ComputeNormal3;
    mNormalFunction[9] = &PhysicsModule::ComputeNormal03;
    mNormalFunction[10] = &PhysicsModule::ComputeNormal13;
    mNormalFunction[11] = &PhysicsModule::ComputeNormal013;
    mNormalFunction[12] = &PhysicsModule::ComputeNormal23;
    mNormalFunction[13] = &PhysicsModule::ComputeNormal032;
    mNormalFunction[14] = &PhysicsModule::ComputeNormal123;
    mNormalFunction[15] = &PhysicsModule::ComputeNothing;
}

void PhysicsModule::InitializeTetrahedron(size_t i, double massDensity,
    Tetrahedron3<double> const& tetra,
    Vector3<double> const& position,
    Vector3<double> const& linearVelocity,
    Quaternion<double> const& qOrientation,
    Vector3<double> const& angularVelocity)
{
    mRigidTetra[i] = std::make_shared<RigidTetrahedron>(tetra, position, massDensity);

    // This sets the initial linear velocity. It also sets the initial linear
    // momentum.
    mRigidTetra[i]->SetLinearVelocity(linearVelocity);

    // This sets the initial orientation. It also synchronizes the world
    // inertia tensor with the current orientation.
    mRigidTetra[i]->SetQOrientation(qOrientation, true);

    // SetAngularVelocity requires a current world inertia tensor, so it must
    // be called after SetQOrientation.
    mRigidTetra[i]->SetAngularVelocity(angularVelocity);

    mRigidTetra[i]->Force = [this, i](double, RigidBodyState<double> const& state)
    {
        // The only external force is gravity.
        double constexpr gravityConstant = 9.81;   // m/sec^2
        Vector3<double> gravityDirection{ 0.0, 0.0, -1.0 };
        Vector3<double> gravityForce =
            (state.GetMass() * gravityConstant) * gravityDirection;

        // Take into account friction when the tetrahedra are sliding on the
        // floor.
        double constexpr epsilon = 1e-03;
        auto const& tetra = mRigidTetra[i]->GetWorldTetrahedron();
        size_t numVerticesOnFloor = 0;
        for (size_t j = 0; j < 4; ++j)
        {
            if (std::fabs(tetra.v[j][2]) <= epsilon)
            {
                ++numVerticesOnFloor;
            }
        }

        Vector3<double> frictionForce{ 0.0, 0.0, 0.0 };
        if (numVerticesOnFloor == 3)
        {
            double viscosity = 100.0 * static_cast<double>(numVerticesOnFloor);
            Vector3<double> linearVelocity = state.GetLinearVelocity();
            Normalize(linearVelocity);
            frictionForce = -viscosity * linearVelocity;
            frictionForce[2] = 0.0;
        }

        return gravityForce + frictionForce;
    };

    mRigidTetra[i]->Torque = [this, i](double, RigidBodyState<double> const& state)
    {
        // No external torque is applied. However, take into account friction
        // when the tetrahedra are spinning on the floor.
        double constexpr epsilon = 1e-03;
        auto const& tetra = mRigidTetra[i]->GetWorldTetrahedron();
        size_t numVerticesOnFloor = 0;
        for (size_t j = 0; j < 4; ++j)
        {
            if (std::fabs(tetra.v[j][2]) <= epsilon)
            {
                ++numVerticesOnFloor;
            }
        }

        Vector3<double> torque{ 0.0, 0.0, 0.0 };
        if (numVerticesOnFloor == 3)
        {
            double constexpr viscosity = 0.1;
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

    // Test for tetrahedron-plane collisions. These checks are done in pairs
    // with the assumption that the bounding spheres of the tetrahedra have
    // diameters smaller than the distance between parallel planar boundaries.
    // In this case, only one of each parallel pair of planes can be
    // intersected at any time. Each pair of parallel planes is tested in
    // order to handle the case when a tetrahedron intersects two planes
    // meeting at a region edge or three planes meeting at a region corner.
    // When the tetrahedron is partially or fully outside a plane, the
    // interpenetration is removed to push the tetrahedron back into the
    // simulation region.
    size_t const numTetra = mRigidTetra.size();
    std::vector<bool> moved(numTetra, false);
    double overlap{};
    for (size_t i = 0; i < numTetra; ++i)
    {
        mRigidTetra[i]->UpdateWorldQuantities();
        auto const& sphere = mRigidTetra[i]->GetWorldSphere();

        // Test for the tetrahedron intersecting or occurring outside an
        // x-constant plane.
        overlap = sphere.radius - mRigidPlane[0]->GetSignedDistance(sphere.center);
        if (overlap > 0.0)
        {
            moved[i] = SetTetrahedronPlaneContact(mRigidTetra[i], mRigidPlane[0]);
        }
        else
        {
            overlap = sphere.radius - mRigidPlane[3]->GetSignedDistance(sphere.center);
            if (overlap > 0.0)
            {
                moved[i] = SetTetrahedronPlaneContact(mRigidTetra[i], mRigidPlane[3]);
            }
        }

        // Test for the tetrahedron intersecting or occurring outside a
        // y-constant plane.
        overlap = sphere.radius - mRigidPlane[1]->GetSignedDistance(sphere.center);
        if (overlap > 0.0)
        {
            moved[i] = SetTetrahedronPlaneContact(mRigidTetra[i], mRigidPlane[1]);
        }
        else
        {
            overlap = sphere.radius - mRigidPlane[4]->GetSignedDistance(sphere.center);
            if (overlap > 0.0)
            {
                moved[i] = SetTetrahedronPlaneContact(mRigidTetra[i], mRigidPlane[4]);
            }
        }

        // Test for the tetrahedron intersecting or occurring outside a
        // z-constant plane.
        overlap = sphere.radius - mRigidPlane[2]->GetSignedDistance(sphere.center);
        if (overlap > 0.0)
        {
            moved[i] = SetTetrahedronPlaneContact(mRigidTetra[i], mRigidPlane[2]);
        }
        else
        {
            overlap = sphere.radius - mRigidPlane[5]->GetSignedDistance(sphere.center);
            if (overlap > 0.0)
            {
                moved[i] = SetTetrahedronPlaneContact(mRigidTetra[i], mRigidPlane[5]);
            }
        }
    }

    // Test for tetrahedron-tetrahedron collisions.
    for (size_t i0 = 0; i0 + 1 < numTetra; ++i0)
    {
        auto const& sphere0 = mRigidTetra[i0]->GetWorldSphere();

        for (size_t i1 = i0 + 1; i1 < numTetra; ++i1)
        {
            auto const& sphere1 = mRigidTetra[i1]->GetWorldSphere();

            // Test for overlap of sphere i0 and sphere i1.
            auto delta = sphere1.center - sphere0.center;
            double lengthDelta = Length(delta);
            overlap = sphere0.radius + sphere1.radius - lengthDelta;
            if (overlap > 0.0)
            {
                UndoTetrahedraOverlap(mRigidTetra[i0], mRigidTetra[i1],
                    moved[i0], moved[i1]);
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

    for (size_t i = 0; i < mRigidTetra.size(); ++i)
    {
        mRigidTetra[i]->Update(time, deltaTime);

        // This is another way to lose kinetic energy. Dampen the
        // linear and angular velocity over time.
        Vector3<double> velocity = mRigidTetra[i]->GetLinearVelocity();
        velocity *= 0.9999;
        mRigidTetra[i]->SetLinearVelocity(velocity);

        velocity = mRigidTetra[i]->GetAngularVelocity();
        velocity *= 0.9999;
        mRigidTetra[i]->SetAngularVelocity(velocity);
    }
}

bool PhysicsModule::SetTetrahedronPlaneContact(
    std::shared_ptr<RigidTetrahedron> const& rigidTetra,
    std::shared_ptr<RigidPlane> const& rigidPlane)
{
    // For this function to be called, the bounding sphere of the tetrahedron
    // is partially outside the plane. Determine whether the tetrahedron
    // itself is partially outside the plane. If the signed distances from
    // the tetrahedron vertices to the plane are all positive, there is
    // no contact with the plane. If the signed distances are nonnegative
    // with at least pne that is zero, the tetrahedron is just in contact
    // with the plane and no interpenetration. If at least one signed
    // distance is negative, find the most negative value in order to know
    // how far the tetrahedron must be moved in the plane-normal direction
    // to place it back in the simulation region.
    auto const& tetra = rigidTetra->GetWorldTetrahedron();
    size_t minIndex = std::numeric_limits<size_t>::max();
    double minDistance = 0.0;
    for (size_t i = 0; i < 4; ++i)
    {
        double distance = rigidPlane->GetSignedDistance(tetra.v[i]);
        if (distance < minDistance)
        {
            minDistance = distance;
            minIndex = i;
        }
    }

    if (minIndex != std::numeric_limits<size_t>::max())
    {
        // The tetrahedron is partially or fully outside the plane. Move
        // the intersecting tetrahedron to be just touching the plane.
        auto const& normal = rigidPlane->GetPlane().normal;
        Vector3<double> overlap = minDistance * normal;

        Contact contact{};
        contact.A = rigidTetra;
        contact.B = rigidPlane;
        contact.P = tetra.v[minIndex] - overlap;
        contact.N = normal;
        mContacts.push_back(contact);

        rigidTetra->SetPosition(rigidTetra->GetPosition() - overlap);
        rigidTetra->UpdateWorldQuantities();
        return true;
    }
    else
    {
        return false;
    }
}

void PhysicsModule::UndoTetrahedraOverlap(
    std::shared_ptr<RigidTetrahedron> const& rigidTetra0,
    std::shared_ptr<RigidTetrahedron> const& rigidTetra1,
    bool moved0, bool moved1)
{
    Tetrahedron3<double> const& tetra0 = rigidTetra0->GetWorldTetrahedron();
    Tetrahedron3<double> const& tetra1 = rigidTetra1->GetWorldTetrahedron();

    TIQuery<double, Tetrahedron3<double>, Tetrahedron3<double>> query{};
    auto result = query(tetra0, tetra1);
    if (!result.intersect)
    {
        // The tetrahedra are separated, so there is no overlap to undo.
        return;
    }

    // Undo the interpenetration.
    double radius0 = rigidTetra0->GetRadius();
    double radius1 = rigidTetra1->GetRadius();
    auto offset = ComputeTetrahedronOffset(tetra0, radius0, tetra1, radius1);
    if (moved0 && !moved1)
    {
        // Tetrahedron i0 moved but tetrahedron i1 did not.
        rigidTetra1->SetPosition(rigidTetra1->GetPosition() + offset);
        rigidTetra1->UpdateWorldQuantities();
    }
    else if (!moved0 && moved1)
    {
        // Tetrahedron i1 moved but tetrahedron i0 did not.
        rigidTetra0->SetPosition(rigidTetra0->GetPosition() - offset);
        rigidTetra0->UpdateWorldQuantities();
    }
    else
    {
        // Neither tetrahedra moved or both tetrahedra moved. Avoid bias by
        // moving both tetrahedra half the offset.
        offset *= 0.5;
        rigidTetra1->SetPosition(rigidTetra1->GetPosition() + offset);
        rigidTetra1->UpdateWorldQuantities();
        rigidTetra0->SetPosition(rigidTetra0->GetPosition() - offset);
        rigidTetra0->UpdateWorldQuantities();
    }

    // Compute the contact information. 
    DCPQuery<double, Tetrahedron3<double>, Tetrahedron3<double>> dcpQuery{};
    auto dcpResult = dcpQuery(tetra0, tetra1);
    Contact contact{};
    contact.P = 0.5 * (dcpResult.closest[0] + dcpResult.closest[1]);
    contact.restitution = mRestitution;
    ClassifyContact(
        rigidTetra0, tetra0, dcpResult.barycentric0,
        rigidTetra1, tetra1, dcpResult.barycentric1,
        contact);

    mContacts.push_back(contact);
}

Vector3<double> PhysicsModule::ComputeTetrahedronOffset(
    Tetrahedron3<double> const& tetra0, double radius0,
    Tetrahedron3<double> const& tetra1, double radius1) const
{
    auto const& centroid0 = tetra0.ComputeCentroid();
    auto const& centroid1 = tetra1.ComputeCentroid();
    auto direction = centroid1 - centroid0;
    double length = Normalize(direction);
    double sMin = 0.0, sMax = (radius0 + radius1) / length - 1.0;
    double fMin = -1.0, fMax = +1.0;

    auto F = [&tetra0, &tetra1, &direction](double const& s)
    {
        Vector3<double> translate = s * direction;
        Tetrahedron3<double> newTetra1{};
        for (int32_t i = 0; i < 4; ++i)
        {
            newTetra1.v[i] = tetra1.v[i] + translate;
        }

        TIQuery<double, Tetrahedron3<double>, Tetrahedron3<double>> query{};
        auto result = query(tetra0, newTetra1, 1e-12);
        return (result.intersect ? -1.0 : +1.0);
    };

    // TODO: Verify that the maximum number of iterations is the precision
    // of 'double', which is 53. For now, use 64.
    RootsBisection1<double> bisector(std::numeric_limits<double>::digits);
    double sRoot = 0.0, fAtSRoot = 0.0;
    (void)bisector(F, sMin, sMax, fMin, fMax, sRoot, fAtSRoot);
    Vector3<double> offset = sRoot * direction;
    return offset;
}

void PhysicsModule::ClassifyContact(
    std::shared_ptr<RigidTetrahedron> const& rigidTetra0,
    Tetrahedron3<double> const& tetra0,
    std::array<double, 4> const& bary0,
    std::shared_ptr<RigidTetrahedron> const& rigidTetra1,
    Tetrahedron3<double> const& tetra1,
    std::array<double, 4> const& bary1,
    Contact& contact)
{
    // The number of (approximately) zero-valued barycentric coordinates
    // determine the type of closest point. It is 0 for an interior point,
    // 1 for a face point, 2 for an edge point or 3 for a vertex. The
    // indicator*[] components are used to support computation of the
    // normal vector at the contact point.
    double constexpr baryEpsilon = 1e-12;
    size_t numZeroBary0 = 0, numZeroBary1 = 0;
    size_t indicator0 = 0, indicator1 = 0;
    for (size_t i = 0, mask = 1; i < 4; ++i, mask <<= 1)
    {
        if (std::fabs(bary0[i]) <= baryEpsilon)
        {
            ++numZeroBary0;
        }
        else
        {
            indicator0 |= mask;
        }

        if (std::fabs(bary1[i]) <= baryEpsilon)
        {
            ++numZeroBary1;
        }
        else
        {
            indicator1 |= mask;
        }
    }

    if (numZeroBary0 == 1)
    {
        // The tetra0 contact point is on a face.
        contact.A = rigidTetra1;
        contact.B = rigidTetra0;
        contact.N = (this->*mNormalFunction[indicator0])(tetra0);
        return;
    }

    if (numZeroBary1 == 1)
    {
        // The tetra1 contact point is on a face.
        contact.A = rigidTetra0;
        contact.B = rigidTetra1;
        contact.N = (this->*mNormalFunction[indicator1])(tetra1);
        return;
    }

    if (numZeroBary0 == 2)
    {
        // The tetra0 contact point is on an edge.
        contact.A = rigidTetra1;
        contact.B = rigidTetra0;
        contact.N = (this->*mNormalFunction[indicator0])(tetra0);
        return;
    }

    if (numZeroBary1 == 2)
    {
        // The tetra1 contact point is on an edge.
        contact.A = rigidTetra0;
        contact.B = rigidTetra1;
        contact.N = (this->*mNormalFunction[indicator1])(tetra1);
        return;
    }

    if (numZeroBary0 == 3)
    {
        // The tetra0 contact point is at a vertex.
        contact.A = rigidTetra1;
        contact.B = rigidTetra0;
        contact.N = (this->*mNormalFunction[indicator0])(tetra0);
        return;
    }

    if (numZeroBary1 == 3)
    {
        // The tetra1 contact point is at a vertex.
        contact.A = rigidTetra0;
        contact.B = rigidTetra1;
        contact.N = (this->*mNormalFunction[indicator1])(tetra1);
        return;
    }

    // The remaining cases are numZeroBary0 = 0 and numZeroBary1 = 0,
    // which should not happen given the epsilon thresholding of the
    // barycentric coordinates. With floating-point arithmetic and small
    // tetrahedra, it might be possible to reach this code. Rather than
    // throw an exception, use the difference of centroids as the normal
    // direction.
    contact.A = rigidTetra0;
    contact.B = rigidTetra1;
    contact.N = tetra0.ComputeCentroid() - tetra1.ComputeCentroid();
    Normalize(contact.N);

    // If you want to trap this case, expose the LogError exception.
    // LogError("Expecting non-interior contact points.");
}

Vector3<double> PhysicsModule::ComputeNormal021(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeFaceNormal(0);
}

Vector3<double> PhysicsModule::ComputeNormal013(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeFaceNormal(1);
}

Vector3<double> PhysicsModule::ComputeNormal032(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeFaceNormal(2);
}

Vector3<double> PhysicsModule::ComputeNormal123(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeFaceNormal(3);
}

Vector3<double> PhysicsModule::ComputeNormal01(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeEdgeNormal(0);
}

Vector3<double> PhysicsModule::ComputeNormal02(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeEdgeNormal(1);
}

Vector3<double> PhysicsModule::ComputeNormal03(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeEdgeNormal(2);
}

Vector3<double> PhysicsModule::ComputeNormal12(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeEdgeNormal(3);
}

Vector3<double> PhysicsModule::ComputeNormal13(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeEdgeNormal(4);
}

Vector3<double> PhysicsModule::ComputeNormal23(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeEdgeNormal(5);
}

Vector3<double> PhysicsModule::ComputeNormal0(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeVertexNormal(0);
}

Vector3<double> PhysicsModule::ComputeNormal1(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeVertexNormal(1);
}

Vector3<double> PhysicsModule::ComputeNormal2(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeVertexNormal(2);
}

Vector3<double> PhysicsModule::ComputeNormal3(Tetrahedron3<double> const& tetra)
{
    return tetra.ComputeVertexNormal(3);
}

Vector3<double> PhysicsModule::ComputeNothing(Tetrahedron3<double> const&)
{
    return Vector3<double>{ 0.0, 0.0, 0.0 };
}

