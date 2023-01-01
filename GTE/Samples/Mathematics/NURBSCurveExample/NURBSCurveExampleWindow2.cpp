// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "NURBSCurveExampleWindow2.h"

NURBSCurveExampleWindow2::NURBSCurveExampleWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mSize(mXSize),
    mH(0.5f * static_cast<float>(mSize)),
    mD(0.0625f * static_cast<float>(mSize)),
    mSimTime(0.0f),
    mSimDelta(0.05f),
    mDrawControlPoints(false)
{
    InitialConfiguration();
    OnDisplay();
}

void NURBSCurveExampleWindow2::OnDisplay()
{
    uint32_t const white = 0xFFFFFFFF;
    uint32_t const black = 0xFF000000;
    uint32_t const gray = 0xFF808080;

    ClearScreen(white);

    int32_t imax = 2048;
    int32_t i, x, y;
    float t;
    Vector2<float> position;

    // Draw the spline.
    float const invIMax = 1.0f / static_cast<float>(imax);
    for (i = 0; i <= imax; ++i)
    {
        t = static_cast<float>(i) * invIMax;
        position = mSpline->GetPosition(t);
        Get(position, x, y);
        SetPixel(x, y, black);
    }

    // Draw the circle.
    if (mCircle)
    {
        for (i = 0; i <= imax; ++i)
        {
            t = static_cast<float>(i) * invIMax;
            position = mCircle->GetPosition(t);
            Get(position, x, y);
            SetPixel(x, y, black);
        }
    }

    // Draw the control points.
    if (mDrawControlPoints)
    {
        // Draw the spline control points.
        imax = mSpline->GetNumControls();
        for (i = 0; i < imax; ++i)
        {
            position = mSpline->GetControl(i);
            Get(position, x, y);
            DrawThickPixel(x, y, 2, gray);
        }

        // Draw the circle control points.
        if (mCircle)
        {
            imax = mCircle->GetNumControls();
            for (i = 0; i < imax; ++i)
            {
                position = mCircle->GetControl(i);
                Get(position, x, y);
                DrawThickPixel(x, y, 2, gray);
            }
        }
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool NURBSCurveExampleWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'g':
        if (mSimTime <= 1.0f)
        {
            DoSimulation1();
        }
        else if (mSimTime <= 2.0f)
        {
            DoSimulation2();
        }
        else
        {
            InitialConfiguration();
        }
        return true;
    case '0':
        InitialConfiguration();
        OnDisplay();
        return true;
    case 'c':
        mDrawControlPoints = !mDrawControlPoints;
        OnDisplay();
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}

void NURBSCurveExampleWindow2::DoSimulation1()
{
    mSimTime += mSimDelta;

    float t = mSimTime;
    float oneMinusT = 1.0f - t;
    int32_t imax = mSpline->GetNumControls();
    for (int32_t i = 0; i < imax; ++i)
    {
        if (i == 2 || i == 10)
        {
            float s = std::pow(t, 1.5f);
            float oneMinusS = 1.0f - s;
            mSpline->SetControl(i, oneMinusS * mControls[i] + s * mTargets[i]);
        }
        else
        {
            mSpline->SetControl(i, oneMinusT * mControls[i] + t * mTargets[i]);
        }
    }

    OnDisplay();
}

void NURBSCurveExampleWindow2::DoSimulation2()
{
    mSimTime += mSimDelta;

    if (!mCircle)
    {
        NextConfiguration();
    }
    else
    {
        // The curve evolves to a line segment.
        float t = mSimTime - 1.0f;
        float oneMinusT = 1.0f - t;
        Vector2<float> control = oneMinusT * mSpline->GetControl(2) + t * mSpline->GetControl(1);
        mSpline->SetControl(2, control);

        // The circle floats up a little bit.
        int32_t imax = mCircle->GetNumControls();
        for (int32_t i = 0; i < imax; ++i)
        {
            control = mCircle->GetControl(i) + Vector2<float>{ 0.0f, 1.0f };
            mCircle->SetControl(i, control);
        }
    }

    OnDisplay();
}

void NURBSCurveExampleWindow2::InitialConfiguration()
{
    mSimTime = 0.0f;
    mSpline = nullptr;
    mCircle = nullptr;
    mControls.clear();
    mTargets.clear();

    int32_t const numControls = 13;
    mControls.resize(numControls);
    mTargets.resize(numControls);
    for (int32_t i = 0; i < numControls; ++i)
    {
        mControls[i] = { 0.125f * mSize + 0.0625f * mSize * i, 0.0625f * mSize };
    }

    mTargets[0] = mControls[0];
    mTargets[1] = mControls[6];
    mTargets[2] = { mControls[6][0], mH - mD };
    mTargets[3] = { mControls[5][0], mH - mD };
    mTargets[4] = { mControls[5][0], mH };
    mTargets[5] = { mControls[5][0], mH + mD };
    mTargets[6] = { mControls[6][0], mH + mD };
    mTargets[7] = { mControls[7][0], mH + mD };
    mTargets[8] = { mControls[7][0], mH };
    mTargets[9] = { mControls[7][0], mH - mD };
    mTargets[10] = { mControls[6][0], mH - mD };
    mTargets[11] = mControls[6];
    mTargets[12] = mControls[12];

    std::vector<float> weights(numControls);
    for (int32_t i = 0; i < numControls; ++i)
    {
        weights[i] = 1.0f;
    }

    float const modWeight = 0.3f;
    weights[3] = modWeight;
    weights[5] = modWeight;
    weights[7] = modWeight;
    weights[9] = modWeight;

    BasisFunctionInput<float> input(numControls, 2);
    mSpline = std::make_shared<NURBSCurve<2, float>>(input, mControls.data(), weights.data());
}

void NURBSCurveExampleWindow2::NextConfiguration()
{
    mTargets.clear();

    int32_t const numControls = 16;
    mControls.resize(numControls);
    mControls[0] = mSpline->GetControl(0);
    mControls[1] = mSpline->GetControl(1);
    mControls[2] = 0.5f * (mSpline->GetControl(1) + mSpline->GetControl(2));
    mControls[3] = mSpline->GetControl(11);
    mControls[4] = mSpline->GetControl(12);
    for (int32_t i = 2, j = 5; i <= 10; ++i, ++j)
    {
        mControls[j] = mSpline->GetControl(i);
    }

    std::vector<float> weights(numControls);
    for (int32_t i = 0; i < numControls; ++i)
    {
        weights[i] = 1.0f;
    }
    weights[6] = mSpline->GetWeight(3);
    weights[8] = mSpline->GetWeight(5);
    weights[10] = mSpline->GetWeight(7);
    weights[12] = mSpline->GetWeight(9);

    // Replicate the first two control-weights to obtain C1 continuity
    // for the periodic 'mCircle'.
    mControls[14] = mControls[5];
    mControls[15] = mControls[6];
    weights[14] = weights[5];
    weights[15] = weights[6];

    BasisFunctionInput<float> input0(5, 2);
    mSpline = std::make_shared<NURBSCurve<2, float>>(input0, mControls.data(), weights.data());

    BasisFunctionInput<float> input1;
    input1.numControls = 11;
    input1.degree = 2;
    input1.uniform = true;
    input1.periodic = true;
    input1.numUniqueKnots = input1.numControls + input1.degree + 1;
    input1.uniqueKnots.resize(input1.numUniqueKnots);
    float invNmD = 1.0f / static_cast<float>(input1.numControls - input1.degree);
    for (int32_t i = 0; i < input1.numUniqueKnots; ++i)
    {
        input1.uniqueKnots[i].t = static_cast<float>(i - input1.degree) * invNmD;
        input1.uniqueKnots[i].multiplicity = 1;
    }
    mCircle = std::make_shared<NURBSCurve<2, float>>(input1, mControls.data() + 5, weights.data() + 5);
}
