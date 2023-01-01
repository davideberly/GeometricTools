// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

class PhysicsModule
{
public:
    PhysicsModule();

    // Initialize the differential equation solver.
    void Initialize(double time, double deltaTime, double x, double y,
        double theta, double xDer, double yDer, double thetaDer);

    // Access the current state.
    inline double GetTime() const
    {
        return mTime;
    }

    inline double GetDeltaTime() const
    {
        return mDeltaTime;
    }

    inline double GetX() const
    {
        return mX;
    }

    inline double GetXDer() const
    {
        return mXDer;
    }

    inline double GetY() const
    {
        return mY;
    }

    inline double GetYDer() const
    {
        return mYDer;
    }

    inline double GetTheta() const
    {
        return mTheta;
    }

    inline double GetThetaDer() const
    {
        return mThetaDer;
    }

    // Get rectangle corners in counterclockwise order.
    void GetRectangle(double& x00, double& y00, double& x10, double& y10,
        double& x11, double& y11, double& x01, double& y01) const;

    // Apply a single step of the solver.
    void Update();

    // physical constants   // symbols used in the Game Physics book
    double muGravity;       // mu * g
    double xLocExt;         // alpha0
    double yLocExt;         // beta0

private:
    // state and auxiliary variables
    double mTime, mDeltaTime;
    double mX, mY, mTheta, mXDer, mYDer, mThetaDer;
    double mX0, mY0, mTheta0, mXDer0, mYDer0, mThetaDer0;
    double mLinVelCoeff, mAngVelCoeff;
};
