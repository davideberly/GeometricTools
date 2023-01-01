// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06

#include "RigidPlane.h"
#include "RigidTetrahedron.h"
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
    PhysicsModule(size_t numTetra, double xMin, double xMax, double yMin,
        double yMax, double zMin, double zMax);

    // This function must be called for each of the numTetrahedra sphere
    // objects before starting the simulation. The initial tetrahedron has
    // body-coordinate vertices {V0,V1,V2,V3}. The input includes the mass
    // density and the initial linear and angular momenta. The mass, centroid
    // and the body inertia tensor are computed by this function. An initial
    // simulation step is computed before the real-time simulation in order to
    // set all the other physics parameters.
    void InitializeTetrahedron(size_t i, double massDensity,
        Tetrahedron3<double> const& tetra,
        Vector3<double> const& position,
        Vector3<double> const& linearVelocity,
        Quaternion<double> const& qOrientation,
        Vector3<double> const& angularVelocity);

    inline size_t GetNumTetrahedra() const
    {
        return mRigidTetra.size();
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
    inline Plane3<double> const& GetPlane(size_t i) const
    {
        return mRigidPlane[i]->GetPlane();
    }

    // The input must satisfy 0 <= i < numTetrahedra where the upper bound was
    // passed to the constructor.
    inline Tetrahedron3<double> const& GetBodyTetrahedron(size_t i) const
    {
        return mRigidTetra[i]->GetBodyTetrahedron();
    }

    inline Tetrahedron3<double> GetWorldTetrahedron(size_t i) const
    {
        return mRigidTetra[i]->GetWorldTetrahedron();
    }

    // The input must satisfy 0 <= i < numTetrahedra where the upper bound was
    // passed to the constructor.
    inline Sphere3<double> const& GetWorldSphere(size_t i) const
    {
        return mRigidTetra[i]->GetWorldSphere();
    }

    inline Vector3<double> const& GetPosition(size_t i) const
    {
        return mRigidTetra[i]->GetPosition();
    }

    inline Matrix3x3<double> const& GetOrientation(size_t i) const
    {
        return mRigidTetra[i]->GetROrientation();
    }

    // Execute the physics simulation. The caller of this function maintains
    // the physics clock.
    void DoTick(double time, double deltaTime);
    
private:
    using Contact = RigidBodyContact<double>;

    void DoCollisionDetection();
    void DoCollisionResponse(double time, double deltaTime);

    bool SetTetrahedronPlaneContact(
        std::shared_ptr<RigidTetrahedron> const& rigidTetra,
        std::shared_ptr<RigidPlane> const& rigidPlane);

    void UndoTetrahedraOverlap(
        std::shared_ptr<RigidTetrahedron> const& rigidTetra0,
        std::shared_ptr<RigidTetrahedron> const& rigidTetra1,
        bool moved0, bool moved1);

    Vector3<double> ComputeTetrahedronOffset(
        Tetrahedron3<double> const& tetra0, double radius0,
        Tetrahedron3<double> const& tetra1, double radius1) const;

    void ClassifyContact(
        std::shared_ptr<RigidTetrahedron> const& rigidTetra0,
        Tetrahedron3<double> const& tetra0,
        std::array<double, 4> const& bary0,
        std::shared_ptr<RigidTetrahedron> const& rigidTetra1,
        Tetrahedron3<double> const& tetra1,
        std::array<double, 4> const& bary1,
        Contact& contact);

    Vector3<double> ComputeNormal021(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal013(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal032(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal123(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal01(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal02(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal03(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal12(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal13(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal23(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal0(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal1(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal2(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNormal3(Tetrahedron3<double> const& tetra);
    Vector3<double> ComputeNothing(Tetrahedron3<double> const& tetra);

    // Physical representations of solid tetrahedra.
    std::vector<std::shared_ptr<RigidTetrahedron>> mRigidTetra;

    // Physical representation of planar boundaries.
    std::array<std::shared_ptr<RigidPlane>, 6> mRigidPlane;

    // Contact points during one pass of the physical simulation.
    std::vector<Contact> mContacts;
    double mRestitution;

    // An array lookup for computing normal vectors based on the
    // indicator values in ClassifyContact.
    using NormalFunction = Vector3<double>(PhysicsModule::*)(Tetrahedron3<double> const&);
    std::array<NormalFunction, 16> mNormalFunction;
};
