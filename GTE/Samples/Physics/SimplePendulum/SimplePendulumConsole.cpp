// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "SimplePendulumConsole.h"
#include <Applications/WICFileIO.h>
#include <Mathematics/ImageUtility2.h>

SimplePendulumConsole::SimplePendulumConsole(Parameters& parameters)
    :
    Console(parameters),
    mSize(512),
    mImage(std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, mSize, mSize)),
    mOutput(mSize),
    mPendulumConstant(1.0f)
{
    mDrawPixel = [this](int32_t x, int32_t y)
    {
        if (0 <= x && x < mSize && 0 <= y && y < mSize)
        {
            uint32_t* pixels = mImage->Get<uint32_t>();
            pixels[x + mSize * y] = 0xFF000000;
        }
    };
}

void SimplePendulumConsole::Execute()
{
    SolveODE(&SimplePendulumConsole::ExplicitEuler, "explicit.png", "explicit.txt");
    SolveODE(&SimplePendulumConsole::ImplicitEuler, "implicit.png", "implicit.txt");
    SolveODE(&SimplePendulumConsole::RungeKutta, "runge.png", "runge.txt");
    SolveODE(&SimplePendulumConsole::LeapFrog, "leapfrog.png", "leapfrog.txt");
    Stiff1();
    Stiff2TrueSolution();
    Stiff2ApproximateSolution();
}

float SimplePendulumConsole::F0(float t, float x, float y)
{
    return 9.0f * x + 24.0f * y + 5.0f * std::cos(t) - std::sin(t) / 3.0f;
}

float SimplePendulumConsole::F1(float t, float x, float y)
{
    return -24.0f * x - 51.0f * y - 9.0f * std::cos(t) + std::sin(t) / 3.0f;
}

void SimplePendulumConsole::ExplicitEuler(float x0, float y0, float h)
{
    for (int32_t i = 0; i < mSize; ++i)
    {
        float x1 = x0 + h * y0;
        float y1 = y0 - h * mPendulumConstant * std::sin(x0);

        mOutput[i] = x1;
        x0 = x1;
        y0 = y1;
    }
}

void SimplePendulumConsole::ImplicitEuler(float x0, float y0, float h)
{
    float const k0 = mPendulumConstant * h * h;
    for (int32_t i = 0; i < mSize; ++i)
    {
        float k1 = x0 + h * y0;
        float x1 = x0;
        int32_t const maxIteration = 32;
        for (int32_t j = 0; j < maxIteration; ++j)
        {
            float g = x1 + k0 * std::sin(x1) - k1;
            float gDer = 1.0f + k0 * std::cos(x1);
            x1 -= g / gDer;
        }
        float y1 = y0 - h * mPendulumConstant * std::sin(x1);

        mOutput[i] = x1;
        x0 = x1;
        y0 = y1;
    }
}

void SimplePendulumConsole::RungeKutta(float x0, float y0, float h)
{
    for (int32_t i = 0; i < mSize; ++i)
    {
        float k1X = h * y0;
        float k1Y = -h * mPendulumConstant * std::sin(x0);
        float x1 = x0 + 0.5f * k1X;
        float y1 = y0 + 0.5f * k1Y;
        float k2X = h * y1;
        float k2Y = -h * mPendulumConstant * std::sin(x1);
        x1 = x0 + 0.5f * k2X;
        y1 = y0 + 0.5f * k2Y;
        float k3X = h * y1;
        float k3Y = -h * mPendulumConstant * std::sin(x1);
        x1 = x0 + k3X;
        y1 = y0 + k3Y;
        float k4X = h * y1;
        float k4Y = -h * mPendulumConstant * std::sin(x1);
        x1 = x0 + (k1X + 2.0f * k2X + 2.0f * k3X + k4X) / 6.0f;
        y1 = y0 + (k1Y + 2.0f * k2Y + 2.0f * k3Y + k4Y) / 6.0f;

        mOutput[i] = x1;
        x0 = x1;
        y0 = y1;
    }
}

void SimplePendulumConsole::LeapFrog(float x0, float y0, float h)
{
    // Generate first iterate with Euler's to start up the process.
    float x1 = x0 + h * y0;
    float y1 = y0 - h * mPendulumConstant * std::sin(x0);
    mOutput[0] = x1;

    for (int32_t i = 1; i < mSize; ++i)
    {
        float x2 = x0 + 2.0f * h * y1;
        float y2 = y0 - 2.0f * h * mPendulumConstant * std::sin(x1);

        mOutput[i] = x2;
        x0 = x1;
        y0 = y1;
        x1 = x2;
        y1 = y2;
    }
}

