// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Matrix4x4.h>
#include <Mathematics/OdeRungeKutta4.h>
#include <memory>
using namespace gte;

class PhysicsModule
{
public:
    // Construction.
    PhysicsModule ();

    // Initialize the differential equation solver.
    void Initialize(float time, float deltaTime, float y1, float y2, float y1Dot, float y2Dot);

    // Access the current state.
    inline float GetTime() const
    {
        return mTime;
    }

    inline float GetDeltaTime() const
    {
        return (mSolver ? mSolver->GetTDelta() : 0.0f);
    }

    inline float GetY1() const
    {
        return mState[0];
    }

    inline float GetY1Dot() const
    {
        return mState[1];
    }

    inline float GetY2() const
    {
        return mState[2];
    }

    inline float GetY2Dot() const
    {
        return mState[3];
    }

    // Get the ball center and incremental rotation (after update).
    void GetData(Vector4<float>& center, Matrix4x4<float>& incrRot) const;

    // Compute paraboloid height from an xy-plane position.
    float GetHeight(float x, float y) const;

    // Take a single step of the solver.
    void Update();

    // The physical constants.
    float gravity;

    // The paraboloid parameters.
    float a1, a2, a3;

    // The ball radius.
    float radius;

private:
    // The paraboloid is x3 = a3 - (x1/a1)^2 - (x2/a2)^2.  The equations of
    // motion are:
    //   x1"+(4*x1/a1^2)*((x1*x1"+(x1')^2)/a1^2+(x2*x2"+(x2')^2)/a2^2)
    //     = 2*g*x1/a1^2
    //   x2"+(4*x2/a2^2)*((x1*x1"+(x1')^2)/a1^2+(x2*x2"+(x2')^2)/a2^2)
    //     = 2*g*x2/a2^2
    // Make the change of variables y1 = x1/a2 and y2 = x2/a2.  The equations
    // of motion are:
    //   a1^2*y1"+4*y1*(y1*y1"+(y1')^2+y2*y2"+(y2')^2) = 2g*y1
    //   a2^2*y2"+4*y2*(y1*y1"+(y1')^2+y2*y2"+(y2')^2) = 2g*y2
    // The second derivatives y1" and y2" can be solved algebraically:
    //  +   +   +                       +^{-1} +                             +
    //  |y1"| = |a1^2+4*y1^2 4*y1*y2    |      |2*g*y1-4*y1*((y1')^2+(y2')^2)|
    //  |y2"|   |4*y1*y2     a2^2+4*y2^2|      |2*g*y2-4*y2*((y1')^2+(y2')^2)|
    //  +   +   +                       +      +                             +
    //
    // The four state variables for the RK4 solver.
    //
    // state[0] = y1
    // state[1] = y1'
    // state[2] = y2
    // state[3] = y2'
    //
    // Auxiliary variables that the caller of the RK4 Update function must
    // set before passing to the update.
    //
    // aux[0] = a1^2
    // aux[1] = a2^2
    // aux[2] = g
    // aux[3] = storage for return value of DY2Dot, calculated in DY1Dot

    // State and auxiliary variables.
    float mTime;
    Vector4<float> mState;
    std::array<float, 3> mAux;

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<float, Vector4<float>> Solver;
    std::unique_ptr<Solver> mSolver;
};
