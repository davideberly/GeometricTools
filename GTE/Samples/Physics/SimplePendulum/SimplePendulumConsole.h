// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Console.h>
#include <Graphics/Texture2.h>
using namespace gte;

class SimplePendulumConsole : public Console
{
public:
    SimplePendulumConsole(Parameters& parameters);

    virtual void Execute() override;

private:
    typedef void (SimplePendulumConsole::*SolverFunction)(float, float, float);

    static float F0(float t, float x, float y);
    static float F1(float t, float x, float y);

    void ExplicitEuler(float x0, float y0, float h);
    void ImplicitEuler(float x0, float y0, float h);
    void RungeKutta(float x0, float y0, float h);
    void LeapFrog(float x0, float y0, float h);
    void SolveODE(SolverFunction solver, std::string const& outImage, std::string const& outText);
    void Stiff1();
    void Stiff2TrueSolution();
    void Stiff2ApproximateSolution();

    int32_t const mSize;
    std::shared_ptr<Texture2> mImage;
    std::vector<float> mOutput;
    float mPendulumConstant;
    std::function<void(int32_t, int32_t)> mDrawPixel;
};
