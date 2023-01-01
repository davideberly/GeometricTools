// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include <Mathematics/OdeRungeKutta4.h>
#include <Mathematics/Vector4.h>
#include <memory>
using namespace gte;

class PhysicsModule
{
public:
    PhysicsModule();

    // Initialize the differential equation solver.
    void Initialize(double time, double deltaTime, double x, double w,
        double xDer, double wDer);

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

    // Apply a single step of the solver.
    void Update();

    // physical constants // symbols used in the Game Physics book
    double gravity;       // g
    double mass;          // m
    double friction;      // c
    double angle;         // phi

private:
    // state and auxiliary variables
    double mTime, mDeltaTime;
    Vector4<double> mState;
    std::array<double, 2> mAux;

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<double, Vector4<double>> Solver;
    std::unique_ptr<Solver> mSolver;
};
