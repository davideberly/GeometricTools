// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "PhysicsModule.h"
#include <Mathematics/Rotation.h>

PhysicsModule::PhysicsModule()
    :
    gravity(0.0f),
    a1(0.0f),
    a2(0.0f),
    a3(0.0f),
    radius(0.0f),
    mTime(0.0f),
    mState{ 0.0f, 0.0f, 0.0f, 0.0f },
    mAux{ 0.0f, 0.0f, 0.0f }
{
}

void PhysicsModule::Initialize(float time, float deltaTime, float y1, float y2, float y1Dot, float y2Dot)
{
    mTime = time;

    // state variables
    mState[0] = y1;     // y1(0)
    mState[1] = y1Dot;  // y1'(0)
    mState[2] = y2;     // y2(0)
    mState[3] = y2Dot;  // y2'(0)

    // auxiliary variables
    mAux[0] = a1 * a1;  // a1^2
    mAux[1] = a2 * a2;  // a2^2
    mAux[2] = gravity;  // g

    // RK4 differential equation solver.
    std::function<Vector4<float>(float, Vector4<float> const&)> odeFunction
        =
        [this](float, Vector4<float> const& input) -> Vector4<float>
        {
            float mat00 = mAux[0] + 4.0f * input[0] * input[0];
            float mat01 = 4.0f * input[0] * input[2];
            float mat11 = mAux[1] + 4.0f * input[2] * input[2];
            float invDet = 1.0f / (mat00*mat11 - mat01*mat01);
            float sqrLen = input[1] * input[1] + input[3] * input[3];
            float rhs0 = 2.0f * input[0] * (mAux[2] - 2.0f * input[0] * sqrLen);
            float rhs1 = 2.0f * input[2] * (mAux[2] - 2.0f * input[2] * sqrLen);
            float y1Dot = (mat11 * rhs0 - mat01 * rhs1) * invDet;
            float y2Dot = (mat00 * rhs1 - mat01 * rhs0) * invDet;

            // (y1, dot(y1), y2, dot(y2))
            return Vector4<float>{ input[1], y1Dot, input[3], y2Dot };
        };

    mSolver = std::make_unique<Solver>(deltaTime, odeFunction);
}

void PhysicsModule::GetData(Vector4<float>& center, Matrix4x4<float>& incrRot) const
{
    // Position is a point exactly on the hill.
    Vector4<float> position;
    position[0] = a1 * mState[0];
    position[1] = a2 * mState[2];
    position[2] = a3 - mState[0] * mState[0] - mState[2] * mState[2];
    position[3] = 1.0f;

    // Lift this point off the hill in the normal direction by the radius of
    // the ball so that the ball just touches the hill.  The hill is
    // implicitly specified by F(x,y,z) = z - [a3 - (x/a1)^2 - (y/a2)^2]
    // where (x,y,z) is the position on the hill.  The gradient of F is a
    // normal vector, Grad(F) = (2*x/a1^2,2*y/a2^2,1).
    Vector4<float> normal;
    normal[0] = 2.0f * position[0] / mAux[0];
    normal[1] = 2.0f * position[1] / mAux[1];
    normal[2] = 1.0f;
    normal[3] = 0.0f;
    Normalize(normal);

    center = position + radius * normal;

    // Let the ball rotate as it rolls down hill.  The axis of rotation is
    // the perpendicular to hill normal and ball velocity.  The angle of
    // rotation from the last position is A = speed*deltaTime/radius.
    Vector4<float> velocity;
    velocity[0] = a1 * mState[1];
    velocity[1] = a1 * mState[3];
    velocity[2] = -2.0f * (velocity[0] * mState[0] + velocity[1] * mState[2]);
    velocity[3] = 0.0f;

    float speed = Normalize(velocity);
    float angle = speed * mSolver->GetTDelta() / radius;
    Vector4<float> axis = UnitCross(normal, velocity);
    incrRot = Rotation<4,float>(AxisAngle<4, float>(axis, angle));
}

float PhysicsModule::GetHeight(float x, float y) const
{
    float xScaled = x / a1;
    float yScaled = y / a2;
    return a3 - xScaled * xScaled - yScaled * yScaled;
}

void PhysicsModule::Update()
{
    // Apply a single step of the ODE solver.
    if (mSolver)
    {
        mSolver->Update(mTime, mState, mTime, mState);
    }
}
