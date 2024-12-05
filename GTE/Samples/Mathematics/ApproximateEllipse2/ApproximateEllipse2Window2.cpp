// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.2.2024.09.06

#include "ApproximateEllipse2Window2.h"
#include <random>
#include <sstream>

ApproximateEllipse2Window2::ApproximateEllipse2Window2(Parameters& parameters)
    :
    Window2(parameters),
    mFitter{},
    mPoints{},
    mTrueEllipse{},
    mApprEllipse{},
    mIteration(0),
    mNumIterations(1024),
    mError(-1.0)
{
    mTrueEllipse.center = { 0.0, 0.0 };
    mTrueEllipse.axis[0] = { 2.0, 1.0 };
    Normalize(mTrueEllipse.axis[0]);
    mTrueEllipse.axis[1] = Perp(mTrueEllipse.axis[0]);
    mTrueEllipse.extent = { 4.0, 1.0 };

    std::default_random_engine dre{};
    std::uniform_real_distribution<double> urd(-0.25, 0.25);
    size_t constexpr numPoints = 1024;
    mPoints.resize(numPoints);
    for (size_t i = 0; i < numPoints; ++i)
    {
        mPoints[i] = GetEllipsePoint(mTrueEllipse, numPoints, i) +
            Vector2<double>{ urd(dre), urd(dre) };
    }

    // It is instructive to step through the mFitter(...) call to see how
    // the errors are reduced as the ellipse matrix and ellipse center are
    // updated.
    bool constexpr useEllipseForInitialGuess = false;
    mError = mFitter(mPoints, mNumIterations, useEllipseForInitialGuess, mApprEllipse);

    mDoFlip = true;
    OnDisplay();
}

void ApproximateEllipse2Window2::OnDisplay()
{
    uint32_t constexpr white = 0xFFFFFFFF;
    uint32_t constexpr blue = 0xFFFF0000;
    uint32_t constexpr green = 0xFF00FF00;
    uint32_t constexpr red = 0xFF0000FF;

    ClearScreen(white);

    int32_t x{}, y{};
    for (size_t i = 0; i < mPoints.size(); ++i)
    {
        Get(mPoints[i], x, y);
        DrawThickPixel(x, y, 1, green);
    }

    size_t constexpr numSamples = 2048;
    DrawMyEllipse(mTrueEllipse, numSamples, blue);
    DrawMyEllipse(mApprEllipse, numSamples, red);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

void ApproximateEllipse2Window2::DrawScreenOverlay()
{
    std::ostringstream stream{};
    stream.setf(std::ios::fixed | std::ios::showpoint);
    stream.precision(16);
    stream << mError;

    std::array<float, 4> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    mEngine->Draw(8, 24, black, "The true ellipse is blue.");
    mEngine->Draw(8, 48, black, "The green points are perturbed from the true ellipse.");
    mEngine->Draw(8, 72, black, "The fitted ellipse to the points is red.");
    mEngine->Draw(8, 96, black,
        "iteration = " + std::to_string(mIteration) + ", error = " + stream.str());
}

bool ApproximateEllipse2Window2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    if (key == ' ')
    {
        bool constexpr useEllipseForInitialGuess = true;

        if (mIteration < mNumIterations)
        {
            // Execute 1 iteration of the fitter for each space-bar press.
            // NOTE: The first call of the fitter in the constructor produces
            // an ellipse which is not modified with later calls. The value
            // mNumIterations = 1024 worked well for this dataset.
            mError = mFitter(mPoints, 1, useEllipseForInitialGuess, mApprEllipse);
            ++mIteration;
            OnDisplay();
            return true;
        }
    }

    return Window2::OnCharPress(key, x, y);
}
