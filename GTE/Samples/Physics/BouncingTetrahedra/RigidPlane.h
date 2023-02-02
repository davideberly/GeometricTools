// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.06

#include <Mathematics/RigidBody.h>
#include <Mathematics/Hyperplane.h>
using namespace gte;

class RigidPlane : public RigidBody<double>
{
public:
    RigidPlane(Plane3<double> const& plane);
    virtual ~RigidPlane() = default;

    inline Plane3<double> const& GetPlane() const
    {
        return mPlane;
    }

    inline double GetSignedDistance(Vector3<double> const& point) const
    {
        return Dot(mPlane.normal, point) - mPlane.constant;
    }

private:
    Plane3<double> mPlane;
};
