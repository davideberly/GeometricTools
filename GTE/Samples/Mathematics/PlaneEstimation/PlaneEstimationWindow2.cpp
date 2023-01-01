// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "PlaneEstimationWindow2.h"
#include <random>

PlaneEstimationWindow2::PlaneEstimationWindow2 (Parameters& parameters)
    :
    Window2(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    // Create the shaders.
    int32_t const numThreads = 8, txWidth = 1024, txHeight = 1024;
    mNumXGroups = txWidth / numThreads;
    mNumYGroups = txHeight / numThreads;

    mProgramFactory->defines.Set("NUM_X_THREADS", numThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", numThreads);
    mProgramFactory->defines.Set("RADIUS", 3);

    std::string csPath = mEnvironment.GetPath(mEngine->GetShaderName("EvaluateBezier.cs"));
    mPositionProgram = mProgramFactory->CreateFromFile(csPath);
    if (!mPositionProgram)
    {
        parameters.created = false;
        return;
    }

    csPath = mEnvironment.GetPath(mEngine->GetShaderName("PlaneEstimation.cs"));
    mPlaneProgram = mProgramFactory->CreateFromFile(csPath);
    if (!mPlaneProgram)
    {
        parameters.created = false;
        return;
    }

    // Create and attach resources to the shaders.
    auto cshader = mPositionProgram->GetComputeShader();
    cshader->Set("ControlPoints", CreateBezierControls());
    mPositions = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, txWidth, txHeight);
    mPositions->SetUsage(Resource::Usage::SHADER_OUTPUT);
    mPositions->SetCopy(Resource::Copy::STAGING_TO_CPU);
    cshader->Set("positions", mPositions);

    cshader = mPlaneProgram->GetComputeShader();
    cshader->Set("positions", mPositions);
    mPlanes = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, txWidth, txHeight);
    mPlanes->SetUsage(Resource::Usage::SHADER_OUTPUT);
    mPlanes->SetCopy(Resource::Copy::STAGING_TO_CPU);
    cshader->Set("planes", mPlanes);

    auto sstate = std::make_shared<SamplerState>();
    sstate->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_P;
    sstate->mode[0] = SamplerState::Mode::CLAMP;
    sstate->mode[1] = SamplerState::Mode::CLAMP;

    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("PositionVisualize.ps"));
    std::string psString = ProgramFactory::GetStringFromFile(psPath);
    mOverlay[0] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, txWidth, txHeight, psString);
    std::array<int32_t, 4> rect = { 0, 0, mXSize / 2, mYSize };
    mOverlay[0]->SetOverlayRectangle(rect);
    std::shared_ptr<Shader> pshader = mOverlay[0]->GetProgram()->GetPixelShader();
    pshader->Set("myTexture", mPositions, "mySampler", sstate);

    psPath = mEnvironment.GetPath(mEngine->GetShaderName("PlaneVisualize.ps"));
    psString = ProgramFactory::GetStringFromFile(psPath);
    mOverlay[1] = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, txWidth, txHeight, psString);
    rect = { mXSize / 2, 0, mXSize / 2, mYSize };
    mOverlay[1]->SetOverlayRectangle(rect);
    pshader = mOverlay[1]->GetProgram()->GetPixelShader();
    pshader->Set("myTexture", mPlanes, "mySampler", sstate);
}

void PlaneEstimationWindow2::OnDisplay ()
{
    mEngine->Execute(mPositionProgram, mNumXGroups, mNumYGroups, 1);
    mEngine->Execute(mPlaneProgram, mNumXGroups, mNumYGroups, 1);
    mEngine->Draw(mOverlay[0]);
    mEngine->Draw(mOverlay[1]);
    mEngine->DisplayColorBuffer(0);
}

bool PlaneEstimationWindow2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Mathematics/PlaneEstimation/Shaders/");
    std::vector<std::string> inputs =
    {
        mEngine->GetShaderName("EvaluateBezier.cs"),
        mEngine->GetShaderName("PlaneEstimation.cs"),
        mEngine->GetShaderName("PlaneVisualize.ps"),
        mEngine->GetShaderName("PositionVisualize.ps")
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find " + input);
            return false;
        }
    }

    return true;
}