void SimplePendulumConsole::SolveODE(SolverFunction solver, std::string const& outImage, std::string const& outText)
{
    float x0 = 0.1f, y0 = 1.0f;
    float h = 0.1f;
    (this->*solver)(x0, y0, h);

    // Write the approximation solution as text.
    std::ofstream outFile(outText);
    for (int32_t i = 0; i < mSize; ++i)
    {
        outFile << "i = " << i << ", " << mOutput[i] << std::endl;
    }
    outFile.close();

    // Draw the approximate solution as an image.
    std::memset(mImage->GetData(), 0xFF, mImage->GetNumBytes());
    float y = 256.0f * (mOutput[0] + 3.0f) / 6.0f;
    int32_t iY0 = mSize - 1 - static_cast<int32_t>(y);
    for (int32_t i = 1; i < mSize; ++i)
    {
        y = 256.0f * (mOutput[i] + 3.0f) / 6.0f;
        int32_t iY1 = mSize - 1 - static_cast<int32_t>(y);
        ImageUtility2::DrawLine(i - 1, iY0, i, iY1, mDrawPixel);
        iY0 = iY1;
    }
    WICFileIO::SaveToPNG(outImage, mImage);
}

void SimplePendulumConsole::Stiff1()
{
    int32_t const maxIterations = 1024 + 256;
    float const cSqr = 2.0f, c = std::sqrt(2.0f);

    float h = 0.01f;
    float x0 = 1.0f, x0Save = x0;
    float y0 = -c * x0;

    std::vector<float> approx(maxIterations);
    int32_t i;
    for (i = 0; i < maxIterations; ++i)
    {
        float k1X = h * y0;
        float k1Y = h * cSqr * x0;
        float x1 = x0 + 0.5f * k1X;
        float y1 = y0 + 0.5f * k1Y;
        float k2X = h * y1;
        float k2Y = h * cSqr * x1;
        x1 = x0 + 0.5f * k2X;
        y1 = y0 + 0.5f * k2Y;
        float k3X = h * y1;
        float k3Y = h * cSqr * x1;
        x1 = x0 + k3X;
        y1 = y0 + k3Y;
        float k4X = h * y1;
        float k4Y = h * cSqr * x1;
        x1 = x0 + (k1X + 2.0f * k2X + 2.0f * k3X + k4X) / 6.0f;
        y1 = y0 + (k1Y + 2.0f * k2Y + 2.0f * k3Y + k4Y) / 6.0f;

        approx[i] = x1;
        x0 = x1;
        y0 = y1;
    }

    // Write the approximation solution as text.
    std::ofstream outFile("stiff1.txt");
    for (i = 0; i < maxIterations; ++i)
    {
        outFile << "i = " << i << ", " << approx[i] << std::endl;
    }
    outFile.close();

    // Draw the true solution.
    std::memset(mImage->GetData(), 0xFF, mImage->GetNumBytes());
    float y = 256.0f * (x0Save + 3.0f) / 6.0f;
    int32_t iY0 = mSize - 1 - static_cast<int32_t>(y);
    for (i = 1; i < mSize; ++i)
    {
        int32_t j = (maxIterations - 1) * i / (mSize - 1);
        y = 256.0f * (x0Save * std::exp(-c * j * h) + 3.0f) / 6.0f;
        int32_t iY1 = mSize - 1 - static_cast<int32_t>(y);
        ImageUtility2::DrawLine(i - 1, iY0, i, iY1, mDrawPixel);
        iY0 = iY1;
    }
    WICFileIO::SaveToPNG("stiff1_true.png", mImage);

    // Draw the approximate solution as an image.
    std::memset(mImage->GetData(), 0xFF, mImage->GetNumBytes());
    y = 256.0f * (approx[0] + 3.0f) / 6.0f;
    iY0 = mSize - 1 - static_cast<int32_t>(y);
    for (i = 1; i < mSize; ++i)
    {
        int32_t j = (maxIterations - 1) * i / (mSize - 1);
        y = 256.0f * (approx[j] + 3.0f) / 6.0f;
        int32_t iY1 = mSize - 1 - static_cast<int32_t>(y);
        ImageUtility2::DrawLine(i - 1, iY0, i, iY1, mDrawPixel);
        iY0 = iY1;
    }
    WICFileIO::SaveToPNG("stiff1_appr.png", mImage);
}

