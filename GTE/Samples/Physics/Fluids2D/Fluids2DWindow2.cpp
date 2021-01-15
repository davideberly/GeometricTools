// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "Fluids2DWindow2.h"
#include <Applications/WICFileIO.h>

Fluids2DWindow2::Fluids2DWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mFluid(mEngine, mProgramFactory, GRID_SIZE, GRID_SIZE, 0.001f, 0.0001f, 0.0001f)
{
    if (!SetEnvironment() || !CreateOverlay())
    {
        parameters.created = false;
        return;
    }

    mFluid.Initialize();
}

void Fluids2DWindow2::OnIdle()
{
    mTimer.Measure();

    mFluid.DoSimulationStep();
    mEngine->Draw(mOverlay);
    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(1);

#if defined(SAVE_RENDERING_TO_DISK)
    mEngine->Enable(mTarget);
    mEngine->ClearBuffers();
    mEngine->Draw(mOverlay);
    mEngine->Disable(mTarget);
    mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
    WICFileIO::SaveToPNG("Video/Smoke" + std::to_string(mVideoFrame) + ".png",
        mTarget->GetRTTexture(0));
    ++mVideoFrame;
#endif

    mTimer.UpdateFrameCount();
}

bool Fluids2DWindow2::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '0':
        mFluid.Initialize();
        return true;
    }

    return Window::OnCharPress(key, x, y);
}

bool Fluids2DWindow2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Physics/Fluids2D/Shaders/");

    if (mEnvironment.GetPath(mEngine->GetShaderName("DrawDensity.ps")) == "")
    {
        LogError("Cannot find file " + mEngine->GetShaderName("DrawDensity.ps"));
        return false;
    }

    return true;
}

bool Fluids2DWindow2::CreateOverlay()
{
    // Create the supporting objects for visualizing the fluid simulation.
    std::string psPath = mEnvironment.GetPath(mEngine->GetShaderName("DrawDensity.ps"));
    std::string psSource = ProgramFactory::GetStringFromFile(psPath);
    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize, mYSize,
        GRID_SIZE, GRID_SIZE, psSource);
    auto stateSampler = std::make_shared<SamplerState>();
    stateSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    stateSampler->mode[0] = SamplerState::CLAMP;
    stateSampler->mode[1] = SamplerState::CLAMP;
    auto pshader = mOverlay->GetProgram()->GetPixelShader();
    pshader->Set("stateTexture", mFluid.GetState(), "stateSampler", stateSampler);

    mNoDepthState = std::make_shared<DepthStencilState>();
    mNoDepthState->depthEnable = false;
    mEngine->SetDepthStencilState(mNoDepthState);
    mNoCullingState = std::make_shared<RasterizerState>();
    mNoCullingState->cullMode = RasterizerState::CULL_NONE;
    mEngine->SetRasterizerState(mNoCullingState);

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize, mYSize);
    mTarget->GetRTTexture(0)->SetCopyType(Resource::COPY_STAGING_TO_CPU);
    mVideoFrame = 0;
#endif
    return true;
}