std::shared_ptr<ConstantBuffer> PlaneEstimationWindow2::CreateBezierControls()
{
    // Generate random samples for the bicubic Bezier surface.  The w-channel
    // is irrelevant, so is set to zero.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-0.25f, 1.0f);
    float P[4][4];
    for (int32_t r = 0; r < 4; ++r)
    {
        for (int32_t c = 0; c < 4; ++c)
        {
            P[r][c] = rnd(mte);
        }
    }

    // Construct the control points from the samples.
    float control[4][4];
    control[0][0] = P[0][0];
    control[0][1] = (
        -5.0f * P[0][0] + 18.0f * P[0][1] - 9.0f * P[0][2] + 2.0f * P[0][3]
        ) / 6.0f;
    control[0][2] = (
        +2.0f * P[0][0] - 9.0f * P[0][1] + 18.0f * P[0][2] - 5.0f * P[0][3]
        ) / 6.0f;
    control[0][3] = P[0][3];
    control[1][0] = (
        -5.0f * P[0][0] + 18.0f * P[1][0] - 9.0f * P[2][0] - 5.0f * P[3][0]
        ) / 6.0f;
    control[1][1] = (
        + 25.0f * P[0][0] -  90.0f * P[0][1] +  45.0f * P[0][2] - 10.0f * P[0][3]
        - 90.0f * P[1][0] + 324.0f * P[1][1] - 162.0f * P[1][2] + 36.0f * P[1][3]
        + 45.0f * P[2][0] - 162.0f * P[2][1] +  81.0f * P[2][2] - 18.0f * P[2][3]
        - 10.0f * P[3][0] +  36.0f * P[3][1] -  18.0f * P[3][2] +  4.0f * P[3][3]
        ) / 36.0f;
    control[1][2] = (
        - 10.0f * P[0][0] +  45.0f * P[0][1] -  90.0f * P[0][2] + 25.0f * P[0][3]
        + 36.0f * P[1][0] - 162.0f * P[1][1] + 324.0f * P[1][2] - 90.0f * P[1][3]
        - 18.0f * P[2][0] +  81.0f * P[2][1] - 162.0f * P[2][2] + 45.0f * P[2][3]
        +  4.0f * P[3][0] -  18.0f * P[3][1] +  36.0f * P[3][2] - 10.0f * P[3][3]
        ) / 36.0f;
    control[1][3] = (
        -5.0f * P[0][3] + 18.0f * P[1][3] - 9.0f * P[2][3] + 2.0f * P[3][3]
        ) / 6.0f;
    control[2][0] = (
        +2.0f * P[0][0] - 9.0f * P[1][0] + 18.0f * P[2][0] - 5.0f * P[3][0]
        ) / 6.0f;
    control[2][1] = (
        - 10.0f * P[0][0] +  36.0f * P[0][1] -  18.0f * P[0][2] +  4.0f * P[0][3]
        + 45.0f * P[1][0] - 162.0f * P[1][1] +  81.0f * P[1][2] - 18.0f * P[1][3]
        - 90.0f * P[2][0] + 324.0f * P[2][1] - 162.0f * P[2][2] + 36.0f * P[2][3]
        + 25.0f * P[3][0] -  90.0f * P[3][1] +  45.0f * P[3][2] - 10.0f * P[3][3]
        ) / 36.0f;
    control[2][2] = (
        +  4.0f * P[0][0] -  18.0f * P[0][1] +  36.0f * P[0][2] - 10.0f * P[0][3]
        - 18.0f * P[1][0] +  81.0f * P[1][1] - 162.0f * P[1][2] + 45.0f * P[1][3]
        + 36.0f * P[2][0] - 162.0f * P[2][1] + 324.0f * P[2][2] - 90.0f * P[2][3]
        - 10.0f * P[3][0] +  45.0f * P[3][1] -  90.0f * P[3][2] + 25.0f * P[3][3]
        ) / 36.0f;
    control[2][3] = (
        +2.0f * P[0][3] - 9.0f * P[1][3] + 18.0f * P[2][3] - 5.0f * P[3][3]
        ) / 6.0f;
    control[3][0] = P[3][0];
    control[3][1] = (
        -5.0f * P[3][0] + 18.0f * P[3][1] - 9.0f * P[3][2] + 2.0f * P[3][3]
        ) / 6.0f;
    control[3][2] = (
        +2.0f * P[3][0] - 9.0f * P[3][1] + 18.0f * P[3][2] - 5.0f * P[3][3]
        ) / 6.0f;
    control[3][3] = P[3][3];

    auto cbuffer = std::make_shared<ConstantBuffer>(4 * sizeof(Vector4<float>), false);
    auto data = cbuffer->Get<Vector4<float>>();
    for (int32_t r = 0; r < 4; ++r)
    {
        Vector4<float>& trg = data[r];
        for (int32_t c = 0; c < 4; ++c)
        {
            trg[c] = control[r][c];
        }
    }

    return cbuffer;
}
