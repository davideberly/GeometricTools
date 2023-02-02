// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include <Mathematics/OdeRungeKutta4.h>
#include <Mathematics/Vector.h>
#include <memory>
using namespace gte;

class PhysicsModule
{
public:
    PhysicsModule();

    // Initialize the differential equation solver.
    void Initialize(double time, double deltaTime, double x, double w,
        double theta, double xDer, double wDer, double thetaDer);

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
        return mState[0];
    }

    inline double GetXDer() const
    {
        return mState[1];
    }

    inline double GetW() const
    {
        return mState[2];
    }

    inline double GetWDer() const
    {
        return mState[3];
    }

    inline double GetTheta() const
    {
        return mState[4];
    }

    inline double GetThetaDer() const
    {
        return mState[5];
    }

    // get rectangle corners in counterclockwise order
    void GetRectangle(double& x00, double& y00, double& x10, double& y10,
        double& x11, double& y11, double& x01, double& y01) const;

    // Apply a single step of the solver.
    void Update();

    // physical constants // symbols used in the Game Physics book
    double mu;            // mu
    double gravity;       // g
    double angle;         // phi
    double sinAngle;      // sin(phi)
    double cosAngle;      // cos(phi)
    double xLocExt;       // a
    double yLocExt;       // b
    double zLocExt;       // h

protected:
    // state and auxiliary variables
    double mTime, mDeltaTime;
    Vector<6, double> mState;
    std::array<double, 2> mAux;

    // Angular speed can be solved in closed form.
    double mTheta0, mThetaDer0, mAngVelCoeff;

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<double, Vector<6, double>> Solver;
    std::unique_ptr<Solver> mSolver;
};
