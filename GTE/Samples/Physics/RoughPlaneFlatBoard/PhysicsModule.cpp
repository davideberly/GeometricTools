// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2023.08.08

#include "PhysicsModule.h"
#include <cmath>

PhysicsModule::PhysicsModule()
    :
    muGravity(0.0),
    xLocExt(0.0),
    yLocExt(0.0),
    mTime(0.0),
    mDeltaTime(0.0),
    mX(0.0),
    mY(0.0),
    mTheta(0.0),
    mXDer(0.0),
    mYDer(0.0),
    mThetaDer(0.0),
    mX0(0.0),
    mY0(0.0),
    mTheta0(0.0),
    mXDer0(0.0),
    mYDer0(0.0),
    mThetaDer0(0.0),
    mLinVelCoeff(0.0),
    mAngVelCoeff(0.0)
{
}

void PhysicsModule::Initialize(double time, double deltaTime, double x,
    double y, double theta, double xDer, double yDer, double thetaDer)
{
    mTime = time;
    mDeltaTime = deltaTime;
    mX = x;
    mY = y;
    mTheta = theta;
    mXDer = xDer;
    mYDer = yDer;
    mThetaDer = thetaDer;

    mX0 = mX;
    mY0 = mY;
    mTheta0 = mTheta;
    mXDer0 = mXDer;
    mYDer0 = mYDer;
    mThetaDer0 = mThetaDer;

    double linSpeed = std::sqrt(xDer * xDer + yDer * yDer);
    if (linSpeed > 0.0)
    {
        mLinVelCoeff = muGravity / linSpeed;
    }
    else
    {
        mLinVelCoeff = 0.0;
    }

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
    double coeff = muGravity * numer / denom;

    double angSpeed = std::fabs(thetaDer);
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

    double cs = std::cos(mTheta);
    double sn = std::sin(mTheta);

    // sx = -1, sy = -1
    x00 = mX - xLocExt * cs + yLocExt * sn;
    y00 = mY - xLocExt * sn - yLocExt * cs;

    // sx = +1, sy = -1
    x10 = mX + xLocExt * cs + yLocExt * sn;
    y10 = mY + xLocExt * sn - yLocExt * cs;

    // sx = +1, sy = +1
    x11 = mX + xLocExt * cs - yLocExt * sn;
    y11 = mY + xLocExt * sn + yLocExt * cs;

    // sx = -1, sy = +1
    x01 = mX - xLocExt * cs - yLocExt * sn;
    y01 = mY - xLocExt * sn + yLocExt * cs;
}

void PhysicsModule::Update()
{
    mTime += mDeltaTime;

    double linTmp = mLinVelCoeff * mTime;
    double linVelMult = 1.0 - linTmp;
    if (linVelMult > 0.0)
    {
        mXDer = linVelMult * mXDer0;
        mYDer = linVelMult * mYDer0;
        mX = mX0 + mTime * (1.0 - 0.5 * linTmp) * mXDer0;
        mY = mY0 + mTime * (1.0 - 0.5 * linTmp) * mYDer0;
    }
    else
    {
        mXDer = 0.0;
        mYDer = 0.0;
    }

    double angTmp = mAngVelCoeff * mTime;
    double angVelMult = 1.0 - angTmp;
    if (angVelMult > 0.0)
    {
        mThetaDer = angVelMult * mThetaDer0;
        mTheta = mTheta0 + mTime * (1.0 - 0.5 * angTmp) * mThetaDer0;
    }
    else
    {
        mThetaDer = 0.0;
    }
}
