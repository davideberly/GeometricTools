// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include <Mathematics/OdeRungeKutta4.h>
#include <Mathematics/Vector.h>
#include <functional>
#include <memory>
using namespace gte;

class PhysicsModule
{
public:
    PhysicsModule();

    // Initialize the differential equation solver.
    void Initialize(double time, double deltaTime, double x, double y,
        double theta, double xDot, double yDot, double thetaDot);

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

    inline double GetXDot() const
    {
        return mState[1];
    }

    inline double GetY() const
    {
        return mState[2];
    }

    inline double GetYDot() const
    {
        return mState[3];
    }

    inline double GetTheta() const
    {
        return mState[4];
    }

    inline double GetThetaDot() const
    {
        return mState[5];
    }

    void Get(double& x1, double& y1, double& x2, double& y2) const;

    // Apply a single step of the solver.
    void Update();

    // physical constants   // symbols used in the Game Physics book
    double length;          // L1 = L2 = L/2
    double massDensity;     // delta0
    double friction;        // c

private:
    // state and auxiliary variables
    double mTime, mDeltaTime;
    Vector<6, double> mState;
    std::array<double, 9> mAux;
    double mHalfLength;

    // Integrands for the generalized forces.
    std::function<double(double)> mFXIntegrand;
    std::function<double(double)> mFYIntegrand;
    std::function<double(double)> mFThetaIntegrand;

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<double, Vector<6, double>> Solver;
    std::unique_ptr<Solver> mSolver;

    // Romberg integration parameter.
    static int32_t constexpr rombergOrder = 16;
};
