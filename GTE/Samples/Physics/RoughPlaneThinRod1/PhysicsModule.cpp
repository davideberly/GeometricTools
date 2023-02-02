// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include <Mathematics/Integration.h>
#include "PhysicsModule.h"

PhysicsModule::PhysicsModule()
    :
    length(0.0),
    massDensity(0.0),
    friction(0.0),
    mTime(0.0),
    mDeltaTime(0.0),
    mState{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
    mAux{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
    mHalfLength(0.0),
    mFXIntegrand{},
    mFYIntegrand{},
    mFThetaIntegrand{},
    mSolver{}
{
}

void PhysicsModule::Initialize(double time, double deltaTime, double x,
    double y, double theta, double xDot, double yDot, double thetaDot)
{
    mTime = time;
    mDeltaTime = deltaTime;

    // state variables
    mState[0] = x;
    mState[1] = xDot;
    mState[2] = y;
    mState[3] = yDot;
    mState[4] = theta;
    mState[5] = thetaDot;

    // auxiliary variable
    double mu0 = massDensity * length;
    double mu2 = massDensity * length * length * length / 12.0;
    mHalfLength = 0.5 * length;
    mAux[0] = mHalfLength;
    mAux[1] = -friction / mu0;
    mAux[2] = -friction / mu2;
    mAux[3] = mState[1];  // need dot(x) for integration
    mAux[4] = mState[3];  // need dot(y) for integration
    mAux[5] = mState[4];  // need theta for integration
    mAux[6] = mState[5];  // need dot(theta) for integration

    mFXIntegrand = [this](double ell)
    {
        // Compute all integrands here. Return one value, store the others for
        // later access.
        double cs = std::cos(mAux[2]);
        double sn = std::sin(mAux[2]);
        double tmp1 = mAux[3] - ell * mAux[6] * sn;
        double tmp2 = mAux[4] + ell * mAux[6] * cs;
        double length = std::sqrt(tmp1 * tmp1 + tmp2 * tmp2);
        if (length > 0.0)
        {
            double invLength = 1.0 / length;

            // FY integrand
            mAux[7] = mAux[1] * tmp2 * invLength;

            // FTheta integrand
            double tmp3 = ell * (ell * mAux[6] - mAux[3] * sn + mAux[4] * cs);
            mAux[8] = mAux[2] * tmp3 * invLength;

            // FX integrand
            return mAux[1] * tmp1 * invLength;
        }
        else
        {
            // FY integrand
            mAux[7] = 0.0;

            // FTheta integrand
            mAux[8] = 0.0;

            // FX integrand
            return 0.0;
        }
    };

    mFYIntegrand = [this](double) { return mAux[7]; };

    mFThetaIntegrand = [this](double) { return mAux[8]; };

    // RK4 differential equation solver.
    std::function<Vector<6, double>(double, Vector<6, double> const&)> odeFunction =
    [this](double, Vector<6, double> const& input)
    {
        Vector<6, double> output
        {
            // x function
            input[1],

            // dot(x) function
            Integration<double>::Romberg(
                rombergOrder, -mAux[4], mAux[4], mFXIntegrand),

            // y function
            input[3],

            // dot(y) function
            Integration<double>::Romberg(
                rombergOrder, -mAux[4], mAux[4], mFYIntegrand),

            // theta function
            input[5],

            // dot(theta) function
            Integration<double>::Romberg(
                rombergOrder, -mAux[4], mAux[4], mFThetaIntegrand)
        };

        return output;
    };

    mSolver = std::make_unique<Solver>(mDeltaTime, odeFunction);
}

void PhysicsModule::Get(double& x1, double& y1, double& x2, double& y2) const
{
    double cs = std::cos(mState[4]);
    double sn = std::sin(mState[4]);
    x1 = mState[0] + mHalfLength * cs;
    y1 = mState[2] + mHalfLength * sn;
    x2 = mState[0] - mHalfLength * cs;
    y2 = mState[2] - mHalfLength * sn;
}

void PhysicsModule::Update()
{
    // Apply a single step of the ODE solver.
    mSolver->Update(mTime, mState, mTime, mState);

    mAux[3] = mState[1];
    mAux[4] = mState[3];
    mAux[5] = mState[4];
    mAux[6] = mState[5];
}
