// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include "PhysicsModule.h"

PhysicsModule::PhysicsModule()
    :
    mu(0.0),
    gravity(0.0),
    angle(0.0),
    sinAngle(0.0),
    cosAngle(0.0),
    xLocExt(0.0),
    yLocExt(0.0),
    zLocExt(0.0),
    mTime(0.0),
    mDeltaTime(0.0),
    mState{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
    mAux{ 0.0, 0.0 },
    mTheta0(0.0),
    mThetaDer0(0.0),
    mAngVelCoeff(0.0),
    mSolver{}
{
}

void PhysicsModule::Initialize(double time, double deltaTime, double x,
    double w, double theta, double xDot, double wDot, double thetaDot)
{
    mTime = time;
    mDeltaTime = deltaTime;

    // state variables
    mState[0] = x;
    mState[1] = xDot;
    mState[2] = w;
    mState[3] = wDot;
    mState[4] = theta;
    mState[5] = thetaDot;

    // auxiliary variables
    sinAngle = std::sin(angle);
    cosAngle = std::cos(angle);
    mAux[0] = mu * gravity;  // c/m in the one-particle system example
    mAux[1] = gravity * sinAngle;

    // RK4 differential equation solver.
    std::function<Vector<6, double>(double, Vector<6, double> const&)> odeFunction =
    [this](double, Vector<6, double> const& input)
    {
        double vLen = std::sqrt(input[1] * input[1] + input[3] * input[3]);
        double xDotFunction, wDotFunction;
        if (vLen > 0.0)
        {
            double temp = -mAux[0] / vLen;
            xDotFunction = temp * input[1];
            wDotFunction = temp * input[3] - mAux[1];
        }
        else
        {
            // Velocity is effectively zero, so frictional force is zero.
            xDotFunction = 0.0;
            wDotFunction = -mAux[1];
        }

        Vector<6, double> output
        {
            // x function
            input[1],

            // dot(x) function
            xDotFunction,

            // w function
            input[3],

            // dot(w) function
            wDotFunction,

            // theta and dot(theta) are set in the Update() call
            0.0,
            0.0
        };

        return output;
    };

    mSolver = std::make_unique<Solver>(mDeltaTime, odeFunction);

    // Set up for angular speed.
    mTheta0 = theta;
    mThetaDer0 = thetaDot;

    double xx = xLocExt * xLocExt;
    double xy = xLocExt * yLocExt;
    double yy = yLocExt * yLocExt;
    double tmp1 = xx + yy;
    double tmp2 = std::sqrt(tmp1);
    double tmp3 = 4.0 * xy / 3.0;
    double tmp4 = 0.5 * std::log((tmp2 + xLocExt) / (tmp2 - xLocExt));
    double tmp5 = 0.5 * std::log((tmp2 + yLocExt) / (tmp2 - yLocExt));
    double numer = tmp3 * tmp2 + xLocExt * xx * tmp5 + yLocExt * yy * tmp4;
    double denom = tmp3 * tmp1;
    double coeff = mu * gravity * numer / denom;

    double angSpeed = std::fabs(thetaDot);
    if (angSpeed > 0.0)
    {
        mAngVelCoeff = coeff / angSpeed;
    }
    else
    {
        mAngVelCoeff = 0.0;
    }
}

void PhysicsModule::GetRectangle(double& x00, double& y00, double& x10,
    double& y10, double& x11, double& y11, double& x01, double& y01) const
{
    // P = (x,y) + sx*xLocExt*(cos(A),sin(A)) + sy*yLocExt*(-sin(A),cos(A))
    // where |sx| = 1 and |sy| = 1 (four choices on sign)

    double cs = std::cos(mState[4]);
    double sn = std::sin(mState[4]);

    // sx = -1, sy = -1
    x00 = mState[0] - xLocExt * cs + yLocExt * sn;
    y00 = mState[2] - xLocExt * sn - yLocExt * cs;

    // sx = +1, sy = -1
    x10 = mState[0] + xLocExt * cs + yLocExt * sn;
    y10 = mState[2] + xLocExt * sn - yLocExt * cs;

    // sx = +1, sy = +1
    x11 = mState[0] + xLocExt * cs - yLocExt * sn;
    y11 = mState[2] + xLocExt * sn + yLocExt * cs;

    // sx = -1, sy = +1
    x01 = mState[0] - xLocExt * cs - yLocExt * sn;
    y01 = mState[2] - xLocExt * sn + yLocExt * cs;
}

void PhysicsModule::Update()
{
    // Apply a single step of the ODE solver.
    mSolver->Update(mTime, mState, mTime, mState);

    // Update for angular speed.
    double angTmp = mAngVelCoeff * mTime;
    double angVelMult = 1.0 - angTmp;
    mState[4] = mTheta0 + mTime * (1.0 - 0.5 * angTmp) * mThetaDer0;
    mState[5] = angVelMult * mThetaDer0;
}
