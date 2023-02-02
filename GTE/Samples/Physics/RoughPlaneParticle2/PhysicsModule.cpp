// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include <Mathematics/Vector2.h>
#include "PhysicsModule.h"

PhysicsModule::PhysicsModule()
    :
    gravity(0.0),
    mass1(0.0),
    mass2(0.0),
    friction1(0.0),
    friction2(0.0),
    mTime(0.0),
    mDeltaTime(0.0),
    mLength1(0.0),
    mLength2(0.0),
    mState{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
    mAux{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
    mSolver{}
{
}

void PhysicsModule::Initialize(double time, double deltaTime, double x1,
    double y1, double x2, double y2, double xDot, double yDot,
    double thetaDot)
{
    mTime = time;
    mDeltaTime = deltaTime;

    // Compute length and moments.
    double deltaX = x2 - x1;
    double deltaY = y2 - y1;
    double length = std::sqrt(deltaX * deltaX + deltaY * deltaY);
    double mu0 = mass1 + mass2;
    double invMu0 = 1.0 / mu0;
    double weight1 = mass1 * invMu0;
    double weight2 = mass2 * invMu0;
    mLength1 = weight2 * length;
    mLength2 = weight1 * length;
    double mu2 = mass1 * mLength1 * mLength1 + mass2 * mLength2 * mLength2;
    double invMu2 = 1.0 / mu2;

    // state variables
    mState[0] = weight1 * x1 + weight2 * x2;
    mState[1] = xDot;
    mState[2] = weight1 * y1 + weight2 * y2;
    mState[3] = yDot;
    mState[4] = std::atan2(deltaY, deltaX);
    mState[5] = thetaDot;

    // auxiliary variable
    mAux[0] = gravity;
    mAux[1] = mLength1;
    mAux[2] = mLength2;
    mAux[3] = -friction1 * invMu0;
    mAux[4] = -friction2 * invMu0;
    mAux[5] = -friction1 * invMu2;
    mAux[6] = -friction2 * invMu2;

    // RK4 differential equation solver.
    std::function<Vector<6, double>(double, Vector<6, double> const&)> odeFunction =
    [this](double, Vector<6, double> const& input)
    {
        double cs = std::cos(input[4]);
        double sn = std::sin(input[4]);
        double angCos = mState[5] * cs;
        double angSin = mState[5] * sn;

        // Compute the friction vectors.  The Normalize function will set a
        // vector to zero if its length is smaller than Mathd::ZERO_TOLERANCE.
        Vector2<double> F1{ input[1] - mAux[1] * angSin, input[3] + mAux[1] * angCos };
        Vector2<double> F2{ input[1] + mAux[2] * angSin, input[3] - mAux[2] * angCos };
        Normalize(F1);
        Normalize(F2);
        double xDotFunction = mAux[3] * F1[0] + mAux[4] * F2[0];
        double yDotFunction = mAux[3] * F1[1] + mAux[4] * F2[1];
        double tmp1 = mAux[1] * mAux[5] * (cs * F1[1] - sn * F1[0]);
        double tmp2 = mAux[2] * mAux[6] * (sn * F2[0] - cs * F2[1]);
        double thetaDotFunction = tmp1 + tmp2;

        Vector<6, double> output
        {
            // x function
            input[1],

            // dot(x) function
            xDotFunction,

            // y function
            input[3],

            // dot(y) function
            yDotFunction,

            // theta function
            input[5],

            // dot(theta) function
            thetaDotFunction
        };

        return output;
    };

    mSolver = std::make_unique<Solver>(mDeltaTime, odeFunction);
}

void PhysicsModule::Get(double& x1, double& y1, double& x2, double& y2) const
{
    double cs = std::cos(mState[4]);
    double sn = std::sin(mState[4]);
    x1 = mState[0] + mLength1 * cs;
    y1 = mState[2] + mLength1 * sn;
    x2 = mState[0] - mLength2 * cs;
    y2 = mState[2] - mLength2 * sn;
}

void PhysicsModule::Update()
{
    // Apply a single step of the ODE solver.
    mSolver->Update(mTime, mState, mTime, mState);
}
