// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "GaussianBlurringWindow2.h"
#include <Applications/WICFileIO.h>

GaussianBlurringWindow2::GaussianBlurringWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mNumXThreads(8),
    mNumYThreads(8),
    mNumXGroups(mXSize / mNumXThreads),  // 1024/8 = 128
    mNumYGroups(mYSize / mNumYThreads),  // 768/8 = 96
    mPass(0)
{
    if (!SetEnvironment() || !CreateImages() || !CreateShader())
    {
        parameters.created = false;
        return;
    }

    // Create an overlay that covers the entire window.  The blurred image
    // is drawn by the overlay effect.
    mOverlay = std::make_shared<OverlayEffect>(mProgramFactory, mXSize,
        mYSize, mXSize, mYSize, SamplerState::Filter::MIN_P_MAG_P_MIP_P,
        SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP, true);
    mOverlay->SetTexture(mImage[1]);

#if defined(SAVE_RENDERING_TO_DISK)
    mTarget = std::make_shared<DrawTarget>(1, DF_R8G8B8A8_UNORM, mXSize, mYSize);
    mTarget->GetRTTexture(0)->SetCopy(Resource::Copy::STAGING_TO_CPU);
#endif
}

void GaussianBlurringWindow2::OnIdle()
{
    mTimer.Measure();

#if defined(SAVE_RENDERING_TO_DISK)
    if (mPass == 0 || mPass == 100 || mPass == 1000 || mPass == 10000)
    {
        mOverlay->SetTexture(mImage[0]);
        mEngine->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        mEngine->ClearBuffers();
        mEngine->Enable(mTarget);
        mEngine->Draw(mOverlay);
        mEngine->Disable(mTarget);
        mEngine->CopyGpuToCpu(mTarget->GetRTTexture(0));
        WICFileIO::SaveToPNG("Gauss" + std::to_string(mPass) + ".png", mTarget->GetRTTexture(0));
        mOverlay->SetTexture(mImage[1]);
    }
#endif

    auto const& cshader = mGaussianBlurProgram->GetComputeShader();
    mEngine->Execute(mGaussianBlurProgram, mNumXGroups, mNumYGroups, 1);
    mEngine->Draw(mOverlay);
    std::swap(mImage[0], mImage[1]);
    cshader->Set("inImage", mImage[0]);
    cshader->Set("outImage", mImage[1]);
    mOverlay->SetTexture(mImage[1]);
    ++mPass;

    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool GaussianBlurringWindow2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Imagics/GaussianBlurring/Shaders/");
    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "MedicineBag.png",
        mEngine->GetShaderName("GaussianBlur3x3.cs")
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

bool GaussianBlurringWindow2::CreateImages()
{
    for (int32_t i = 0; i < 2; ++i)
    {
        mImage[i] = std::make_shared<Texture2>(DF_R32G32B32A32_FLOAT, mXSize, mYSize);
        mImage[i]->SetUsage(Resource::Usage::SHADER_OUTPUT);
    }

    std::string path = mEnvironment.GetPath("MedicineBag.png");
    auto original = WICFileIO::Load(path, false);
    auto const* src = original->Get<uint32_t>();
    auto* trg = mImage[0]->Get<float>();
    for (int32_t j = 0; j < mXSize*mYSize; ++j)
    {
        uint32_t rgba = *src++;
        *trg++ = (rgba & 0x000000FF) / 255.0f;
        *trg++ = ((rgba & 0x0000FF00) >> 8) / 255.0f;
        *trg++ = ((rgba & 0x00FF0000) >> 16) / 255.0f;
        *trg++ = 1.0f;
    }

    return true;
}

bool GaussianBlurringWindow2::CreateShader()
{
    mProgramFactory->defines.Set("NUM_X_THREADS", mNumXThreads);
    mProgramFactory->defines.Set("NUM_Y_THREADS", mNumYThreads);
    std::string csPath = mEnvironment.GetPath(mEngine->GetShaderName("GaussianBlur3x3.cs"));
    mGaussianBlurProgram = mProgramFactory->CreateFromFile(csPath);
    mProgramFactory->defines.Clear();

    if (mGaussianBlurProgram)
    {
        auto cshader = mGaussianBlurProgram->GetComputeShader();
        cshader->Set("inImage", mImage[0]);
        cshader->Set("outImage", mImage[1]);
        return true;
    }
    return false;
}
