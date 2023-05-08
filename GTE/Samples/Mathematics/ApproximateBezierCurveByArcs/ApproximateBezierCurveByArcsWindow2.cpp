// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.05.06

#include "ApproximateBezierCurveByArcsWindow2.h"
#include <Mathematics/ApprCurveByArcs.h>
#include <Mathematics/BezierCurve.h>
#include <Mathematics/ImageUtility2.h>
#include <Applications/WICFileIO.h>

ApproximateBezierCurveByArcsWindow2::ApproximateBezierCurveByArcsWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mCurve{},
    mNumArcs(1),
    mTimes{},
    mEndpoints{},
    mArcs{},
    mDrawCurve(true),
    mDrawArcs(true),
    mDrawEndpoints(true),
    mDrawMidpoints(true),
    mDrawText(true)
{
    // The curve lies in the rectangle (x,y) in [0,4]x[0,5].
    int32_t constexpr degree = 7;
    int32_t constexpr numControls = degree + 1;
    std::vector<Vector2<double>> controls(numControls);
    controls[0] = { 0.0, 0.0 };
    controls[1] = { 0.0, 2.0 };
    controls[2] = { 2.0, 2.0 };
    controls[3] = { 1.0, 1.0 };
    controls[4] = { 3.0, 0.0 };
    controls[5] = { 4.0, 3.0 };
    controls[6] = { 1.0, 5.0 };
    controls[7] = { 0.0, 4.0 };
    mCurve = std::make_shared<BezierCurve<2, double>>(degree, controls.data());

    ApproximateCurveByArcs<double>(mCurve, mNumArcs, mTimes, mEndpoints, mArcs);

    mDoFlip = true;
    OnDisplay();
}

void ApproximateBezierCurveByArcsWindow2::OnDisplay()
{
    uint32_t constexpr white = 0xFFFFFFFF;
    uint32_t constexpr black = 0xFF000000;
    uint32_t constexpr red = 0xFF0000FF;
    uint32_t constexpr green = 0xFF00FF00;
    uint32_t constexpr blue = 0xFFFF0000;
    uint32_t constexpr orange = 0xFF0080FF;

    ClearScreen(white);

    int32_t x0{}, y0{}, x1{}, y1{};

    if (mDrawCurve)
    {
        // Draw the Bezier curve. Arbitrarily choose 1025 points to sample
        // uniformly in the t-parameter.
        size_t constexpr numCurvePoints = 1024;
        std::array<Vector2<double>, 4> jet{};
        mCurve->Evaluate(0.0, 0, jet.data());
        Transform(jet[0], x0, y0);
        for (size_t j = 1; j <= numCurvePoints; ++j)
        {
            double t = static_cast<double>(j) / static_cast<double>(numCurvePoints);
            mCurve->Evaluate(t, 0, jet.data());
            Transform(jet[0], x1, y1);
            DrawLine(x0, y0, x1, y1, black);
            x0 = x1;
            y0 = y1;
        }
    }

    if (mDrawArcs)
    {
        // Draw the arcs and/or segments that estimate the curve. The naive
        // algorithm samples the arc for an arbitrarily chosen number of
        // samples.
        for (size_t i = 0, j = 1; i < mArcs.size(); ++i, j += 2)
        {
            Arc2<double> const& arc = mArcs[i];
            if (arc.radius != std::numeric_limits<double>::max())
            {
                // Sample the circle containing the arc. This is a quick-hack
                // approach that is inefficient, but suffices for the purpose
                // of this illustrative application.
                Vector2<double> const& midpoint = mEndpoints[j];
                Vector2<double> QmC = arc.center - midpoint;
                Vector2<double> Q = QmC + arc.center;
                size_t constexpr numCircleSamples = 512;
                for (size_t k = 0; k < numCircleSamples; ++k)
                {
                    double t = static_cast<double>(k) / static_cast<double>(numCircleSamples);
                    Vector2<double> W = (1.0 - t) * arc.end[0] + t * arc.end[1] - Q;
                    double s = -2.0 * Dot(W, QmC) / Dot(W, W);
                    Vector2<double> point = Q + s * W;
                    Transform(point, x0, y0);
                    SetPixel(x0, y0, green);
                }
            }
            else
            {
                // The arc represents a line segment. Draw the segment in blue.
                Transform(arc.end[0], x0, y0);
                Transform(arc.end[1], x1, y1);
                DrawLine(x0, y0, x1, y1, orange);
            }
        }
    }

    if (mDrawEndpoints)
    {
        // Draw the arc endpoints.
        for (size_t i = 0; i < mEndpoints.size(); i += 2)
        {
            auto const& endpoint = mEndpoints[i];
            Transform(endpoint, x0, y0);
            DrawThickPixel(x0, y0, 2, red);
        }
    }

    if (mDrawMidpoints)
    {
        // Draw the midpoints.
        for (size_t i = 1; i < mEndpoints.size(); i += 2)
        {
            auto const& midpoint = mEndpoints[i];
            Transform(midpoint, x0, y0);
            DrawThickPixel(x0, y0, 2, blue);
        }
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

void ApproximateBezierCurveByArcsWindow2::DrawScreenOverlay()
{
    if (mDrawText)
    {
        std::array<float, 4> black{ 0.0, 0.0, 0.0, 1.0 };
        int32_t constexpr x = 275;
        mEngine->Draw(x, 24, black, "Press '+' to increase samples.");
        mEngine->Draw(x, 48, black, "Press '-' to decrease samples.");
        mEngine->Draw(x, 72, black, "Number of arcs = " + std::to_string(mNumArcs));
        mEngine->Draw(x, 512 - 120, black, "Bezier curve is black.");
        mEngine->Draw(x, 512 - 96, black, "Segments are orange.");
        mEngine->Draw(x, 512 - 72, black, "Arcs are green.");
        mEngine->Draw(x, 512 - 48, black, "Arc endpoints are red.");
        mEngine->Draw(x, 512 - 24, black, "Arc midpoints are blue.");
    }
}

bool ApproximateBezierCurveByArcsWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case '+':
    case '=':
        if (mNumArcs < 32)
        {
            ++mNumArcs;
            ApproximateCurveByArcs<double>(mCurve, mNumArcs, mTimes, mEndpoints, mArcs);
            OnDisplay();
        }
        return true;

    case '-':
    case '_':
        if (mNumArcs > 1)
        {
            --mNumArcs;
            ApproximateCurveByArcs<double>(mCurve, mNumArcs, mTimes, mEndpoints, mArcs);
            OnDisplay();
        }
        return true;

    case 'c':
    case 'C':
        mDrawCurve = !mDrawCurve;
        OnDisplay();
        return true;

    case 'a':
    case 'A':
        mDrawArcs = !mDrawArcs;
        OnDisplay();
        return true;

    case 'e':
    case 'E':
        mDrawEndpoints = !mDrawEndpoints;
        OnDisplay();
        return true;

    case 'm':
    case 'M':
        mDrawMidpoints = !mDrawMidpoints;
        OnDisplay();
        return true;

    case 't':
    case 'T':
        mDrawText = !mDrawText;
        OnDisplay();
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}