void SimplePendulumConsole::Stiff2TrueSolution()
{
    float h = 0.05f;
    float x0 = 4.0f / 3.0f;
    float y0 = 2.0f / 3.0f;
    float t0 = 0.0f;

    std::ofstream outFile("stiff2_true.txt");
    int32_t const maxIterations = 20;
    for (int32_t i = 0; i <= maxIterations; ++i, t0 += h)
    {
        float e0 = std::exp(-3.0f * t0);
        float e1 = std::exp(-39.0f * t0);
        float cDiv3 = std::cos(t0) / 3.0f;
        x0 = 2.0f * e0 - e1 + cDiv3;
        y0 = -e0 + 2.0f * e1 - cDiv3;
        if (i >= 2 && ((i % 2) == 0))
        {
            outFile << "i = " << i << ", " << x0 << ", " << y0 << std::endl;
        }
    }
    outFile.close();
}

void SimplePendulumConsole::Stiff2ApproximateSolution()
{
    // Approximation with step size 0.05.
    float h = 0.05f;
    float x0 = 4.0f / 3.0f;
    float y0 = 2.0f / 3.0f;
    float t0 = 0.0f;

    int32_t const maxIterations = 20;
    std::vector<float> approx0(maxIterations + 1), approx1(maxIterations + 1);
    approx0[0] = x0;
    approx1[0] = y0;
    int32_t i;
    for (i = 1; i <= maxIterations; ++i)
    {
        float k1X = h * F0(t0, x0, y0);
        float k1Y = h * F1(t0, x0, y0);
        float x1 = x0 + 0.5f * k1X;
        float y1 = y0 + 0.5f * k1Y;
        float k2X = h * F0(t0 + 0.5f * h, x1, y1);
        float k2Y = h * F1(t0 + 0.5f * h, x1, y1);
        x1 = x0 + 0.5f * k2X;
        y1 = y0 + 0.5f * k2Y;
        float k3X = h * F0(t0 + 0.5f * h, x1, y1);
        float k3Y = h * F1(t0 + 0.5f * h, x1, y1);
        x1 = x0 + k3X;
        y1 = y0 + k3Y;
        float k4X = h * F0(t0 + h, x1, y1);
        float k4Y = h * F1(t0 + h, x1, y1);
        x1 = x0 + (k1X + 2.0f * k2X + 2.0f * k3X + k4X) / 6.0f;
        y1 = y0 + (k1Y + 2.0f * k2Y + 2.0f * k3Y + k4Y) / 6.0f;

        approx0[i] = x1;
        approx1[i] = y1;
        x0 = x1;
        y0 = y1;
        t0 += h;
    }

    std::ofstream outFile("stiff2_appr_h0.05.txt");
    for (i = 0; i <= maxIterations; ++i)
    {
        if ((i % 2) == 0)
        {
            outFile << "i = " << i << ", " << approx0[i] << ", " << approx1[i] << std::endl;
        }
    }
    outFile.close();

    // Approximation with step size 0.1.
    h = 0.1f;
    x0 = 4.0f / 3.0f;
    y0 = 2.0f / 3.0f;
    t0 = 0.0f;

    approx0[0] = x0;
    approx1[0] = y0;
    for (i = 1; i <= maxIterations / 2; ++i)
    {
        float k1X = h * F0(t0, x0, y0);
        float k1Y = h * F1(t0, x0, y0);
        float x1 = x0 + 0.5f * k1X;
        float y1 = y0 + 0.5f * k1Y;
        float k2X = h * F0(t0 + 0.5f * h, x1, y1);
        float k2Y = h * F1(t0 + 0.5f * h, x1, y1);
        x1 = x0 + 0.5f * k2X;
        y1 = y0 + 0.5f * k2Y;
        float k3X = h * F0(t0 + 0.5f * h, x1, y1);
        float k3Y = h * F1(t0 + 0.5f * h, x1, y1);
        x1 = x0 + k3X;
        y1 = y0 + k3Y;
        float k4X = h * F0(t0 + h, x1, y1);
        float k4Y = h * F1(t0 + h, x1, y1);
        x1 = x0 + (k1X + 2.0f * k2X + 2.0f * k3X + k4X) / 6.0f;
        y1 = y0 + (k1Y + 2.0f * k2Y + 2.0f * k3Y + k4Y) / 6.0f;

        approx0[i] = x1;
        approx1[i] = y1;
        x0 = x1;
        y0 = y1;
        t0 += h;
    }

    outFile.open("stiff2_appr_h0.10.txt");
    for (i = 0; i <= maxIterations / 2; ++i)
    {
        outFile << "i = " << i << ", " << approx0[i] << ", " << approx1[i] << std::endl;
    }
    outFile.close();
}
