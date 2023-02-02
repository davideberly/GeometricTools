// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BSplineCurveFitterWindow3.h"
#include <Graphics/VertexColorEffect.h>
#include <random>

BSplineCurveFitterWindow3::BSplineCurveFitterWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mSamples(NUM_SAMPLES),
    mDegree(3),
    mNumControls(NUM_SAMPLES / 2),
    mSpline(nullptr),
    mAvrError(0.0f),
    mRmsError(0.0f)
{
    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 0.0f, 0.0f, -4.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
    mPVWMatrices.Update();
}

void BSplineCurveFitterWindow3::OnIdle()
{
    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mHelix);
    mEngine->Draw(mPolyline);

    std::array<float, 4> textColor{ 0.0f, 0.0f, 0.0f, 1.0f };
    mMessage = "samples = " + std::to_string(mSamples.size()) +
        ", degree = " + std::to_string(mDegree) +
        ", controls = " + std::to_string(mNumControls);
    mEngine->Draw(8, 16, textColor, mMessage);

    mMessage = "avr error = " + std::to_string(mAvrError) +
        ", rms error = " + std::to_string(mRmsError);
    mEngine->Draw(8, 36, textColor, mMessage);

    mEngine->DisplayColorBuffer(0);
}

bool BSplineCurveFitterWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'd':  // reduce the degree
        if (mDegree > 1)
        {
            --mDegree;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'D':  // increase the degree
        ++mDegree;
        CreateBSplinePolyline();
        OnDisplay();
        return true;

    case 's':  // (small) reduce the number of control points by 1
        if (mNumControls - 1 > mDegree + 1)
        {
            --mNumControls;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'S':  // (small) increase the number of control points by 1
        if (mNumControls + 1 < NUM_SAMPLES)
        {
            ++mNumControls;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'm':  // (medium) reduce the number of control points by 10
        if (mNumControls - 10 > mDegree + 1)
        {
            mNumControls -= 10;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'M':  // (medium) increase the number of control points by 10
        if (mNumControls + 10 < NUM_SAMPLES)
        {
            mNumControls += 10;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'l':  // (large) reduce the number of control points by 100
        if (mNumControls - 100 > mDegree + 1)
        {
            mNumControls -= 100;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;

    case 'L':  // (large) increase the number of control points by 100
        if (mNumControls + 100 < NUM_SAMPLES)
        {
            mNumControls += 100;
            CreateBSplinePolyline();
            OnDisplay();
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void BSplineCurveFitterWindow3::CreateScene()
{
    // Generate samples on a helix.
    uint32_t numSamples = (uint32_t)mSamples.size();
    float multiplier = 2.0f / (numSamples - 1.0f);
    for (uint32_t i = 0; i < numSamples; ++i)
    {
        float t = -1.0f + multiplier * i;
        float angle = 2.0f * (float)GTE_C_TWO_PI * t;
        float amplitude = 1.0f - t * t;
        mSamples[i][0] = amplitude * std::cos(angle);
        mSamples[i][1] = amplitude * std::sin(angle);
        mSamples[i][2] = t;
        Normalize(mSamples[i]);
    }

    // Create a graphics representation of the helix.  The colors are
    // randomly generated.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.25f, 0.75f);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numSamples);
    auto* vertex = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numSamples; ++i)
    {
        vertex[i].position = mSamples[i];
        vertex[i].color = { rnd(mte), rnd(mte), rnd(mte), 1.0f };
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, numSamples - 1);

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    mHelix = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mHelix->worldTransform, effect->GetPVWMatrixConstant());

    mTrackBall.Attach(mHelix);

    CreateBSplinePolyline();
}

void BSplineCurveFitterWindow3::CreateBSplinePolyline()
{
    if (mPolyline)
    {
        mTrackBall.Detach(mPolyline);
        mPVWMatrices.Unsubscribe(mPolyline->worldTransform);
        mPolyline = nullptr;
    }

    // Create the curve from the current parameters.
    mSpline = std::make_unique<BSplineCurveFit<float>>(3, static_cast<int32_t>(mSamples.size()),
        reinterpret_cast<float const*>(&mSamples[0]), mDegree, mNumControls);

    // Sample it the same number of times as the original data.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);

    uint32_t numSamples = (uint32_t)mSamples.size();
    float multiplier = 1.0f / (numSamples - 1.0f);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numSamples);
    auto* vertex = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numSamples; ++i)
    {
        float t = multiplier * i;
        mSpline->GetPosition(t, reinterpret_cast<float*>(&vertex[i].position[0]));
        vertex[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, numSamples - 1);

    auto effect = std::make_shared<VertexColorEffect>(mProgramFactory);

    mPolyline = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mPolyline->worldTransform, effect->GetPVWMatrixConstant());

    // Compute error measurements.
    mAvrError = 0.0f;
    mRmsError = 0.0f;
    for (uint32_t i = 0; i < numSamples; ++i)
    {
        Vector3<float> diff = mSamples[i] - vertex[i].position;
        float sqrLength = Dot(diff, diff);
        mRmsError += sqrLength;
        float length = std::sqrt(sqrLength);
        mAvrError += length;
    }
    float invNumSamples = 1.0f / static_cast<float>(numSamples);
    mAvrError *= invNumSamples;
    mRmsError *= invNumSamples;
    mRmsError = std::sqrt(mRmsError);

    mTrackBall.Attach(mPolyline);
    mTrackBall.Update();
    mPVWMatrices.Update();
}
